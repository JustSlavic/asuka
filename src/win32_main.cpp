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
#include <debug/casts.hpp>

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>


struct Win32_GameDLL {
    HMODULE GameDLL;
    Game_UpdateAndRenderT* UpdateAndRender;
    FILETIME Timestamp;

    bool32 IsValid;
};


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


typedef int16 sound_sample_t;


struct Win32_SoundOutput {
    uint32 SamplesPerSecond;
    uint32 RunningSoundCursor;
    uint32 ChannelCount;
    uint32 BytesPerSoundFrame;
    uint32 SecondaryBufferSize;
    uint32 SafetyBytes;
    sound_sample_t* Samples;
};


#ifdef ASUKA_DEBUG
struct Win32_DebugSoundCursors {
    uint32 PlayCursor;
    uint32 WriteCursor;

    uint32 OutputLocationStart;
    uint32 OutputLocationEnd;

    uint32 PageFlip;
    uint32 ExpectedNextPageFlip;
};


enum Win32_DebugInputRecordingState {
    INPUT_RECORDING_IDLE,
    INPUT_RECORDING_RECORD,
    INPUT_RECORDING_PLAYBACK,
};


struct Win32_DebugInputRecording {
    Game_State InitialGameState;
    uint64 InputRecordingSize;
    void * InputRecording;
    uint64 RecordedInputsCount;
    uint64 CurrentPlaybackInputIndex;
    Win32_DebugInputRecordingState RecordingState;
};

static Win32_DebugInputRecording Global_DebugInputRecording;
#endif // ASUKA_DEBUG


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


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
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


static FILETIME Win32_GetFileTimestamp(const char* Filename) {
    FILETIME Result {};

    WIN32_FILE_ATTRIBUTE_DATA FileData;
    GetFileAttributesExA(Filename, GetFileExInfoStandard, &FileData);
    Result = FileData.ftLastWriteTime;

    return Result;
}


static Win32_GameDLL Win32_LoadGameDLL(const char* DllPath, const char* TempDllPath) {
#if defined(ASUKA_DLL_BUILD)
    Win32_GameDLL Result {};

    Result.Timestamp = Win32_GetFileTimestamp(DllPath);
    CopyFile(DllPath, TempDllPath, FALSE);
    Result.GameDLL = LoadLibraryA(TempDllPath);

    if (Result.GameDLL) {
        Result.UpdateAndRender = (Game_UpdateAndRenderT*)GetProcAddress(Result.GameDLL, "Game_UpdateAndRender");

        Result.IsValid = (Result.UpdateAndRender != NULL);
    }

    return Result;
#else
    Win32_GameDLL Result {};
    Result.IsValid = true;
    Result.UpdateAndRender = Game_UpdateAndRender;

    return Result;
#endif
}


static void Win32_UnloadGameDLL(Win32_GameDLL* GameCode) {
#if defined(ASUKA_DLL_BUILD)
    if (GameCode->GameDLL) {
        FreeLibrary(GameCode->GameDLL);
        GameCode->GameDLL = NULL;
    }

    GameCode->IsValid = false;
    GameCode->UpdateAndRender = NULL;
#endif
}


static void Win32_ProcessKeyboardEvent(Game_ButtonState* NewState, bool32 IsDown) {
    ASSERT(NewState->EndedDown != IsDown);
    NewState->EndedDown = IsDown;
    NewState->HalfTransitionCount++;
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
        WaveFormat.wBitsPerSample = sizeof(sound_sample_t) * 8;
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
    Buffer->BytesPerPixel = BytesPerPixel;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // For coordinates be top-down instead of default bottom-up
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32; // 32 bits instead of 24 for memory alignment (1 byte of padding)
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width * Buffer->Height)*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}


