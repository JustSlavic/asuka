#include <defines.hpp>

#include <windows.h>
#include <xaudio2.h>

#include <stdlib.h>
#include <math.h>

#define PI 3.1415926535897932384626433f


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


void ErrorBeepBoop(char *Message)
{
    MessageBeep(MB_ICONERROR);
    MessageBoxA(0, Message, "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
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
        ErrorBeepBoop("System error! Could not register window class.");
        return 1;
    }

    Int32 ClientWidth  = 800;
    Int32 ClientHeight = 600;
    RECT WindowRectangle { 0, 0, ClientWidth, ClientHeight };
    if (!AdjustWindowRect(&WindowRectangle, WS_OVERLAPPEDWINDOW, false))
    {
        ErrorBeepBoop("System error! AdjustWindowRect failed.");
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
        ErrorBeepBoop("System error! Could not create a window.");
        return 1;
    }

    IXAudio2 *AudioDevice = NULL;
    HRESULT Result = XAudio2Create(&AudioDevice, 0, 0);
    if (FAILED(Result))
    {
        ErrorBeepBoop("Cannot initialize XAudio2 Device!");
        return 1;
    }

    defer { AudioDevice->Release(); };


    // Sound
    i32 SamplesPerSecond = 48000;
    i32 RunningSoundCursor = 0;
    i32 ChannelCount = 2;

    IXAudio2MasteringVoice *MasteringVoice = NULL;
    Result = AudioDevice->CreateMasteringVoice(
        &MasteringVoice,
        ChannelCount,
        SamplesPerSecond);

    i32 BytesPerSoundFrame = sizeof(sound_sample_t) * ChannelCount;
    i32 SoundBufferSize = SamplesPerSecond * BytesPerSoundFrame; // 1 sec of sound

    sound_sample_t *SoundBuffer = (sound_sample_t *) malloc(SoundBufferSize);

    f32 SineFreqHz = 60; // 60 Hz
    f32 tSound = 0.0f;

    f32 SamplesPerPeriod = SamplesPerSecond / SineFreqHz;

    sound_sample_t *Sample = SoundBuffer;
    for (i32 i = 0; i < SoundBufferSize; i += 2)
    {
        sound_sample_t SineValue = (sound_sample_t)sinf(tSound);
        *Sample++ = SineValue;
        *Sample++ = SineValue;

        tSound += 2 * PI / SamplesPerPeriod;
        if (tSound > 2 * PI)
        {
            tSound -= 2 * PI;
        }
    }

    Running = true;
    while (Running)
    {
        Win32_ProcessPendingMessages();
    }

    return 0;
}
