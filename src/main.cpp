#include <windows.h>


static bool running = false;


LRESULT CALLBACK
MainWindowCallback(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT result;

    switch (message) {
        case WM_MOVE:
            OutputDebugStringA("WM_MOVE\n");
            break;
        case WM_SIZE:
            OutputDebugStringA("WM_SIZE\n");
            break;
        case WM_DESTROY:
            OutputDebugStringA("WM_DESTROY\n");
            break;
        case WM_CLOSE:
            OutputDebugStringA("WM_CLOSE\n");
            running = false;
            break;
        case WM_ACTIVATEAPP:
            OutputDebugStringA("WN_ACTIVATEAPP\n");
            break;
        case WM_PAINT: {
            PAINTSTRUCT paint;

            HDC device_context = BeginPaint(window, &paint);

            LONG width = paint.rcPaint.right - paint.rcPaint.left;
            LONG height = paint.rcPaint.bottom - paint.rcPaint.top;

            static DWORD operation = WHITENESS; //BLACKNESS;
            if (operation == WHITENESS) {
                operation = BLACKNESS;
            } else {
                operation = WHITENESS;
            }

            PatBlt(
                device_context,
                paint.rcPaint.left,
                paint.rcPaint.top,
                width,
                height,
                operation
            );

            EndPaint(window, &paint);

            break;
        }
        default:
            // OutputDebugStringA("default\n");
            result = DefWindowProc(window, message, wParam, lParam);
    }

    return result;
}


int WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    WNDCLASSA windowClass{};

    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWindowCallback;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = "AsukaWindowClass";
    // HICON     hIcon;

    if (RegisterClassA(&windowClass)) {
        HWND windowHandle = CreateWindowExA(
            0, // DWORD dwExStyle,
            windowClass.lpszClassName, // LPCSTR lpClassName,
            "AsukaWindow", // LPCSTR lpWindowName,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // DWORD dwStyle,
            CW_USEDEFAULT, // int X,
            CW_USEDEFAULT, // int Y,
            CW_USEDEFAULT, // int nWidth,
            CW_USEDEFAULT, // int nHeight,
            0, // HWND hWndParent,
            0, // HMENU hMenu,
            hInstance, // HINSTANCE hInstance,
            0 // LPVOID lpParam
        );

        if (windowHandle) {
            MSG message;
            running = true;
            while (running) {
                BOOL messageResult = GetMessage(&message, 0, 0, 0);
                if (messageResult > 0) {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                } else {
                    break;
                }
            }
        } else {
            // Handle error
        }
    } else {
        // Handle error
    }

    return 0;
}
