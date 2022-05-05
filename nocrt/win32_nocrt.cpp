#include <defines.hpp>

#include <windows.h>

#include "win32_crt_float.cpp"
#include "win32_crt_memory.cpp"

GLOBAL b32 Running;

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam) {
    LRESULT Result {};

    switch (Message) {
        case WM_SIZE:
        {
        }
        break;

        case WM_MOVE:
        {
        }
        break;

        case WM_CLOSE:
        {
            Running = false;
        }
        break;

        case WM_DESTROY:
        {
            Running = false;
        }
        break;

        case WM_ACTIVATEAPP:
        {
        }
        break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
        }
        break;

        default:
        {
            Result = DefWindowProcA(Window, Message, wParam, lParam);
        }
    }

    return Result;
}


void Win32_ProcessPendingMessages() {
    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
        if (Message.message == WM_QUIT) Running = false;
        TranslateMessage(&Message);

        switch (Message.message) {
            case WM_MOUSEMOVE:
                break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
                break;
            default: {
                DispatchMessageA(&Message);
                break;
            }
        }
    }
}


Int32 Width(RECT Rect)
{
    Int32 Result = Rect.right - Rect.left;
    return Result;
}


Int32 Height(RECT Rect)
{
    Int32 Result = Rect.bottom - Rect.top;
    return Result;
}


Int32 WinMain(HINSTANCE Instance,
              HINSTANCE PrevInstance,
              LPSTR CommandLine,
              Int32 ShowCode)
{
    WNDCLASSA WindowClass {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "WindowClass";
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = NULL;

    ATOM ClassAtomResult = RegisterClassA(&WindowClass);
    if (!ClassAtomResult)
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! Could not register window class.", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    Int32 ClientWidth  = 800;
    Int32 ClientHeight = 600;
    RECT WindowRectangle { 0, 0, ClientWidth, ClientHeight };
    if (!AdjustWindowRect(&WindowRectangle, WS_OVERLAPPEDWINDOW, false))
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! AdjustWindowRect failed.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    HWND Window = CreateWindowExA(
        0,                                // ExStyle
        WindowClass.lpszClassName,        // ClassName
        "",                               // WindowName
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Style
        CW_USEDEFAULT,                    // X,
        CW_USEDEFAULT,                    // Y,
        Width(WindowRectangle),           // Width
        Height(WindowRectangle),          // Height
        0,                                // WndParent
        0,                                // Menu
        Instance,                         // Instance
        NULL);                            // Param

    if (!Window)
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! Could not create a window.", "Asuka Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    Running = true;
    while (Running)
    {
        Win32_ProcessPendingMessages();
    }

    return 0;
}


void __stdcall WinMainCRTStartup()
{
    Int32 rc = WinMain(GetModuleHandle(0), 0, 0, 0);
    ExitProcess(rc);
}
