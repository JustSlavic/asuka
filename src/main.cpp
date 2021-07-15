#include <windows.h>


static bool running = false;
static BITMAPINFO BitmapInfo{};
static void* BitmapMemory{};
HBITMAP BitmapHandle{};
HDC BitmapDeviceContext;

static void
ResizeDIBSection(LONG width, LONG height)
{
    // Maybe don't free first, free after, then free first if second succeed

    // todo: Free first DIBSection
    if (BitmapHandle) {
        DeleteObject(BitmapHandle);
    }

    if (!BitmapDeviceContext) {
        // Should we recreate this in the certain circumstances
        BitmapDeviceContext = CreateCompatibleDC(0);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = width;
    BitmapInfo.bmiHeader.biHeight = height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    BitmapHandle = CreateDIBSection(
        BitmapDeviceContext,
        &BitmapInfo,
        DIB_RGB_COLORS,
        &BitmapMemory,
        0, 0);
}


static void
Win32UpdateWindow(HDC device_context, int x, int y, int width, int height)
{
    StretchDIBits(
        device_context,
        x, y, width, height,
        x, y, width, height,
        BitmapMemory, &BitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK
MainWindowCallback(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT result;

    switch (message) {
        case WM_SIZE: {
            RECT client_rectangle;
            GetClientRect(
              window,
              &client_rectangle
            );

            LONG width = client_rectangle.right - client_rectangle.left;
            LONG height = client_rectangle.bottom - client_rectangle.top;

            ResizeDIBSection(width, height);
            OutputDebugStringA("WM_SIZE\n");
            break;
        }
        case WM_MOVE:
            OutputDebugStringA("WM_MOVE\n");
            break;
        case WM_CLOSE:
            running = false;
            OutputDebugStringA("WM_CLOSE\n");
            break;
        case WM_DESTROY:
            running = false;
            OutputDebugStringA("WM_DESTROY\n");
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

    // windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
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
