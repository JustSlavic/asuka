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
#include <os/time.hpp>

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>


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
    uint32 RunningSampleIndex;
    int ChannelCount;
    int BytesPerSample;
    int SecondaryBufferSize;
    int LatencySampleCount;
};

typedef int16 sound_sample;

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


static void Win32_ProcessXInputButton(
    Game_ButtonState* OldState,
    Game_ButtonState* NewState,
    DWORD XInputButtonState,
    DWORD ButtonBit)
{
    NewState->EndedDown = (XInputButtonState & ButtonBit) == ButtonBit;
    NewState->HalfTransitionCount = ((NewState->EndedDown != OldState->EndedDown) ? 1 : 0);
}


static float32 Win32_ProcessXInputStick(int16 value, int16 deadzone) {
    if (value < -deadzone) {
        return (float32)(value + deadzone) / (float32)(32768 - deadzone);
    } else if (value > deadzone) {
        return (float32)(value - deadzone) / (float32)(32767 - deadzone);
    }

    return 0.f;
}


static float32 Win32_ProcessXInputTrigger(uint8 value, uint8 deadzone) {
    if (value < deadzone) {
        return (float32)(value + deadzone) / (float32)(255 - deadzone);
    }

    return 0.f;
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


static void Win32_ClearSoundBuffer(Win32_SoundOutput* SoundOutput) {
    VOID* Region1;
    DWORD Region1_Size;
    VOID* Region2;
    DWORD Region2_Size;

    // DirectSound output test
    if (SUCCEEDED(Global_SecondaryBuffer->Lock(
        0, SoundOutput->SecondaryBufferSize,
        &Region1, &Region1_Size,
        &Region2, &Region2_Size,
        0)))
    {
        uint8* DestMemory = (uint8*)Region1;
        for (int ByteIndex = 0; ByteIndex < Region1_Size; ByteIndex++) {
            *DestMemory++ = 0;
        }
        DestMemory = (uint8*)Region2;
        for (int ByteIndex = 0; ByteIndex < Region2_Size; ByteIndex++) {
            *DestMemory++ = 0;
        }

        Global_SecondaryBuffer->Unlock(Region1, Region1_Size, Region2, Region2_Size);
    }
}


static void Win32_FillSoundBuffer(
    Win32_SoundOutput* SoundOutput, DWORD BytesToLock, DWORD BytesToWrite,
    Game_SoundOutputBuffer* SourceBuffer)
{
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
        int16* SourceSamples = SourceBuffer->Samples;

        {
            DWORD SampleCount = Region1_Size / SoundOutput->BytesPerSample;
            int16* DestSamples = (int16*) Region1;

            for (DWORD SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
                *DestSamples++ = *SourceSamples++;
                *DestSamples++ = *SourceSamples++;
                SoundOutput->RunningSampleIndex++;
            }
        }

        {
            DWORD SampleCount = Region2_Size / SoundOutput->BytesPerSample;
            int16* DestSamples = (int16*) Region2;

            for (DWORD SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
                *DestSamples++ = *SourceSamples++;
                *DestSamples++ = *SourceSamples++;
                SoundOutput->RunningSampleIndex++;
            }
        }

        Global_SecondaryBuffer->Unlock(Region1, Region1_Size, Region2, Region2_Size);
    }
}


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result;

    switch (message) {
        case WM_SIZE: {
            OutputDebugStringA("WM_SIZE\n");
            break;
        }
        case WM_MOVE: {
            OutputDebugStringA("WM_MOVE\n");
            break;
        }
        case WM_CLOSE: {
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
            break;
        }
        case WM_DESTROY: {
            Running = false;
            OutputDebugStringA("WM_DESTROY\n");
            break;
        }
        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WN_ACTIVATEAPP\n");
            break;
        }
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            OutputDebugStringA("SYSKEY HANDLING IN MAIN WINDOW CALLBACK\n");
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

    int64 PerformanceFrequency = os::get_performance_counter_frequency();

    ATOM ClassAtomResult = RegisterClassA(&WindowClass);
    if (!ClassAtomResult) {
        // Handle error
    }

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

    if (!Window) {
        // Handle error
    }

    Win32_SoundOutput SoundOutput {};

    SoundOutput.SamplesPerSecond = 48000;
    SoundOutput.RunningSampleIndex = 0;
    SoundOutput.ChannelCount = 2;
    SoundOutput.BytesPerSample = sizeof(int16) * SoundOutput.ChannelCount;
    SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
    SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 30; // We want to fill buffer up to 1/60th of a second

    Win32_InitDirectSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
    Win32_ClearSoundBuffer(&SoundOutput);
    Global_SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

    int16* Samples = (int16*) VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#ifdef ASUKA_DEBUG
    LPVOID BaseAddress = (LPVOID)TERABYTES(1);
