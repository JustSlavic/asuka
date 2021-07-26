// BIG TODO FOR PLATFORM LAYER:
//
// - Saved game locations
// - Getting a handle to our own executable file
// - Asset loading path
// - Threading (launch a thread)
// - Raw input (support for multiple keyboards)
// - Sleep / timeBeginPeriod
// - ClipCursor() (multimonitor support)
// - Fullscreen support
// - WM_SETCURSOR (control cursor visibility)
// - QueryCancelAutoplay
// - WM_ACTIVATEAPP (for when we are not the active application)
// - Blit speed improvements (BitBlt)
// - Hardware acceleration (OpenGL or Direct3D)
// - GetKeyboardLayout (for French keyboards, international WASD support)
//
// Just a partial list of stuff
//

#include <defines.hpp>
#include "asuka.hpp"
#include <math.hpp>

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>
#include <math.h>



struct Win32_OffscreenBuffer {
    // Pixels are always 32-bits wide Little Endian, Memory Order BBGGRRxx
    BITMAPINFO Info;
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};


struct Win32_Window_Dimensions {
    int Width;
    int Height;
};


struct Win32_SoundOutput {
    int SamplesPerSecond;
    int ToneHz;
    uint32 RunningSampleIndex;
    int WavePeriod;
    int ChannelCount;
    int BytesPerSample;
    int SecondaryBufferSize;
    float32 SineTime;
    int LatencySampleCount;
};


static bool Running;
static Win32_OffscreenBuffer Global_BackBuffer;
static LPDIRECTSOUNDBUFFER Global_SecondaryBuffer;


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(Win32_XInputGetStateT);
X_INPUT_GET_STATE(Win32_XInputGetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
static Win32_XInputGetStateT* XInputGetState_ = Win32_XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(Win32_XInputSetStateT);
X_INPUT_SET_STATE(Win32_XInputSetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
static Win32_XInputSetStateT* XInputSetState_ = Win32_XInputSetStateStub;
#define XInputSetState XInputSetState_


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter )
typedef DIRECT_SOUND_CREATE(Win32_DirectSoundCreateT);


static void Win32_LoadXInputFunctions() {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");

    if (!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
        // Diagnostic
    }

    if (!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
        // Diagnostic
    }

    if (XInputLibrary) {
        XInputGetState = (Win32_XInputGetStateT*)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (Win32_XInputSetStateT*)GetProcAddress(XInputLibrary, "XInputSetState");
    } else {
        // Diagnostic
    }
}


static void Win32_InitDirectSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize) {
    // Load the library
    HMODULE DirectSoundLibrary = LoadLibraryA("dsound.dll");

    if (DirectSoundLibrary) {
        Win32_DirectSoundCreateT* DirectSoundCreate = (Win32_DirectSoundCreateT*)GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;

        WAVEFORMATEX WaveFormat {};
        WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
        WaveFormat.nChannels = 2;
        WaveFormat.wBitsPerSample = 16;
        WaveFormat.nSamplesPerSec = SamplesPerSecond;
        WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
        WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
        WaveFormat.cbSize = 0;

        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
                DSBUFFERDESC PrimaryBufferDescription {};
                PrimaryBufferDescription.dwSize = sizeof(PrimaryBufferDescription);
                PrimaryBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&PrimaryBufferDescription, &PrimaryBuffer, 0))) {
                        OutputDebugStringA("We successfully created primary buffer!\n");
                    if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) {
                        // Now we finally set the format!
                        OutputDebugStringA("We successfully set format!\n");
                    } else {
                        // Diagnostics
                    }
                } else {
                    // Diagnostics
                }

                // "Create" a secondary buffer
                // Start it playing

                DSBUFFERDESC SecondaryBufferDescription {};
                SecondaryBufferDescription.dwSize = sizeof(SecondaryBufferDescription);
                SecondaryBufferDescription.dwFlags = 0;
                SecondaryBufferDescription.dwBufferBytes = BufferSize;
                SecondaryBufferDescription.lpwfxFormat = &WaveFormat;

                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&SecondaryBufferDescription, &Global_SecondaryBuffer, 0))) {
                        OutputDebugStringA("We successfully created secondary buffer!\n");
                } else {
                    // Diagnostics
                }
            } else {
                // Diagnostics
            }
        } else {
            // Diagnostic
        }
    }
}


static Win32_Window_Dimensions Win32_GetWindowDimention(HWND Window) {
    Win32_Window_Dimensions Result;

    RECT ClientRectangle;
    GetClientRect(Window, &ClientRectangle);

    Result.Width = ClientRectangle.right - ClientRectangle.left;
    Result.Height = ClientRectangle.bottom - ClientRectangle.top;

    return Result;
}


