#include <defines.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <cerrno>

#include <asuka.hpp>
#include <os/memory.hpp>
#include <os/time.hpp>


#define GAMEPAD_EVENT_BUTTON         0x01    /* button pressed/released */
#define GAMEPAD_EVENT_AXIS           0x02    /* joystick moved */
#define GAMEPAD_EVENT_INIT           0x80    /* initial state of device */

struct linux_joystick_event {
    uint32 time;   /* event timestamp in milliseconds */
    int16  value;  /* value */
    uint8  type;   /* event type */
    uint8  number; /* axis/button number */
};


int linux_is_valid_fd(int fd) {
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}


bool32 linux_gamepad_is_plugged_in(int fd) {
    struct stat statbuf;
    int ec = fstat(fd, &statbuf);

    bool32 result = (ec == 0) && (statbuf.st_nlink > 0);
    return result;
}


struct linux_screen_buffer {
    XImage x_image;
    void* memory;
    uint32 width;
    uint32 height;
    uint32 bytes_per_pixel;
};


bool running;


static void linux_resize_screen_buffer(linux_screen_buffer* buffer, uint32 width, uint32 height) {
    if (buffer->memory) {
        os::free_pages(buffer->memory, buffer->width * buffer->height * buffer->bytes_per_pixel);
        buffer->memory = NULL;
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = 4;

    buffer->memory = os::allocate_pages(width * height * buffer->bytes_per_pixel);
    ASSERT(buffer->memory);
}


static void linux_copy_buffer_to_window(linux_screen_buffer* buffer, Display* display, Window window, int screen) {
    // @TODO: Should I cache this structure int the linux_screen_buffer to avoid XInitImage call ??
    XImage x_image {};
    x_image.width = buffer->width;
    x_image.height = buffer->height;
    x_image.xoffset = 0;
    x_image.format = ZPixmap;
    x_image.data = (char*)buffer->memory;
    x_image.byte_order = LSBFirst;
    x_image.bitmap_unit = 32;
    x_image.bitmap_bit_order = LSBFirst;
    x_image.bitmap_pad = 32;
    x_image.depth = 24;
    x_image.bits_per_pixel = 32;
    x_image.red_mask = 0xFF0000;
    x_image.green_mask = 0xFF00;
    x_image.blue_mask = 0xFF;

    int good = XInitImage(&x_image);
    ASSERT(good);

    XPutImage(display, window, DefaultGC(display, screen), &x_image,
        0, 0, 0, 0, buffer->width, buffer->height);
}


int32 main(int32 argc, char** argv) {
    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("Cannot open display\n");
        return 1;
    }

    int screen = XDefaultScreen(display);

    uint32 window_width = 1280;
    uint32 window_height = 720;
    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        0, // WIN_X,
        0, // WIN_Y,
        window_width,
        window_height,
        1, // WIN_BORDER,
        BlackPixel(display, screen),
        WhitePixel(display, screen));

    linux_screen_buffer screen_buffer {};
    linux_resize_screen_buffer(&screen_buffer, window_width, window_height);

    // Process window close event through event handler so XNextEvent does not fail
    Atom del_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &del_window, 1);

    // Select kind of events we are interested in
    XSelectInput(display, window,
        ExposureMask |
        KeyPressMask |
        KeyReleaseMask |
        PointerMotionMask |
        ButtonPressMask |
        ButtonReleaseMask);

    // Display the window
    XMapWindow(display, window);

#ifdef ASUKA_DEBUG
    void* base_address = (void*)TERABYTES(1);
#else
    void* base_address = 0;
