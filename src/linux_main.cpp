#include <defines.hpp>

#include <stdio.h>

#include <X11/Xlib.h>
#include <cerrno>

#include <asuka.hpp>
#include <os/memory.hpp>
#include <os/time.hpp>


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
    XSelectInput(display, window, ExposureMask | KeyPressMask);

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

    while (running) {
        XEvent event;
        XNextEvent(display, &event);

        switch (event.type) {
            case KeyPress:
            case ClientMessage:
                running = false;
                break;
            case Expose:
                // TODO: get new window buffer size
                linux_resize_screen_buffer(&screen_buffer, window_width, window_height);
                break;
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