static void Win32_ResizeDIBSection(Win32_OffscreenBuffer* Buffer, LONG Width, LONG Height) {
    if (Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    int BytesPerPixel = 4;

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->Pitch = Width * BytesPerPixel;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // For coordinates be top-down instead of default bottom-up
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32; // 32 bits instead of 24 for memory alignment (1 byte of padding)
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width * Buffer->Height)*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}


static void Win32_CopyBufferToWindow(HDC device_context, int WindowWidth, int WindowHeight, Win32_OffscreenBuffer *Buffer) {
    // TODO: Aspect ratio correction
    StretchDIBits(
        device_context,
        0, 0, WindowWidth, WindowHeight,
        0, 0, Buffer->Width, Buffer->Height,
        Buffer->Memory, &Buffer->Info,
        DIB_RGB_COLORS, SRCCOPY);
}


static void Win32_FillSoundBuffer(Win32_SoundOutput* SoundOutput, DWORD BytesToLock, DWORD BytesToWrite) {
    VOID* Region1;
    DWORD Region1_Size;
    VOID* Region2;
    DWORD Region2_Size;

    // DirectSound output test
    if (SUCCEEDED(Global_SecondaryBuffer->Lock(
        BytesToLock,
        BytesToWrite,
        &Region1,
        &Region1_Size,
        &Region2,
        &Region2_Size,
        0)))
    {
        int16 ToneVolume = 2000;

        {
            DWORD SampleCount = Region1_Size / SoundOutput->BytesPerSample;
            int16* SampleOut = (int16*) Region1;

            for (DWORD SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
                float32 t = SoundOutput->SineTime;
                float32 SineValue = sinf(t);
                int16 SampleValue = (int16)(SineValue * ToneVolume);

                *SampleOut++ = SampleValue;
                *SampleOut++ = SampleValue;

                SoundOutput->SineTime += 2.f * math::consts<float32>::pi() * 1.f / SoundOutput->WavePeriod;
                ++SoundOutput->RunningSampleIndex;
            }
        }

        {
            DWORD SampleCount = Region2_Size / SoundOutput->BytesPerSample;
            int16* SampleOut = (int16*) Region2;

            for (DWORD SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
                float32 t = SoundOutput->SineTime;
                float32 SineValue = sinf(t);
                int16 SampleValue = (int16)(SineValue * ToneVolume);

                *SampleOut++ = SampleValue;
                *SampleOut++ = SampleValue;

                SoundOutput->SineTime += 2.f * math::consts<float32>::pi() * 1.f / SoundOutput->WavePeriod;
                ++SoundOutput->RunningSampleIndex;
            }
        }
    }

    Global_SecondaryBuffer->Unlock(Region1, Region1_Size, Region2, Region2_Size);
}


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result;

    switch (message) {
        case WM_SIZE: {
            OutputDebugStringA("WM_SIZE\n");
            break;
        }
        case WM_MOVE:
            OutputDebugStringA("WM_MOVE\n");
            break;
        case WM_CLOSE:
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
            break;
        case WM_DESTROY:
            Running = false;
            OutputDebugStringA("WM_DESTROY\n");
            break;
        case WM_ACTIVATEAPP:
            OutputDebugStringA("WN_ACTIVATEAPP\n");
            break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            uint32_t VKCode = wParam;
            bool WasDown = (lParam & (1 << 30)) != 0;
            bool IsDown = (lParam & (1 << 31)) == 0;

            if (WasDown != IsDown) {
                if (VKCode == 'W') {
                    OutputDebugStringA("W: ");
                    if (IsDown) {
                        OutputDebugStringA("DOWN\n");
                    }
                    if (WasDown) {
                        OutputDebugStringA("UP\n");
                    }
                } else if (VKCode == 'A') {

                } else if (VKCode == 'S') {

                } else if (VKCode == 'D') {

                } else if (VKCode == 'E') {

                } else if (VKCode == 'Q') {

                } else if (VKCode == VK_UP) {

                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT paint;

            HDC DeviceContext = BeginPaint(Window, &paint);

            Win32_Window_Dimensions WindowSize = Win32_GetWindowDimention(Window);
            Win32_CopyBufferToWindow(DeviceContext, WindowSize.Width, WindowSize.Height, &Global_BackBuffer);

            EndPaint(Window, &paint);

            break;
        }
        default: {
            // OutputDebugStringA("default\n");
            result = DefWindowProcA(Window, message, wParam, lParam);
        }
    }

    return result;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Win32_LoadXInputFunctions();
    WNDCLASSA WindowClass{};

    Win32_ResizeDIBSection(&Global_BackBuffer, 1200, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "AsukaWindowClass";
    // HICON     hIcon;

    LARGE_INTEGER PerfCountFrequency;
    QueryPerformanceFrequency(&PerfCountFrequency);
    int64_t PerformanceFrequency = PerfCountFrequency.QuadPart;

    if (RegisterClassA(&WindowClass)) {
        HWND Window = CreateWindowExA(
            0, // DWORD dwExStyle,
            WindowClass.lpszClassName, // LPCSTR lpClassName,
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

        if (Window) {
            MSG Message;

            int XOffset = 0;
            int YOffset = 0;

            Win32_SoundOutput SoundOutput {};

            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.ToneHz = 256;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
            SoundOutput.ChannelCount = 2;
            SoundOutput.BytesPerSample = sizeof(int16) * SoundOutput.ChannelCount;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15; // We want to fill buffer up to 1/60th of a second

            Win32_InitDirectSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample);
            Win32_FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount); // SoundOutput.SecondaryBufferSize);
            Global_SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            Running = true;
            int FrameCounter = 0;
            bool PrintBatteryInformation = false;
            bool TestGamepadVibration = false;

            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            uint64_t LastCycleCount = __rdtsc();

            while (Running) {
                FrameCounter ++;
                // BOOL messageResult = GetMessage(&Message, 0, 0, 0);

                while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
                    if (Message.message == WM_QUIT) Running = false;
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                XINPUT_STATE InputState;
                for (DWORD GamepadIndex = 0; GamepadIndex < XUSER_MAX_COUNT; GamepadIndex++ ) {
                    auto CONTROLLER_STATE_EC = XInputGetState(0, &InputState);
                    if (CONTROLLER_STATE_EC == ERROR_SUCCESS) {
                        XINPUT_GAMEPAD Gamepad = InputState.Gamepad;

                        bool GamepadDPadUp = Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
                        bool GamepadDPadDown = Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                        bool GamepadDPadLeft = Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                        bool GamepadDPadRight = Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                        bool GamepadStart = Gamepad.wButtons & XINPUT_GAMEPAD_START;
                        bool GamepadBack = Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
                        bool GamepadLeftThumb = Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
                        bool GamepadRightThumb = Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
                        bool GamepadLeftShoulder = Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                        bool GamepadRightShoulder = Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
                        bool GamepadA = Gamepad.wButtons & XINPUT_GAMEPAD_A;
                        bool GamepadB = Gamepad.wButtons & XINPUT_GAMEPAD_B;
                        bool GamepadX = Gamepad.wButtons & XINPUT_GAMEPAD_X;
                        bool GamepadY = Gamepad.wButtons & XINPUT_GAMEPAD_Y;

                        int16 GamepadStickLX = Gamepad.sThumbLX;
                        int16 GamepadStickLY = Gamepad.sThumbLY;

                        uint8 GamepadLeftTrigger = Gamepad.bLeftTrigger;
                        uint8 GamepadRightTrigger = Gamepad.bRightTrigger;

                        XOffset += GamepadStickLX / 4096;
                        YOffset += GamepadStickLY / 4096;

                        SoundOutput.ToneHz = 256 + (int32)(256.f * ((float32)GamepadStickLY / 32767.f));
                        if (SoundOutput.ToneHz == 0) SoundOutput.ToneHz = 1;
                        SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;

                        if (GamepadX) {
                            PrintBatteryInformation = true;
                        } else {
                            PrintBatteryInformation = false;
                        }

                        if (GamepadY) {
                            TestGamepadVibration = true;
                        } else {
                            TestGamepadVibration = false;
                            XINPUT_VIBRATION GamepadVibrationState{};
                            XInputSetState(0, &GamepadVibrationState);
                        }

                        XINPUT_BATTERY_INFORMATION BatteryInformation;
                        if (PrintBatteryInformation) {
                            if (XInputGetBatteryInformation(0, BATTERY_DEVTYPE_GAMEPAD, &BatteryInformation) == ERROR_SUCCESS) {
                                switch (BatteryInformation.BatteryType) {
                                    case BATTERY_TYPE_DISCONNECTED: {
                                        OutputDebugStringA("BATTERY DISCONNECTED\n");
                                    }
                                    break;
                                    case BATTERY_TYPE_WIRED:
                                        OutputDebugStringA("BATTERY WIRED: ");
                                    break;
                                    case BATTERY_TYPE_ALKALINE:
                                        OutputDebugStringA("BATTERY ALKALINE: ");
                                    break;
                                    case BATTERY_TYPE_NIMH:
                                        OutputDebugStringA("BATTERY NIMH: ");
                                    break;
                                    case BATTERY_TYPE_UNKNOWN:
                                    break;
                                }

                                switch (BatteryInformation.BatteryLevel) {
                                    case BATTERY_LEVEL_EMPTY: {
                                        OutputDebugStringA("BATTERY EMPTY\n");
                                    }
                                    break;
                                    case BATTERY_LEVEL_LOW: {
                                        OutputDebugStringA("BATTERY LOW\n");
                                    }
                                    break;
                                    case BATTERY_LEVEL_MEDIUM: {
                                        OutputDebugStringA("BATTERY MEDIUM\n");
                                    }
                                    break;
                                    case BATTERY_LEVEL_FULL: {
                                        OutputDebugStringA("BATTERY FULL\n");
                                    }
                                    break;
                                }
                            } else {
                                // Diagnostics
                            }
                        }

                        if (TestGamepadVibration) {
                            if (FrameCounter % 100 == 0 && FrameCounter % 200 == 0) {
                                XINPUT_VIBRATION GamepadVibrationState {};
                                GamepadVibrationState.wLeftMotorSpeed  = 0;
                                GamepadVibrationState.wRightMotorSpeed = 10000;

                                XInputSetState(0, &GamepadVibrationState);
                            } else if (FrameCounter % 100 == 0 && FrameCounter % 200 != 0) {
                                XINPUT_VIBRATION GamepadVibrationState {};
                                GamepadVibrationState.wLeftMotorSpeed  = 10000;
                                GamepadVibrationState.wRightMotorSpeed = 0;

                                XInputSetState(0, &GamepadVibrationState);
                            }
                        }
                    } else {
                        // Controller is unpluged or other error
                        // OutputDebugStringA("Some error occurred when polling device input state.\n");
                        // if (rc == ERROR_DEVICE_NOT_CONNECTED) {
                        //     OutputDebugStringA("Device is not connected.\n");
                        // }
                    }
                }

                // RenderGradient(&Global_BackBuffer, XOffset++, YOffset);

                Game_OffscreenBuffer Buffer;
                Buffer.Memory = Global_BackBuffer.Memory;
                Buffer.Width = Global_BackBuffer.Width;
                Buffer.Height = Global_BackBuffer.Height;
                Buffer.Pitch = Global_BackBuffer.Pitch;
                Buffer.BytesPerPixel = Global_BackBuffer.BytesPerPixel;

                Game_UpdateAndRender(&Buffer, XOffset++, YOffset);

                DWORD PlayCursor;
                DWORD WriteCursor;
                if (
                    // !SoundPlaying &&
                    SUCCEEDED(Global_SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    DWORD BytesToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
                    DWORD TargetCursor = (PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize;
                    DWORD BytesToWrite = 0;
                    if (BytesToLock == TargetCursor) {
                        BytesToWrite = 0;
                    } else
                    if (BytesToLock > TargetCursor) {
                        BytesToWrite = SoundOutput.SecondaryBufferSize - BytesToLock;
                        BytesToWrite += TargetCursor;
                    } else {
                        BytesToWrite = TargetCursor - BytesToLock;
                    }

                    Win32_FillSoundBuffer(&SoundOutput, BytesToLock, BytesToWrite);
                }

                {
                    HDC DeviceContext = GetDC(Window);

                    Win32_Window_Dimensions WindowSize = Win32_GetWindowDimention(Window);
                    Win32_CopyBufferToWindow(DeviceContext, WindowSize.Width, WindowSize.Height, &Global_BackBuffer);

                    ReleaseDC(Window, DeviceContext);
                }

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);

                uint64_t EndCycleCount = __rdtsc();

                int64_t CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                float32 MilliSecondsElapsed = (1000.f * CounterElapsed) / PerformanceFrequency;

                uint64_t CyclesElapsed = EndCycleCount - LastCycleCount;
                float32 MegaCyclesElapsed = (float32)CyclesElapsed / 1'000'000.f;

                float32 FPS = (float32)PerformanceFrequency / CounterElapsed;

                //char Buffer[256];
                //sprintf(Buffer, "MegaCycles / frame: %f\nMilliseconds / frame: %fms\nFPS: %f\n", MegaCyclesElapsed, MilliSecondsElapsed, FPS);

                //OutputDebugStringA(Buffer);

                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
            }
        } else {
            // Handle error
        }
    } else {
        // Handle error
    }

    return 0;
}