#endif

    Game_Memory game_memory{};
    game_memory.PermanentStorageSize = MEGABYTES(64);
    game_memory.TransientStorageSize = GIGABYTES(2);

    uint64 total_size = game_memory.PermanentStorageSize + game_memory.TransientStorageSize;

    game_memory.PermanentStorage = os::allocate_pages(total_size);
    game_memory.TransientStorage = (uint8*)game_memory.PermanentStorage + game_memory.PermanentStorageSize;

    running = true;
    uint64 last_counter = os::get_wall_clock();
    uint64 last_cycles = os::get_processor_cycles();

    int gamepad_fds[4] {};
    const char* gamepad_device_paths[] = {
        "/dev/input/js0",
        "/dev/input/js1",
        "/dev/input/js2",
        "/dev/input/js3",
    };

    while (running) {
        for (int gamepad_index = 0; gamepad_index < ARRAY_COUNT(gamepad_fds); gamepad_index++) {
            if (gamepad_fds[gamepad_index] == 0) {
                // Check if new gamepad plugged-in
                int fd = open(gamepad_device_paths[gamepad_index], O_RDONLY | O_NONBLOCK);
                if (fd == -1) continue;

                printf("Found new gamepad id: %d\n", gamepad_index);
                gamepad_fds[gamepad_index] = fd;
            } else {
                int fd = gamepad_fds[gamepad_index];
                // 1. Check if existing gamepad plugged-off
                if (!linux_gamepad_is_plugged_in(fd)) {
                    // Gamepad is plugged off
                    printf("Gamepad %d plugged off\n", gamepad_index);
                    gamepad_fds[gamepad_index] = 0;
                } else {
                    // 2. If gamepad is alive, read events from it:
                    linux_joystick_event joystick_event;
                    while (read(fd, &joystick_event, sizeof(joystick_event)) > 0) {
                        if (joystick_event.type & GAMEPAD_EVENT_INIT) {
                            // These are synthetic events for initialization
                            if (joystick_event.type & GAMEPAD_EVENT_BUTTON) {
                                printf("INIT BUTTON %d\n", joystick_event.number);
                            } else if (joystick_event.type & GAMEPAD_EVENT_AXIS) {
                                printf("INIT AXIS %d\n", joystick_event.number);
                            }
                        } else if (joystick_event.type & GAMEPAD_EVENT_BUTTON) {
                            switch (joystick_event.number) {
                                case 0: printf("A %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 1: printf("B %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 2: printf("X %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 3: printf("Y %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 4: printf("LB %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 5: printf("RB %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 6: printf("Back %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 7: printf("Start %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 8: printf("XBOX %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 9: printf("Left Stick %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                                case 10: printf("Right Stick %s", joystick_event.value ? "PRESS" : "RELEASE"); break;
                            }
                            printf(" BUTTON %d\n", joystick_event.number);
                        } else if (joystick_event.type & GAMEPAD_EVENT_AXIS) {
                            switch (joystick_event.number) {
                                case 0: printf("Left Stick X %d", joystick_event.value); break;
                                case 1: printf("Left Stick Y %d", joystick_event.value); break;
                                case 2: printf("Left Trigger %d", joystick_event.value); break;
                                case 3: printf("Right Stick X %d", joystick_event.value); break;
                                case 4: printf("Right Stick Y %d", joystick_event.value); break;
                                case 5: printf("Right Trigger %d", joystick_event.value); break;
                                case 6: printf("Dpad X %d", joystick_event.value); break;
                                case 7: printf("Dpad Y %d", joystick_event.value); break;
                            }
                            printf(" AXIS %d\n", joystick_event.number);
                        }
                    }
                }
            }
        }

        XEvent event;
        while (XPending(display)) {
            XNextEvent(display, &event);
            switch (event.type) {
                case MotionNotify: {
                    int x = event.xmotion.x;
                    int y = event.xmotion.y;
                    int x_root = event.xmotion.x_root;
                    int y_root = event.xmotion.y_root;

                    printf("Motion event: (%d, %d), root(%d, %d);\n", x, y, x_root, y_root);
                    break;
                }
                case KeyPress:
                    printf("KeyPress\n"); {
                    int x = event.xmotion.x;
                    int y = event.xmotion.y;
                    int x_root = event.xmotion.x_root;
                    int y_root = event.xmotion.y_root;

                    printf("Motion event: (%d, %d), root(%d, %d);\n", x, y, x_root, y_root);
                    break;
                }
                case KeyRelease:
                    printf("KeyRelease\n");
                    break;
                case ButtonPress:
                    printf("ButtonPress\n");
                    break;
                case ButtonRelease:
                    printf("ButtonRelease\n");
                    break;
                case ClientMessage:
                    running = false;
                    break;
                case Expose:
                    int x = event.xexpose.x;
                    int y = event.xexpose.y;
                    int w = event.xexpose.width;
                    int h = event.xexpose.height;
                    // printf("(%d, %d)\n", x + w, y + h);
                    // TODO: get new window buffer size
                    linux_resize_screen_buffer(&screen_buffer, x + w, y + h);
                    break;
            }
        }

        Game_OffscreenBuffer graphics_buffer;
        graphics_buffer.Memory = screen_buffer.memory;
        graphics_buffer.Width = screen_buffer.width;
        graphics_buffer.Height = screen_buffer.height;
        graphics_buffer.Pitch = screen_buffer.width * screen_buffer.bytes_per_pixel;
        graphics_buffer.BytesPerPixel = screen_buffer.bytes_per_pixel;

        Game_UpdateAndRender(&game_memory, NULL, &graphics_buffer, NULL);

        linux_copy_buffer_to_window(&screen_buffer, display, window, screen);

        uint64 counter = os::get_wall_clock();
        uint64 cycles = os::get_processor_cycles();

        uint64 microseconds_elapsed = counter - last_counter;
        uint64 megacycles_elapsed = cycles - last_cycles;

        // printf("milliseconds elapsed: %5.2f\n", microseconds_elapsed / 1000.f);

        last_cycles = cycles;
        last_counter = counter;
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
