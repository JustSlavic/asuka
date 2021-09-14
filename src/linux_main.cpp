#include <defines.hpp>

#include <stdio.h>

#include <X11/Xlib.h>
#include <cerrno>

#include <asuka.hpp>
#include <os/memory.hpp>


struct linux_screen_buffer {
    XImage* x_image;
    void* memory;
    uint32 width;
    uint32 height;
    uint32 bytes_per_pixel;
    uint32 pad;
};


bool running;


static void linux_resize_screen_buffer(linux_screen_buffer* buffer, Display* display, Visual* visual, uint32 width, uint32 height) {
    if (buffer->memory) {
        // TODO: free memory
        os::free_pages(buffer->memory, width * height * buffer->bytes_per_pixel);
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = 4;
    buffer->pad = 32;

    buffer->memory = os::allocate_pages(width * height * buffer->bytes_per_pixel);
    ASSERT(buffer->memory);

    int pitch = width * buffer->bytes_per_pixel;
    uint8* row = (uint8*)buffer->memory;
    for (int y = 0; y < height; y++) {
        uint32* pixel = (uint32*)row;
        for (int x = 0; x < width; x++) {
            uint8 blue = (uint8) x;
            uint8 green = (uint8) y;
            uint8 red = (uint8) 0;
            uint8 alpha = (uint8) 255;

            *pixel = (blue) | (green << 8) | (red << 16) | (alpha << 24);
            pixel++;
        }

        row += pitch;
    }

    uint32 bytes_per_line = width * buffer->bytes_per_pixel;
    buffer->x_image = XCreateImage(
        display,
        visual,
        24,
        ZPixmap,
        0 /* offset */,
        (char*)buffer->memory,
        buffer->width,
        buffer->height,
        buffer->pad,
        0);
    ASSERT(buffer->x_image);
}


int32 main(int32 argc, char** argv) {
    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("Cannot open display\n");
        return 1;
    }

    int screen = XDefaultScreen(display);

    XSetErrorHandler(x11_error_handler);

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
    linux_resize_screen_buffer(&screen_buffer, display, DefaultVisual(display, screen), window_width, window_height);

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

    void* allocated_memory = os::allocate_pages(total_size);
    if (allocated_memory == NULL) {
        printf("%s", os::get_allocate_pages_error());
        return 1;
    }

    game_memory.PermanentStorage = allocated_memory;
    game_memory.TransientStorage = (uint8*)allocated_memory + game_memory.PermanentStorageSize;

    running = true;

    while (running) {
        XEvent event;
        XNextEvent(display, &event);

        switch (event.type) {
            case KeyPress:
            case ClientMessage:
                running = false;
                break;
            case Expose:
                XFillRectangle(display, window, DefaultGC(display, screen), 20, 20, 10, 10);
                XPutImage(display, window, DefaultGC(display, screen), screen_buffer.x_image,
                    0, 0, 0, 0, screen_buffer.width, screen_buffer.height);
        }
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
