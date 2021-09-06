#include <defines.hpp>
#include <stdio.h>
#include <X11/Xlib.h>


bool running;


int32 main(int32 argc, char** argv) {
    Display* display;
    Window window;
    XEvent event;
    int screen;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("Cannot open display\n");
        return 1;
    }

    screen = XDefaultScreen(display);

    window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        0, // WIN_X,
        0, // WIN_Y,
        100, // WIN_WIDTH,
        100, // WIN_HEIGHT,
        1, // WIN_BORDER,
        BlackPixel(display, screen),
        WhitePixel(display, screen));

    /* process window close event through event handler so XNextEvent does not fail */
    Atom del_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &del_window, 1);

    /* select kind of events we are interested in */
    XSelectInput(display, window, ExposureMask | KeyPressMask);

    /* display the window */
    XMapWindow(display, window);

    running = true;

    /* event loop */
    while (running) {
            XNextEvent(display, &event);

            switch (event.type) {
                    case KeyPress:
                            /* FALLTHROUGH */
                    case ClientMessage:
                            running = false;
                            break;
                    case Expose:
                            /* draw the window */
                            XFillRectangle(display, window, DefaultGC(display, screen), 20, 20, 10, 10);

                    /* NO DEFAULT */
            }
    }

    /* destroy window */
    XDestroyWindow(display, window);

    /* close connection to server */
    XCloseDisplay(display);

    return 0;
}
