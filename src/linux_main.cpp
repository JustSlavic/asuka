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

#define GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define GAMEPAD_TRIGGER_DEADZONE     7710


static bool global_running;
static const char* global_gamepad_device_paths[] = {
    "/dev/input/js0",
    "/dev/input/js1",
    "/dev/input/js2",
    "/dev/input/js3",
};


struct linux_gamepad {
    int fd;
};


static bool32 linux_gamepad_connect(linux_gamepad* gamepad, const char* device_path) {
    int fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) return false;

    gamepad->fd = fd;
    return true;
}


static void linux_gamepad_disconnect(linux_gamepad* gamepad) {
    gamepad->fd = 0;
}


static bool32 linux_gamepad_connected(linux_gamepad* gamepad) {
    return gamepad->fd > 0;
}


static bool32 linux_gamepad_lost_connection(linux_gamepad* gamepad) {
    struct stat statbuf;
    int ec = fstat(gamepad->fd, &statbuf);

    bool32 result = (ec == 0) && (statbuf.st_nlink > 0);
    return !result;
}


struct linux_joystick_event {
    uint32 time;   /* event timestamp in milliseconds */
    int16  value;  /* value */
    uint8  type;   /* event type */
    uint8  number; /* axis/button number */
};


static bool32 linux_gamepad_next_event(linux_gamepad* gamepad, linux_joystick_event* event) {
    int64 bytes = read(gamepad->fd, event, sizeof(linux_joystick_event));
    // note: returns -1 when read is not successful
    return bytes > 0;
}


static void linux_process_controller_button(Game_ButtonState* Button, int16 Value) {
    Button->HalfTransitionCount += 1;
    Button->EndedDown = (Value == 1);
}


static float32 linux_controller_process_stick(int16 value, int16 deadzone) {
    if (value < -deadzone) {
        return ((float32)value + (float32)deadzone) / ((float32)32767 - (float32)deadzone);
    } else if (value > deadzone) {
        return ((float32)value - (float32)deadzone) / ((float32)32767 - (float32)deadzone);
    }

    return 0.f;
}


static void linux_process_controller_axis(float* Axis, int16 Value) {
    if (Value > 0) {
        *Axis = (float32)Value / INT16_MAX;
    } else {
        *Axis = (float32)Value / INT16_MIN;
    }
}


static float32 linux_process_controller_trigger(int16 value, int16 deadzone) {
    uint16 value_ = value + 32767;
    if (value_ > deadzone) {
        return ((float32)value_ - (float32)deadzone) / ((float32)65534 - (float32)deadzone);
    }

    return 0.f;
}


static void linux_gamepad_process_events(linux_gamepad* device, Game_ControllerInput* Controller) {
    linux_joystick_event event;
    while (linux_gamepad_next_event(device, &event)) {
        if (event.type & GAMEPAD_EVENT_INIT) {
            // These are synthetic events for initialization
            if (event.type & GAMEPAD_EVENT_BUTTON) {
                // printf("INIT BUTTON %d\n", event.number);
            } else if (event.type & GAMEPAD_EVENT_AXIS) {
                // printf("INIT AXIS %d\n", event.number);
            }
        } else if (event.type & GAMEPAD_EVENT_BUTTON) {
            switch (event.number) {
                case 0: linux_process_controller_button(&Controller->A, event.value); break;
                case 1: linux_process_controller_button(&Controller->B, event.value); break;
                case 2: linux_process_controller_button(&Controller->X, event.value); break;
                case 3: linux_process_controller_button(&Controller->Y, event.value); break;
                case 4: linux_process_controller_button(&Controller->ShoulderLeft, event.value); break;
                case 5: linux_process_controller_button(&Controller->ShoulderRight, event.value); break;
                case 6: linux_process_controller_button(&Controller->Back, event.value); break;
                case 7: linux_process_controller_button(&Controller->Start, event.value); break;
                // case 8: linux_process_controller_button(&Controller->XBox, event.value); printf("XBOX %s", event.value ? "PRESS" : "RELEASE"); break;
                case 9: linux_process_controller_button(&Controller->StickLeft, event.value); break;
                case 10: linux_process_controller_button(&Controller->StickRight, event.value);break;
            }
        } else if (event.type & GAMEPAD_EVENT_AXIS) {
            switch (event.number) {
                case 0: {
                    Controller->StickLXEnded = linux_controller_process_stick(event.value, GAMEPAD_LEFT_THUMB_DEADZONE);
                    break;
                }
                case 1: {
                    Controller->StickLYEnded = linux_controller_process_stick(-event.value, GAMEPAD_LEFT_THUMB_DEADZONE);
                    break;
                }
                case 2: {
                    Controller->TriggerLeftEnded = linux_process_controller_trigger(event.value, GAMEPAD_TRIGGER_DEADZONE);
                    break;
                }
                case 3: {
                    Controller->StickRXEnded = linux_controller_process_stick(event.value, GAMEPAD_RIGHT_THUMB_DEADZONE);
                    break;
                }
                case 4: {
                    Controller->StickRYEnded = linux_controller_process_stick(-event.value, GAMEPAD_RIGHT_THUMB_DEADZONE);
                    break;
                }
                case 5: {
                    Controller->TriggerRightEnded = linux_process_controller_trigger(event.value, GAMEPAD_TRIGGER_DEADZONE);
                    break;
                }
                case 6: printf("Dpad X %d\n", event.value); break;
                case 7: printf("Dpad Y %d\n", event.value); break;
            }
        }
    }
}


struct linux_screen_buffer {
    XImage x_image;
    void* memory;
    uint32 width;
    uint32 height;
    uint32 bytes_per_pixel;
};


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

    Game_Input Input;

    global_running = true;
    uint64 last_counter = os::get_wall_clock();
    uint64 last_cycles = os::get_processor_cycles();

    linux_gamepad gamepad_devices[ARRAY_COUNT(global_gamepad_device_paths)];

    while (global_running) {
        for (int gamepad_index = 0; gamepad_index < ARRAY_COUNT(gamepad_devices); gamepad_index++) {
            Game_ControllerInput* Controller = &Input.Controllers[gamepad_index];
            linux_gamepad* device = &gamepad_devices[gamepad_index];

            if (!linux_gamepad_connected(device)) {
                // Check if new gamepad is connected
                bool32 success = linux_gamepad_connect(device, global_gamepad_device_paths[gamepad_index]);
                if (!success) continue;

                // Success: new gamepad is connected
                printf("Found new gamepad id: %d\n", gamepad_index);

                // @todo: Should I read events right away?
            } else {
                // Checking if already connected gamepad is plugged-off (by the player, by battery running out, or another reason)
                if (linux_gamepad_lost_connection(device)) {
                    // Gamepad is plugged off
                    linux_gamepad_disconnect(device);
                    printf("Gamepad %d plugged off\n", gamepad_index);
                } else {
                    // 2. If gamepad is alive, read events from it:
                    linux_gamepad_process_events(device, Controller);
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
                    global_running = false;
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

        Game_UpdateAndRender(&game_memory, &Input, &graphics_buffer, NULL);

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