#else
    LPVOID BaseAddress = 0;
#endif

    Game_Memory GameMemory{};
    GameMemory.PermanentStorageSize = MEGABYTES(64);
    GameMemory.TransientStorageSize = GIGABYTES(2);

    uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    GameMemory.TransientStorage = (uint8*)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;

    Running = true;
    int FrameCounter = 0;
    bool SoundIsValid = false;

    int64 LastCounter = os::get_performance_counter();
    uint64 LastCycleCount = os::get_processor_cycles();

    Game_Input Input[2] {};
    Game_Input* OldInput = &Input[0];
    Game_Input* NewInput = &Input[1];

    while (Running) {
        FrameCounter ++;

        MSG Message;
        while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
            if (Message.message == WM_QUIT) Running = false;
            TranslateMessage(&Message);

            switch (Message.message) {
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP: {
                    OutputDebugStringA("SYSKEY MY HANDLING\n");
                    uint32 VKCode = Message.wParam;
                    bool WasDown = (Message.lParam & (1 << 30)) != 0;
                    bool IsDown = (Message.lParam & (1 << 31)) == 0;

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
                default: {
                    DispatchMessageA(&Message);
                    break;
                }
            }
        }

        DWORD MaxControllerCount = XUSER_MAX_COUNT;
        if (MaxControllerCount > ARRAY_COUNT(Input[0].Controllers)) {
            MaxControllerCount = ARRAY_COUNT(Input[0].Controllers);
        }

        for (DWORD GamepadIndex = 0; GamepadIndex < MaxControllerCount; GamepadIndex++ ) {
            XINPUT_STATE InputState;
            auto CONTROLLER_STATE_EC = XInputGetState(0, &InputState);
            if (CONTROLLER_STATE_EC == ERROR_SUCCESS) {
                XINPUT_GAMEPAD Gamepad = InputState.Gamepad;

                Game_ControllerInput* OldGamepadInput = &OldInput->Controllers[GamepadIndex];
                Game_ControllerInput* NewGamepadInput = &NewInput->Controllers[GamepadIndex];

                NewGamepadInput->IsAnalog = true;

                Win32_ProcessXInputButton(&OldGamepadInput->A, &NewGamepadInput->A, Gamepad.wButtons, XINPUT_GAMEPAD_A);
                Win32_ProcessXInputButton(&OldGamepadInput->B, &NewGamepadInput->B, Gamepad.wButtons, XINPUT_GAMEPAD_B);
                Win32_ProcessXInputButton(&OldGamepadInput->X, &NewGamepadInput->X, Gamepad.wButtons, XINPUT_GAMEPAD_X);
                Win32_ProcessXInputButton(&OldGamepadInput->Y, &NewGamepadInput->Y, Gamepad.wButtons, XINPUT_GAMEPAD_Y);

                Win32_ProcessXInputButton(&OldGamepadInput->DpadUp, &NewGamepadInput->DpadUp, Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP);
                Win32_ProcessXInputButton(&OldGamepadInput->DpadDown, &NewGamepadInput->DpadDown, Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
                Win32_ProcessXInputButton(&OldGamepadInput->DpadLeft, &NewGamepadInput->DpadLeft, Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
                Win32_ProcessXInputButton(&OldGamepadInput->DpadRight, &NewGamepadInput->DpadRight, Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);

                Win32_ProcessXInputButton(&OldGamepadInput->ShoulderLeft, &NewGamepadInput->ShoulderLeft, Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
                Win32_ProcessXInputButton(&OldGamepadInput->ShoulderRight, &NewGamepadInput->ShoulderRight, Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);

                Win32_ProcessXInputButton(&OldGamepadInput->StickLeft, &NewGamepadInput->StickLeft, Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
                Win32_ProcessXInputButton(&OldGamepadInput->StickRight, &NewGamepadInput->StickRight, Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);

                Win32_ProcessXInputButton(&OldGamepadInput->Back, &NewGamepadInput->Back, Gamepad.wButtons, XINPUT_GAMEPAD_BACK);
                Win32_ProcessXInputButton(&OldGamepadInput->Start, &NewGamepadInput->Start, Gamepad.wButtons, XINPUT_GAMEPAD_START);

                NewGamepadInput->StickLXStarted = OldGamepadInput->StickLXEnded;
                NewGamepadInput->StickLXEnded = Win32_ProcessXInputStick(Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                NewGamepadInput->StickLYStarted = OldGamepadInput->StickLYEnded;
                NewGamepadInput->StickLYEnded = Win32_ProcessXInputStick(Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                NewGamepadInput->StickRXStarted = OldGamepadInput->StickRXEnded;
                NewGamepadInput->StickRXEnded = Win32_ProcessXInputStick(Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

                NewGamepadInput->StickRYStarted = OldGamepadInput->StickRYEnded;
                NewGamepadInput->StickRYEnded = Win32_ProcessXInputStick(Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

                NewGamepadInput->TriggerLeftStarted = OldGamepadInput->TriggerLeftEnded;
                NewGamepadInput->TriggerLeftEnded = Win32_ProcessXInputTrigger(Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

                NewGamepadInput->TriggerRightStarted = OldGamepadInput->TriggerRightEnded;
                NewGamepadInput->TriggerRightEnded = Win32_ProcessXInputTrigger(Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

                // XINPUT_BATTERY_INFORMATION BatteryInformation;
                // if (PrintBatteryInformation) {
                //     if (XInputGetBatteryInformation(0, BATTERY_DEVTYPE_GAMEPAD, &BatteryInformation) == ERROR_SUCCESS) {
                //         switch (BatteryInformation.BatteryType) {
                //             case BATTERY_TYPE_DISCONNECTED: {
                //                 OutputDebugStringA("BATTERY DISCONNECTED\n");
                //             }
                //             break;
                //             case BATTERY_TYPE_WIRED:
                //                 OutputDebugStringA("BATTERY WIRED: ");
                //             break;
                //             case BATTERY_TYPE_ALKALINE:
                //                 OutputDebugStringA("BATTERY ALKALINE: ");
                //             break;
                //             case BATTERY_TYPE_NIMH:
                //                 OutputDebugStringA("BATTERY NIMH: ");
                //             break;
                //             case BATTERY_TYPE_UNKNOWN:
                //             break;
                //         }

                //         switch (BatteryInformation.BatteryLevel) {
                //             case BATTERY_LEVEL_EMPTY: {
                //                 OutputDebugStringA("BATTERY EMPTY\n");
                //             }
                //             break;
                //             case BATTERY_LEVEL_LOW: {
                //                 OutputDebugStringA("BATTERY LOW\n");
                //             }
                //             break;
                //             case BATTERY_LEVEL_MEDIUM: {
                //                 OutputDebugStringA("BATTERY MEDIUM\n");
                //             }
                //             break;
                //             case BATTERY_LEVEL_FULL: {
                //                 OutputDebugStringA("BATTERY FULL\n");
                //             }
                //             break;
                //         }
                //     } else {
                //         // Diagnostics
                //     }
                // }

                // if (TestGamepadVibration) {
                //     if (FrameCounter % 100 == 0 && FrameCounter % 200 == 0) {
                //         XINPUT_VIBRATION GamepadVibrationState {};
                //         GamepadVibrationState.wLeftMotorSpeed  = 0;
                //         GamepadVibrationState.wRightMotorSpeed = 10000;

                //         XInputSetState(0, &GamepadVibrationState);
                //     } else if (FrameCounter % 100 == 0 && FrameCounter % 200 != 0) {
                //         XINPUT_VIBRATION GamepadVibrationState {};
                //         GamepadVibrationState.wLeftMotorSpeed  = 10000;
                //         GamepadVibrationState.wRightMotorSpeed = 0;

                //         XInputSetState(0, &GamepadVibrationState);
                //     }
                // }
            } else {
                // Controller is unpluged or other error
                // OutputDebugStringA("Some error occurred when polling device input state.\n");
                // if (rc == ERROR_DEVICE_NOT_CONNECTED) {
                //     OutputDebugStringA("Device is not connected.\n");
                // }
            }
        }

        // RenderGradient(&Global_BackBuffer, XOffset++, YOffset);

        DWORD PlayCursor;
        DWORD WriteCursor;

        DWORD ByteToLock;
        DWORD TargetCursor;
        DWORD BytesToWrite;

        if (SUCCEEDED(Global_SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
        {
            if (SoundIsValid) {
                ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
            } else {
                ByteToLock = WriteCursor;
                SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
            }

            // TargetCursor = (PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize;
            // BytesToWrite = 0;
            TargetCursor = (WriteCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize;

            if (ByteToLock == TargetCursor) {
                BytesToWrite = 0;
            } else if (ByteToLock > TargetCursor) {
                BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
                BytesToWrite += TargetCursor;
            } else {
                BytesToWrite = TargetCursor - ByteToLock;
            }

            SoundIsValid = true;
        }

        char print_buffer[1024];
        sprintf(print_buffer, "PC: %d\nWC: %d\nBL: %d\nBW: %d\n\n", PlayCursor, WriteCursor, ByteToLock, BytesToWrite);
        OutputDebugString(print_buffer);

        // if (FrameCounter == 10) Running = false;

        Game_SoundOutputBuffer SoundBuffer {};
        SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
        SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
        SoundBuffer.Samples = Samples;

        Game_OffscreenBuffer Buffer;
        Buffer.Memory = Global_BackBuffer.Memory;
        Buffer.Width  = Global_BackBuffer.Width;
        Buffer.Height = Global_BackBuffer.Height;
        Buffer.Pitch  = Global_BackBuffer.Pitch;
        Buffer.BytesPerPixel = Global_BackBuffer.BytesPerPixel;

        Game_UpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);

        if (SoundIsValid) {
            Win32_FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
        }

        {
            HDC DeviceContext = GetDC(Window);

            Win32_Window_Dimensions WindowSize = Win32_GetWindowDimention(Window);
            Win32_CopyBufferToWindow(DeviceContext, WindowSize.Width, WindowSize.Height, &Global_BackBuffer);

            ReleaseDC(Window, DeviceContext);
        }

        int64 EndCounter = os::get_performance_counter();
        uint64 EndCycleCount = os::get_processor_cycles();

        int64 CounterElapsed = EndCounter - LastCounter;
        float32 MilliSecondsElapsed = (1000.f * CounterElapsed) / PerformanceFrequency;

        uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
        float32 MegaCyclesElapsed = (float32)CyclesElapsed / 1'000'000.f;

        float32 FPS = (float32)PerformanceFrequency / CounterElapsed;

        //char Buffer[256];
        //sprintf(Buffer, "MegaCycles / frame: %f\nMilliseconds / frame: %fms\nFPS: %f\n", MegaCyclesElapsed, MilliSecondsElapsed, FPS);

        //OutputDebugStringA(Buffer);

        LastCounter = EndCounter;
        LastCycleCount = EndCycleCount;

        Game_Input* TmpInput = NewInput;
        NewInput = OldInput;
        OldInput = TmpInput;
    }

    return 0;
}