static void Win32_CopyBufferToWindow(Win32_OffscreenBuffer *Buffer, HDC device_context, int WindowWidth, int WindowHeight) {
    StretchDIBits(
        device_context,
        0, 0, Buffer->Width, Buffer->Height,
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
        for (DWORD ByteIndex = 0; ByteIndex < Region1_Size; ByteIndex++) {
            *DestMemory++ = 0;
        }
        DestMemory = (uint8*)Region2;
        for (DWORD ByteIndex = 0; ByteIndex < Region2_Size; ByteIndex++) {
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
        sound_sample_t* SourceSamples = SourceBuffer->Samples;

        {
            DWORD SampleCount = Region1_Size / SoundOutput->BytesPerSoundFrame;
            sound_sample_t* DestSamples = (sound_sample_t*) Region1;

            for (DWORD SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
                *DestSamples++ = *SourceSamples++;
                *DestSamples++ = *SourceSamples++;
                SoundOutput->RunningSoundCursor += 2 * sizeof(sound_sample_t);
            }
        }

        {
            DWORD SampleCount = Region2_Size / SoundOutput->BytesPerSoundFrame;
            sound_sample_t* DestSamples = (sound_sample_t*) Region2;

            for (DWORD SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
                *DestSamples++ = *SourceSamples++;
                *DestSamples++ = *SourceSamples++;
                SoundOutput->RunningSoundCursor += 2 * sizeof(sound_sample_t);
            }
        }

        Global_SecondaryBuffer->Unlock(Region1, Region1_Size, Region2, Region2_Size);
    }
}


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result {};

    switch (message) {
        case WM_SIZE: {
            break;
        }
        case WM_MOVE: {
            break;
        }
        case WM_CLOSE: {
            Running = false;
            break;
        }
        case WM_DESTROY: {
            Running = false;
            break;
        }
        case WM_ACTIVATEAPP: {
            if (wParam == TRUE) {
                LONG_PTR SetExtendedStyleResult = SetWindowLongPtrA(Window, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED);
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            } else {
                LONG_PTR SetExtendedStyleResult = SetWindowLongPtrA(Window, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT);
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
            break;
        }
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            ASSERT_FAIL("Key handling happens in the main loop.");
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT paint;

            HDC DeviceContext = BeginPaint(Window, &paint);

            Win32_Window_Dimensions WindowSize = Win32_GetWindowDimention(Window);
            Win32_CopyBufferToWindow(&Global_BackBuffer, DeviceContext, WindowSize.Width, WindowSize.Height);

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


void Win32_ProcessPendingMessages(Game_ControllerInput* KeyboardController, Game_MouseState* Mouse) {
    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
        if (Message.message == WM_QUIT) Running = false;
        TranslateMessage(&Message);

        switch (Message.message) {
            case WM_MOUSEMOVE: {
                Mouse->X = (Message.lParam & 0x0000FFFF);
                Mouse->Y = (Message.lParam & 0xFFFF0000) >> 16;
                break;
            }
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                uint32 VKCode = (uint32)Message.wParam;
                bool WasDown = (Message.lParam & (1 << 30)) != 0;
                bool IsDown = (Message.lParam & (1 << 31)) == 0;

                if (WasDown != IsDown) {
                    if (VKCode == VK_ESCAPE) {
                        Running = false;
                        Win32_ProcessKeyboardEvent(&KeyboardController->Back, IsDown);
                    } else if (VKCode == VK_SPACE) {
                        Win32_ProcessKeyboardEvent(&KeyboardController->Start, IsDown);
                    } else if (VKCode == 'W') {
                        Win32_ProcessKeyboardEvent(&KeyboardController->DpadUp, IsDown);
                    } else if (VKCode == 'A') {
                        Win32_ProcessKeyboardEvent(&KeyboardController->DpadLeft, IsDown);
                    } else if (VKCode == 'S') {
                        Win32_ProcessKeyboardEvent(&KeyboardController->DpadDown, IsDown);
                    } else if (VKCode == 'D') {
                        Win32_ProcessKeyboardEvent(&KeyboardController->DpadRight, IsDown);
                    } else if (VKCode == 'Q') {
                        Win32_ProcessKeyboardEvent(&KeyboardController->ShoulderLeft, IsDown);
                    } else if (VKCode == 'E') {
                        Win32_ProcessKeyboardEvent(&KeyboardController->ShoulderRight, IsDown);
                    } else if (VKCode == VK_UP) {
                        Win32_ProcessKeyboardEvent(&KeyboardController->Y, IsDown);
                    } else if (VKCode == VK_DOWN) {
                        Win32_ProcessKeyboardEvent(&KeyboardController->A, IsDown);
                    } else if (VKCode == VK_LEFT) {
                        Win32_ProcessKeyboardEvent(&KeyboardController->X, IsDown);
                    } else if (VKCode == VK_RIGHT) {
                        Win32_ProcessKeyboardEvent(&KeyboardController->B, IsDown);
#ifdef ASUKA_DEBUG
                    } else if (VKCode == 'L') {
                        if (IsDown == FALSE) {
                            if (Global_DebugInputRecording.RecordingState == INPUT_RECORDING_IDLE) {
                                Global_DebugInputRecording.RecordedInputsCount = 0;
                                Global_DebugInputRecording.RecordingState = INPUT_RECORDING_RECORD;
                            } else if (Global_DebugInputRecording.RecordingState == INPUT_RECORDING_RECORD) {
                                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
                                Global_DebugInputRecording.RecordingState = INPUT_RECORDING_PLAYBACK;
                            } else if (Global_DebugInputRecording.RecordingState == INPUT_RECORDING_PLAYBACK) {
                                Global_DebugInputRecording.RecordingState = INPUT_RECORDING_IDLE;
                            }
                        }
                    } else if (VKCode == 'K') {
                        if (IsDown == FALSE) {
                            if (Global_DebugInputRecording.RecordingState == INPUT_RECORDING_IDLE) {
                                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
                                Global_DebugInputRecording.RecordingState = INPUT_RECORDING_PLAYBACK;
                            } else if (Global_DebugInputRecording.RecordingState == INPUT_RECORDING_PLAYBACK) {
                                Global_DebugInputRecording.RecordingState = INPUT_RECORDING_IDLE;
                            }
                        }
#endif // ASUKA_DEBUG
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
}


#if ASUKA_DEBUG
static void Win32_DebugDrawVerticalMark(
    Win32_OffscreenBuffer* ScreenBuffer,
    int X,
    int Top,
    int Bottom,
    uint32 Color,
    bool Dotted = false)
{
    uint8* Pixel = (uint8*) ScreenBuffer->Memory
                 + X*ScreenBuffer->BytesPerPixel
                 + Top*ScreenBuffer->Pitch;

    if (Dotted) {
        int DashHeight_InPixels = 4;
        for (int Y = Top; Y < Bottom; Y += 2*DashHeight_InPixels) {
            for (int DashY = 0; DashY < DashHeight_InPixels / 2; DashY++) {
                *(uint32*)Pixel = Color;
                Pixel += ScreenBuffer->Pitch;
            }
            Pixel += ScreenBuffer->Pitch * (DashHeight_InPixels + DashHeight_InPixels / 2 + DashHeight_InPixels % 2);
        }
    } else {
        for (int Y = Top; Y < Bottom; Y++) {
            *(uint32*)Pixel = Color;
            Pixel += ScreenBuffer->Pitch;
        }
    }
}


static void Win32_DebugSoundDisplay(
    Win32_OffscreenBuffer* ScreenBuffer,
    Win32_SoundOutput* SoundOutput,
    Win32_DebugSoundCursors* Cursors,
    uint32  CursorCount,
    uint32  CurrentCursorIndex,
    float32 SecondsPerFrame)
{
    int PadX = 16;
    int PadY = 16;
    int LineHeight = 20;

    float32 C = (float32) (ScreenBuffer->Width - 2*PadX) / (float32) SoundOutput->SecondaryBufferSize;

    for (uint32 CursorIndex = 0; CursorIndex < CursorCount; CursorIndex++) {
        int Top    = PadY + LineHeight*CursorIndex;
        int Bottom = PadY + LineHeight*(CursorIndex + 1);

        {
            int X = PadX + (int)(C * (float32)Cursors[CursorIndex].PlayCursor);
            uint32 Color = CursorIndex == CurrentCursorIndex ? 0xFFFF00FF : 0xFFFFFFFF;
            Win32_DebugDrawVerticalMark(ScreenBuffer, X, Top, Bottom, Color, false);
        }
        {
            int X = PadX + (int)(C * (float32)Cursors[CursorIndex].WriteCursor);
            Win32_DebugDrawVerticalMark(ScreenBuffer, X, Top, Bottom, 0xFFFF0000, false);
        }
        {
            int X = PadX + (int)(C * (float32)Cursors[CursorIndex].OutputLocationStart);
            Win32_DebugDrawVerticalMark(ScreenBuffer, X, Top, Bottom, 0xFF00FF00, false);
        }
        {
            int X = PadX + (int)(C * (float32)Cursors[CursorIndex].OutputLocationEnd);
            Win32_DebugDrawVerticalMark(ScreenBuffer, X, Top, Bottom, 0xFF0000FF, false);
        }
        {
            int X = PadX + (int)(C * (float32)Cursors[CursorIndex].PageFlip);
            Win32_DebugDrawVerticalMark(ScreenBuffer, X, Top, Bottom, 0xFFFFFF00, true);
        }
        {
            int X = PadX + (int)(C * (float32)Cursors[CursorIndex].ExpectedNextPageFlip);
            Win32_DebugDrawVerticalMark(ScreenBuffer, X, Top, Bottom, 0xFF00FFFF, true);
        }
    }
}
#endif // ASUKA_DEBUG


static void Win32_DebugCatStrings(
    char* Source1, DWORD Source1Size,
    char* Source2, DWORD Source2Size,
    char* Dest, DWORD DestSize)
{
    for (DWORD CharIndex = 0; CharIndex < Source1Size; CharIndex++) {
        *Dest++ = Source1[CharIndex];
    }
    for (DWORD CharIndex = 0; CharIndex < Source2Size; CharIndex++) {
        *Dest++ = Source2[CharIndex];
    }
    *Dest = 0;
}


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    UINT DesiredSchedulerGranularityMS = 1; // ms
    MMRESULT TimeBeginPeriodResult = timeBeginPeriod(DesiredSchedulerGranularityMS); // Set this so that sleep granularity
    bool32 SleepIsGranular = TimeBeginPeriodResult == TIMERR_NOERROR;


    char ProgramPath [256] {};
    DWORD ProgramPathSize = GetModuleFileNameA(hInstance, ProgramPath, 256);

    DWORD IndexOfLastSlash = 0;
    for (DWORD CharIndex = 0; CharIndex < ProgramPathSize; CharIndex++) {
        if (ProgramPath[CharIndex] == '\\') {
            IndexOfLastSlash = CharIndex;
        }
    }
    DWORD ProgramPathNoFilenameSize = IndexOfLastSlash + 1;


    Win32_LoadXInputFunctions();
    WNDCLASSA WindowClass{};

    Win32_ResizeDIBSection(&Global_BackBuffer, 1200, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "AsukaWindowClass";
    // HICON     hIcon;

    int64 WallClockFrequency = os::get_wall_clock_frequency();

    ATOM ClassAtomResult = RegisterClassA(&WindowClass);
    if (!ClassAtomResult) {
        // Handle error
        return 1;
    }

    HWND Window = CreateWindowExA(
        0, // WS_EX_TOPMOST | WS_EX_LAYERED,
        WindowClass.lpszClassName,
        "AsukaWindow",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, // int X,
        CW_USEDEFAULT, // int Y,
        CW_USEDEFAULT, // int nWidth,
        CW_USEDEFAULT, // int nHeight,
        0, 0, hInstance, 0);

    if (!Window) {
        // Handle error
        return 1;
    }

    int MonitorRefreshHz;
    {
        HDC DeviceContext = GetDC(Window);
        MonitorRefreshHz = GetDeviceCaps(DeviceContext, VREFRESH);
        ReleaseDC(Window, DeviceContext);
    }

    int GameUpdateHz = MonitorRefreshHz / 2;
    float32 TargetSecondsPerFrame = 1.f / GameUpdateHz;

    // @TODO: remove this assert
    // Require this to be 60 Hz for now, this may be different for different monitors, but for testing purposes that's fine
    ASSERT(MonitorRefreshHz == 60);

    Win32_SoundOutput SoundOutput {};

    SoundOutput.SamplesPerSecond = 48000;
    SoundOutput.RunningSoundCursor = 0;
    SoundOutput.ChannelCount = 2;
    SoundOutput.BytesPerSoundFrame = sizeof(sound_sample_t) * SoundOutput.ChannelCount;
    SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSoundFrame;
    SoundOutput.SafetyBytes = (SoundOutput.BytesPerSoundFrame * SoundOutput.SamplesPerSecond / GameUpdateHz);

    Win32_InitDirectSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
    Win32_ClearSoundBuffer(&SoundOutput);
    Global_SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

    SoundOutput.Samples = (sound_sample_t*) VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#ifdef ASUKA_DEBUG
    LPVOID BaseAddress = (LPVOID)TERABYTES(1);
#else
    LPVOID BaseAddress = 0;
#endif

    Game_Memory GameMemory{};
    GameMemory.PermanentStorageSize = MEGABYTES(64);
    GameMemory.TransientStorageSize = GIGABYTES(1);

    uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    GameMemory.TransientStorage = (uint8*)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;

#if ASUKA_DEBUG
    Global_DebugInputRecording.InputRecordingSize = MEGABYTES(1);
    Global_DebugInputRecording.InputRecording = VirtualAlloc((uint8*)GameMemory.PermanentStorage + TotalSize, Global_DebugInputRecording.InputRecordingSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Global_DebugInputRecording.RecordedInputsCount = 0;
    Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
    Global_DebugInputRecording.RecordingState = INPUT_RECORDING_IDLE;

    Win32_DebugSoundCursors Debug_Cursors[30] {};
    uint32 Debug_SoundCursorIndex = 0;
#endif // ASUKA_DEBUG

    Running = true;
    int FrameCounter = 0;

    ThreadContext GameThread {};

    Game_Input Input[2] {};
    Game_Input* OldInput = &Input[0];
    Game_Input* NewInput = &Input[1];

    char GameDllFilename[] = "asuka.dll";
    char GameTempDllFilename[] = "asuka_running.dll";

    char GameDllFilepath [256];
    char GameTempDllFilepath [256];

    Win32_DebugCatStrings(
        ProgramPath, ProgramPathNoFilenameSize,
        GameDllFilename, sizeof(GameDllFilename),
        GameDllFilepath, sizeof(GameDllFilepath));
    Win32_DebugCatStrings(
        ProgramPath, ProgramPathNoFilenameSize,
        GameTempDllFilename, sizeof(GameTempDllFilename),
        GameTempDllFilepath, sizeof(GameTempDllFilepath));

    Win32_GameDLL Game = Win32_LoadGameDLL(GameDllFilepath, GameTempDllFilepath);

    int64 LastClockTimepoint = os::get_wall_clock();
    while (Running) {
        FrameCounter += 1;

        FILETIME GameDllTimestamp = Win32_GetFileTimestamp(GameDllFilepath);
        if (CompareFileTime(&GameDllTimestamp, &Game.Timestamp) != 0) {
            Win32_UnloadGameDLL(&Game);
            Game = Win32_LoadGameDLL(GameDllFilepath, GameTempDllFilepath);
        }

        Game_ControllerInput* OldKeyboardController = GetController(OldInput, 0);
        Game_ControllerInput* NewKeyboardController = GetController(NewInput, 0);

        Game_ControllerInput ZeroController{};
        *NewKeyboardController = ZeroController;

        // Save EndedDown state between frames so that we could hold buttons
        for (uint32 ButtonIndex = 0; ButtonIndex < ARRAY_COUNT(OldKeyboardController->Buttons); ButtonIndex++) {
            NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
        }

        Win32_ProcessPendingMessages(NewKeyboardController, &NewInput->Mouse);
        OldInput->Mouse = NewInput->Mouse;

        DWORD MaxControllerCount = XUSER_MAX_COUNT;
        if (MaxControllerCount > ARRAY_COUNT(Input[0].Controllers) - 1) {
            MaxControllerCount = ARRAY_COUNT(Input[0].Controllers) - 1;
        }

        for (DWORD GamepadXInputIndex = 0; GamepadXInputIndex < MaxControllerCount; GamepadXInputIndex++ ) {
            DWORD GamepadIndex = GamepadXInputIndex + 1; // 0-th Controller is Keyboard controller

            XINPUT_STATE InputState;
            auto CONTROLLER_STATE_EC = XInputGetState(0, &InputState);
            if (CONTROLLER_STATE_EC == ERROR_SUCCESS) {
                XINPUT_GAMEPAD Gamepad = InputState.Gamepad;

                Game_ControllerInput* OldGamepadInput = GetController(OldInput, GamepadIndex);
                Game_ControllerInput* NewGamepadInput = GetController(NewInput, GamepadIndex);

                NewGamepadInput->IsAnalog = true;
                OldGamepadInput->IsAnalog = true;

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

        DWORD PlayCursor = 0;
        DWORD WriteCursor = 0;

        DWORD ByteToLock = 0;
        DWORD BytesToWrite = 0;

        // DWORD AudioLatencyBytes = 0;
        // float32 AudioLatencySeconds = 0;

        if (SUCCEEDED(Global_SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
        {
            if (SoundOutput.RunningSoundCursor <= WriteCursor) {
                // This might be the result of the underrun. Correct the writing position.
                SoundOutput.RunningSoundCursor = WriteCursor;
            }

            // DWORD UnwrappedWriteCursor = WriteCursor;
            // if (UnwrappedWriteCursor < PlayCursor) {
            //     UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
            // }

            // AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
            // AudioLatencySeconds = ((float32)AudioLatencyBytes / (float32)SoundOutput.BytesPerSoundFrame) / (float32) SoundOutput.SamplesPerSecond;

            DWORD SoundBytesPerFrame = (DWORD)(SoundOutput.BytesPerSoundFrame * SoundOutput.SamplesPerSecond / GameUpdateHz);
            DWORD TargetCursor = (WriteCursor + SoundBytesPerFrame + SoundOutput.SafetyBytes) % SoundOutput.SecondaryBufferSize;
            ByteToLock = SoundOutput.RunningSoundCursor % SoundOutput.SecondaryBufferSize;

            if (ByteToLock > TargetCursor) {
                BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
                BytesToWrite += TargetCursor;
            } else {
                BytesToWrite = TargetCursor - ByteToLock;
            }
        }

        // char print_buffer[1024];
        // sprintf(print_buffer, "Between cursors: %d; Audio latency %f; SafetyBytes: %d\n",
        //     AudioLatencyBytes, AudioLatencySeconds, SoundOutput.SafetyBytes);
        // OutputDebugString(print_buffer);

        ASSERT(BytesToWrite <= SoundOutput.SecondaryBufferSize);

        Game_SoundOutputBuffer SoundBuffer {};
        SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
        SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSoundFrame;
        SoundBuffer.Samples = SoundOutput.Samples;

        Game_OffscreenBuffer ScreenBuffer;
        ScreenBuffer.Memory = Global_BackBuffer.Memory;
        ScreenBuffer.Width  = Global_BackBuffer.Width;
        ScreenBuffer.Height = Global_BackBuffer.Height;
        ScreenBuffer.Pitch  = Global_BackBuffer.Pitch;
        ScreenBuffer.BytesPerPixel = Global_BackBuffer.BytesPerPixel;

#ifdef ASUKA_DEBUG
        Game_State* GameState = (Game_State*) GameMemory.PermanentStorage;
        if (Global_DebugInputRecording.RecordingState == INPUT_RECORDING_IDLE) {
            GameState->BorderVisible = false;

        } else if (Global_DebugInputRecording.RecordingState == INPUT_RECORDING_RECORD) {
            if (Global_DebugInputRecording.RecordedInputsCount == 0) {
                Global_DebugInputRecording.InitialGameState = *GameState;
            }

            // Checking if there's room for one more Game_Input
            if (((Global_DebugInputRecording.RecordedInputsCount + 1) * sizeof(Game_Input)) < Global_DebugInputRecording.InputRecordingSize) {
                Game_Input* RecordedInputs = (Game_Input*)Global_DebugInputRecording.InputRecording;
                RecordedInputs[Global_DebugInputRecording.RecordedInputsCount++] = *NewInput;
            } else {
                ASSERT_FAIL("The space for storing input is ran out. Increase it in code above (it's debug code, nobody cares)\n");
            }

            GameState->BorderVisible = true;
            GameState->BorderColor = 0xFF'FF'FF'00;

        } else if (Global_DebugInputRecording.RecordingState == INPUT_RECORDING_PLAYBACK) {
            if (Global_DebugInputRecording.CurrentPlaybackInputIndex == 0) {
                *GameState = Global_DebugInputRecording.InitialGameState;
            }

            Game_Input* RecordedInputs = (Game_Input*)Global_DebugInputRecording.InputRecording;
            *NewInput = RecordedInputs[Global_DebugInputRecording.CurrentPlaybackInputIndex++];

            if (Global_DebugInputRecording.CurrentPlaybackInputIndex == Global_DebugInputRecording.RecordedInputsCount) {
                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
            }

            GameState->BorderVisible = true;
            GameState->BorderColor = 0xFF'00'FF'00;
        }
#endif // ASUKA_DEBUG

        if (Game.UpdateAndRender) {
            Game.UpdateAndRender(&GameThread, &GameMemory, NewInput, &ScreenBuffer, &SoundBuffer);
        }

        Win32_FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);

        int64 WorkCounter = os::get_wall_clock();
        float32 SecondsElapsedForWork = (float32)(WorkCounter - LastClockTimepoint) / WallClockFrequency;

        float32 SecondsElapsedForFrame = SecondsElapsedForWork;
        if (SecondsElapsedForFrame < TargetSecondsPerFrame) {
            if (SleepIsGranular) {
                DWORD SleepMS = truncate_cast_to_uint32(1000.f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                if (SleepMS > 0) {
                    // @todo
                    // Sleep(SleepMS);
                }
            }

            while (SecondsElapsedForFrame < TargetSecondsPerFrame) {
                SecondsElapsedForFrame = (float32)(os::get_wall_clock() - LastClockTimepoint) / (float32)WallClockFrequency;
            }

            if (SecondsElapsedForFrame < TargetSecondsPerFrame) {
                // @TODO: Slept for good time!
            } else {
                // OutputDebugStringA("MISSED FRAME!\n");
                // @TODO: handle missed frame rate!
            }
        } else {
            // OutputDebugStringA("MISSED FRAME!\n");
            // @TODO: handle missed frame rate!
        }

        LastClockTimepoint = os::get_wall_clock();

#if ASUKA_DEBUG
        {
            DWORD Debug_PageFlip_PlayCursor = 0;
            DWORD Debug_PageFlip_WriteCursor = 0;

            Global_SecondaryBuffer->GetCurrentPosition(&Debug_PageFlip_PlayCursor, &Debug_PageFlip_WriteCursor);

            Debug_Cursors[Debug_SoundCursorIndex].PlayCursor = PlayCursor;
            Debug_Cursors[Debug_SoundCursorIndex].WriteCursor = WriteCursor;

            Debug_Cursors[Debug_SoundCursorIndex].OutputLocationStart = ByteToLock;
            Debug_Cursors[Debug_SoundCursorIndex].OutputLocationEnd = (ByteToLock + BytesToWrite) % SoundOutput.SecondaryBufferSize;

            Debug_Cursors[Debug_SoundCursorIndex].PageFlip = Debug_PageFlip_PlayCursor;
            Debug_Cursors[Debug_SoundCursorIndex].ExpectedNextPageFlip = Debug_PageFlip_PlayCursor +
                (SoundOutput.BytesPerSoundFrame * SoundOutput.SamplesPerSecond / GameUpdateHz);

            Win32_DebugSoundDisplay(
                &Global_BackBuffer,
                &SoundOutput,
                Debug_Cursors,
                ARRAY_COUNT(Debug_Cursors),
                Debug_SoundCursorIndex,
                TargetSecondsPerFrame);

            Debug_SoundCursorIndex = (Debug_SoundCursorIndex + 1) % ARRAY_COUNT(Debug_Cursors);
        }
#endif

        {
            HDC DeviceContext = GetDC(Window);

            Win32_Window_Dimensions WindowSize = Win32_GetWindowDimention(Window);
            Win32_CopyBufferToWindow(&Global_BackBuffer, DeviceContext, WindowSize.Width, WindowSize.Height);

            ReleaseDC(Window, DeviceContext);
        }

        {
            float32 MilliSecondsElapsed = 1000.f * SecondsElapsedForFrame;
            float32 FPS = (float32)1.f / SecondsElapsedForFrame;

            char Buffer[256];
            sprintf(Buffer, "%f ms/f; FPS: %f\n", MilliSecondsElapsed, FPS);
            OutputDebugStringA(Buffer);
        }

        Game_Input* TmpInput = NewInput;
        NewInput = OldInput;
        OldInput = TmpInput;
    }

    return 0;
}
