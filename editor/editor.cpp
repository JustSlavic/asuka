#include <defines.hpp>

#include <windows.h>


GLOBAL b32 Running;

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result {};

    switch (Message)
    {
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


void Win32_ProcessPendingMessages()
{
    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        if (Message.message == WM_QUIT) Running = false;
        TranslateMessage(&Message);
        DispatchMessageA(&Message);

        // switch (Message.message)
        // {
        //     case WM_MOUSEMOVE:
        //     {
        //     }
        //     break;

        //     case WM_SYSKEYDOWN:
        //     case WM_SYSKEYUP:
        //     case WM_KEYDOWN:
        //     case WM_KEYUP:
        //     {
        //     }
        //     break;

        //     default:
        //     {
        //         DispatchMessageA(&Message);
        //     }
        //     break;
        // }
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


struct ButtonDeclaration
{
    LPSTR ClassName;
};


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
        MessageBoxA(0, "System error! AdjustWindowRect failed.", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
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

    if (Window == NULL)
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! Could not create a window.", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    WNDCLASSA ButtonClass = {};
    ButtonClass.style = CS_HREDRAW | CS_VREDRAW; // UINT
    ButtonClass.lpfnWndProc = NULL;              // WNDPROC
    ButtonClass.cbClsExtra = 0;                  // int
    ButtonClass.cbWndExtra = 0;                  // int
    ButtonClass.hInstance = Instance;            // HINSTANCE
    ButtonClass.hIcon = NULL;                    // HICON
    ButtonClass.hCursor = NULL;                  // HCURSOR
    ButtonClass.hbrBackground = NULL;            // HBRUSH
    ButtonClass.lpszMenuName = NULL;             // LPCSTR
    ButtonClass.lpszClassName = "ButtonClass";   // LPCSTR

    ATOM ButtonClassAtomResult = RegisterClassA(&ButtonClass);
    if (!ButtonClassAtomResult)
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! Could not register button class.", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    HWND Button = CreateWindow(
        "button",  // Predefined class; Unicode assumed
        "OK",      // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
        10,         // x position
        10,         // y position
        100,        // Button width
        100,        // Button height
        Window,     // Parent window
        NULL,       // No menu.
        Instance,
        NULL);      // Pointer not needed.

    if (Button == NULL)
    {
        MessageBeep(MB_ICONERROR);
        MessageBoxA(0, "System error! Could not create button.", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
        return 1;
    }

    Running = true;
    while (Running)
    {
        MSG Message;
        while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
            if (Message.message == WM_QUIT) Running = false;
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }
        Sleep(30);
    }

    return 0;
}
