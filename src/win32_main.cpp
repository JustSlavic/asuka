// BIG TODO FOR PLATFORM LAYER:
//
// - Saved game locations
// - Getting a handle to our own executable file
// - Asset loading path
// - Threading (launch a thread)
// - Raw input (support for multiple keyboards)
// - ClipCursor() (multimonitor support)
// - Fullscreen support
// - WM_SETCURSOR (control cursor visibility)
// - QueryCancelAutoplay
// - WM_ACTIVATEAPP (for when we are not the active application)
// - Blit speed improvements (BitBlt)
// - Hardware acceleration (OpenGL or Direct3D)
// - GetKeyboardLayout (for French keyboards, international WASD support)
//
// - ChangeDisplaySettings ?
//
// Just a partial list of stuff
//

#include <defines.hpp>
#include <asuka.hpp>
#include <os/time.hpp>
#include <debug/casts.hpp>

// Windows
#include <windows.h>

// XInput
#include <xinput.h>

// Direct Sound
#include <dsound.h>
#include <stdio.h>

// #include <ft2build.h>
// #include FT_FREETYPE_H


#define DRAW_DEBUG_SOUND_CURSORS (0 && ASUKA_DEBUG)
#define DEBUG_WINDOW_ON_TOP (0 && ASUKA_DEBUG)


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


struct Win32_SoundOutput {
    uint32 SamplesPerSecond;
    uint32 RunningSoundCursor;
    uint32 ChannelCount;
    uint32 BytesPerSoundFrame;
    uint32 SecondaryBufferSize;
    uint32 SafetyBytes;
    sound_sample_t* Samples;
};


#ifdef DRAW_DEBUG_SOUND_CURSORS
struct Win32_DebugSoundCursors {
    uint32 PlayCursor;
    uint32 WriteCursor;

    uint32 OutputLocationStart;
    uint32 OutputLocationEnd;

    uint32 PageFlip;
    uint32 ExpectedNextPageFlip;
};
#endif // DRAW_DEBUG_SOUND_CURSORS

#if ASUKA_PLAYBACK_LOOP
struct Win32_DebugInputRecording {
    uint64 InitialGameMemorySize;
    void*  InitialGameMemory;

    // Storage of recorded inputs
    uint64 InputRecordingSize;
    void * InputRecording;

    // Where playback is currently replaying input
    uint64 CurrentPlaybackInputIndex;
    // How many input frames recorded to playback
    uint64 RecordedInputsCount;

    Debug_PlaybackLoopState PlaybackLoopState;
};

GLOBAL_VARIABLE Win32_DebugInputRecording Global_DebugInputRecording;
#endif // ASUKA_PLAYBACK_LOOP


GLOBAL_VARIABLE bool Running;
GLOBAL_VARIABLE Win32_OffscreenBuffer Global_BackBuffer;
GLOBAL_VARIABLE LPDIRECTSOUNDBUFFER Global_SecondaryBuffer;
GLOBAL_VARIABLE bool Global_CursorIsVisible;
GLOBAL_VARIABLE WINDOWPLACEMENT Global_WindowPosition = { sizeof(Global_WindowPosition) };
GLOBAL_VARIABLE bool Global_IsFullscreen;


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(Win32_XInputGetStateT);
X_INPUT_GET_STATE(Win32_XInputGetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
GLOBAL_VARIABLE Win32_XInputGetStateT* XInputGetState_ = Win32_XInputGetStateStub;
#define XInputGetState XInputGetState_


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(Win32_XInputSetStateT);
X_INPUT_SET_STATE(Win32_XInputSetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
GLOBAL_VARIABLE Win32_XInputSetStateT* XInputSetState_ = Win32_XInputSetStateStub;
#define XInputSetState XInputSetState_


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(Win32_DirectSoundCreateT);


INTERNAL_FUNCTION
void Win32_LoadXInputFunctions() {
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


// #define FT_INIT_LIBRARY_SIGNATURE(name) FT_EXPORT(FT_Error) name(FT_Library *library)
// typedef FT_INIT_LIBRARY_SIGNATURE(Win32_FT_Init_FreeTypeT);
// FT_INIT_LIBRARY_SIGNATURE(Win32_FT_Init_FreeTypeStub) {
//     return 1;
// }
// GLOBAL_VARIABLE Win32_FT_Init_FreeTypeT *FT_Init_FreeType_ = Win32_FT_Init_FreeTypeStub;
// #define FT_Init_FreeType FT_Init_FreeType_

// #define FT_INIT_LIBRARY_SIGNATURE(name) FT_EXPORT(FT_Error) name(FT_Library *library)
// typedef FT_INIT_LIBRARY_SIGNATURE(Win32_FT_Init_FreeTypeT);
// FT_INIT_LIBRARY_SIGNATURE(Win32_FT_Init_FreeTypeStub) {
//     return 1;
// }
// GLOBAL_VARIABLE Win32_FT_Init_FreeTypeT *FT_Init_FreeType_ = Win32_FT_Init_FreeTypeStub;
// #define FT_Init_FreeType FT_Init_FreeType_

// FT_EXPORT( FT_Error )
//   FT_New_Face( FT_Library   library,
//                const char*  filepathname,
//                FT_Long      face_index,
//                FT_Face     *aface );

// FT_New_Face
// FT_Set_Char_Size
// FT_Load_Char
// FT_Set_Transform


// INTERNAL_FUNCTION
// void Win32_LoadFreeTypeFunctions() {

// }


INTERNAL_FUNCTION
FILETIME Win32_GetFileTimestamp(const char* Filename) {
    FILETIME Result {};

    WIN32_FILE_ATTRIBUTE_DATA FileData;
    GetFileAttributesExA(Filename, GetFileExInfoStandard, &FileData);
    Result = FileData.ftLastWriteTime;

    return Result;
}


INTERNAL_FUNCTION
Win32_GameDLL Win32_LoadGameDLL(const char* DllPath, const char* TempDllPath, const char *LockFilename) {
#if defined(ASUKA_DLL_BUILD)
    Win32_GameDLL Result {};

    DWORD dwAttrib = GetFileAttributes(LockFilename);
    BOOL LockFileExists = (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

    if (!LockFileExists) {
        Result.Timestamp = Win32_GetFileTimestamp(DllPath);
        CopyFile(DllPath, TempDllPath, FALSE);
        Result.GameDLL = LoadLibraryA(TempDllPath);

        if (Result.GameDLL) {
            Result.UpdateAndRender = (Game_UpdateAndRenderT*)GetProcAddress(Result.GameDLL, "Game_UpdateAndRender");

            Result.IsValid = (Result.UpdateAndRender != NULL);
        }
    }

    return Result;
#else
    Win32_GameDLL Result {};
    Result.IsValid = true;
    Result.UpdateAndRender = Game_UpdateAndRender;

    return Result;
#endif
}


INTERNAL_FUNCTION
void Win32_UnloadGameDLL(Win32_GameDLL* GameCode) {
#if defined(ASUKA_DLL_BUILD)
    if (GameCode->GameDLL) {
        FreeLibrary(GameCode->GameDLL);
        GameCode->GameDLL = NULL;
    }

    GameCode->IsValid = false;
    GameCode->UpdateAndRender = NULL;
#endif
}


INTERNAL_FUNCTION
void Win32_ProcessKeyboardEvent(Game_ButtonState* NewState, bool32 IsDown) {
    if (NewState->EndedDown != IsDown) {
        NewState->EndedDown = IsDown;
        NewState->HalfTransitionCount++;
    }
}


INTERNAL_FUNCTION
void Win32_ProcessXInputButton(
    Game_ButtonState* OldState,
    Game_ButtonState* NewState,
    DWORD XInputButtonState,
    DWORD ButtonBit)
{
    NewState->EndedDown = (XInputButtonState & ButtonBit) == ButtonBit;
    NewState->HalfTransitionCount = ((NewState->EndedDown != OldState->EndedDown) ? 1 : 0);
}


INTERNAL_FUNCTION
float32 Win32_ProcessXInputStick(int16 value, int16 deadzone) {
    if (value < -deadzone) {
        return (float32)(value + deadzone) / (float32)(32768 - deadzone);
    } else if (value > deadzone) {
        return (float32)(value - deadzone) / (float32)(32767 - deadzone);
    }

    return 0.f;
}


INTERNAL_FUNCTION
float32 Win32_ProcessXInputTrigger(uint8 value, uint8 deadzone) {
    if (value < deadzone) {
        return (float32)(value + deadzone) / (float32)(255 - deadzone);
    }

    return 0.f;
}


INTERNAL_FUNCTION
void Win32_InitDirectSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize) {
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


INTERNAL_FUNCTION
Win32_Window_Dimensions Win32_GetWindowDimention(HWND Window) {
    Win32_Window_Dimensions Result;

    RECT ClientRectangle;
    GetClientRect(Window, &ClientRectangle);

    Result.Width = ClientRectangle.right - ClientRectangle.left;
    Result.Height = ClientRectangle.bottom - ClientRectangle.top;

    return Result;
}


INTERNAL_FUNCTION
void Win32_ResizeDIBSection(Win32_OffscreenBuffer* Buffer, LONG Width, LONG Height) {
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


INTERNAL_FUNCTION
void Win32_CopyBufferToWindow(Win32_OffscreenBuffer *Buffer, HDC device_context, int WindowWidth, int WindowHeight) {
    if (Global_IsFullscreen) {
        StretchDIBits(
            device_context,
            0, 0, WindowWidth, WindowHeight,
            0, 0, Buffer->Width, Buffer->Height,
            Buffer->Memory, &Buffer->Info,
            DIB_RGB_COLORS, SRCCOPY);
    } else {
        PatBlt(
            device_context,
            Buffer->Width, 0,
            WindowWidth - Buffer->Width, WindowHeight,
            BLACKNESS);
        PatBlt(
            device_context,
            0, Buffer->Height,
            Buffer->Width, WindowHeight,
            BLACKNESS);

        StretchDIBits(
            device_context,
            0, 0, Buffer->Width, Buffer->Height,
            0, 0, Buffer->Width, Buffer->Height,
            Buffer->Memory, &Buffer->Info,
            DIB_RGB_COLORS, SRCCOPY);
    }
}


INTERNAL_FUNCTION
void Win32_ClearSoundBuffer(Win32_SoundOutput* SoundOutput) {
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


INTERNAL_FUNCTION
void Win32_FillSoundBuffer(
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


INTERNAL_FUNCTION
void Win32_ToggleFullscreen(HWND Window) {
    /*
        Reference: https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    */
    DWORD WindowStyle = GetWindowLong(Window, GWL_STYLE);
    if (WindowStyle & WS_OVERLAPPEDWINDOW) {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        if (GetWindowPlacement(Window, &Global_WindowPosition) && GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo)) {
                SetWindowLong(Window, GWL_STYLE, WindowStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(Window, HWND_TOP,
                    MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                    MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                    MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }

        Global_IsFullscreen = true;
    } else {
        SetWindowLong(Window, GWL_STYLE, WindowStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &Global_WindowPosition);
        SetWindowPos(Window, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        Global_IsFullscreen = false;
    }
}


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result {};

    switch (message) {
        case WM_SETCURSOR: {
            if (Global_CursorIsVisible) {
                result = DefWindowProcA(Window, message, wParam, lParam);
            } else {
                SetCursor(NULL);
            }
            break;
        }
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
#if DEBUG_WINDOW_ON_TOP
            if (wParam == TRUE) {
                LONG_PTR SetExtendedStyleResult = SetWindowLongPtrA(Window, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED);
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            } else {
                LONG_PTR SetExtendedStyleResult = SetWindowLongPtrA(Window, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT);
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
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
                bool32 AltDown = (Message.lParam & (1 << 29));
                bool32 WasDown = (Message.lParam & (1 << 30)) != 0;
                bool32 IsDown = (Message.lParam & (1 << 31)) == 0;

                if (WasDown != IsDown) {
                    if (VKCode == VK_ESCAPE) {
                        Running = false;
                        Win32_ProcessKeyboardEvent(&KeyboardController->Back, IsDown);
                    } else if (VKCode == VK_SPACE) {
                        Win32_ProcessKeyboardEvent(&KeyboardController->Start, IsDown);
                    } else if (VKCode == 'W') {
                        if (IsDown) {
                            KeyboardController->LeftStickEnded.y = math::clamp(KeyboardController->LeftStickEnded.y + 1, -1, 1);
                        } else {
                            KeyboardController->LeftStickEnded.y = math::clamp(KeyboardController->LeftStickEnded.y - 1, -1, 1);
                        }
                    } else if (VKCode == 'A') {
                        if (IsDown) {
                            KeyboardController->LeftStickEnded.x = math::clamp(KeyboardController->LeftStickEnded.x - 1, -1, 1);
                        } else {
                            KeyboardController->LeftStickEnded.x = math::clamp(KeyboardController->LeftStickEnded.x + 1, -1, 1);
                        }
                    } else if (VKCode == 'S') {
                        if (IsDown) {
                            KeyboardController->LeftStickEnded.y = math::clamp(KeyboardController->LeftStickEnded.y - 1, -1, 1);
                        } else {
                            KeyboardController->LeftStickEnded.y = math::clamp(KeyboardController->LeftStickEnded.y + 1, -1, 1);
                        }
                    } else if (VKCode == 'D') {
                        if (IsDown) {
                            KeyboardController->LeftStickEnded.x = math::clamp(KeyboardController->LeftStickEnded.x + 1, -1, 1);
                        } else {
                            KeyboardController->LeftStickEnded.x = math::clamp(KeyboardController->LeftStickEnded.x - 1, -1, 1);
                        }
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
#if ASUKA_PLAYBACK_LOOP
                    } else if (VKCode == 'L') {
                        if (IsDown == FALSE) {
                            if (Global_DebugInputRecording.PlaybackLoopState == PLAYBACK_LOOP_IDLE) {
                                Global_DebugInputRecording.RecordedInputsCount = 0;
                                Global_DebugInputRecording.PlaybackLoopState = PLAYBACK_LOOP_RECORDING;
                            } else if (Global_DebugInputRecording.PlaybackLoopState == PLAYBACK_LOOP_RECORDING) {
                                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
                                Global_DebugInputRecording.PlaybackLoopState = PLAYBACK_LOOP_PLAYBACK;
                            } else if (Global_DebugInputRecording.PlaybackLoopState == PLAYBACK_LOOP_PLAYBACK) {
                                // Nullify keyboard controller such as nothing is pressed on stoping the playback loop
                                // because if there's something left pressed, it will stay pressed although nothing is
                                // pressed on the actual keyboard
                                *KeyboardController = {};
                                Global_DebugInputRecording.PlaybackLoopState = PLAYBACK_LOOP_IDLE;
                            }
                        }
                    } else if (VKCode == 'K') {
                        if (IsDown == FALSE) {
                            if (Global_DebugInputRecording.PlaybackLoopState == PLAYBACK_LOOP_IDLE) {
                                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
                                Global_DebugInputRecording.PlaybackLoopState = PLAYBACK_LOOP_PLAYBACK;
                            } else if (Global_DebugInputRecording.PlaybackLoopState == PLAYBACK_LOOP_PLAYBACK) {
                                // Nullify keyboard controller such as nothing is pressed on stoping the playback loop
                                // because if there's something left pressed, it will stay pressed although nothing is
                                // pressed on the actual keyboard
                                *KeyboardController = {};
                                Global_DebugInputRecording.PlaybackLoopState = PLAYBACK_LOOP_IDLE;
                            }
                        }
#endif // ASUKA_PLAYBACK_LOOP
                    }

                    if (IsDown) {
                        if (AltDown && VKCode == VK_RETURN) {
                            if (Message.hwnd) {
                                Win32_ToggleFullscreen(Message.hwnd);
                            }
                        }
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


#if DRAW_DEBUG_SOUND_CURSORS
INTERNAL_FUNCTION
void Win32_DebugDrawVerticalMark(
    Win32_OffscreenBuffer* ScreenBuffer,
    int X,
    int Top,
    int Bottom,
    uint32 Color,
    bool Dotted = false)
{
    if (Bottom >= ScreenBuffer->Height) {
        Bottom = ScreenBuffer->Height;
    }

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

INTERNAL_FUNCTION
void Win32_DebugSoundDisplay(
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
#endif // DRAW_DEBUG_SOUND_CURSORS


INTERNAL_FUNCTION
void Win32_DebugCatStrings(
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
    Win32_ResizeDIBSection(&Global_BackBuffer, 960, 540);

#ifdef ASUKA_DEBUG
    Global_CursorIsVisible = true;
#endif

    WNDCLASSA WindowClass{};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "AsukaWindowClass";
    // HICON     hIcon;
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    int64 WallClockFrequency = os::get_wall_clock_frequency();

    ATOM ClassAtomResult = RegisterClassA(&WindowClass);
    if (!ClassAtomResult) {
        // Handle error
        return 1;
    }

    HWND Window = CreateWindowExA(
#if DEBUG_WINDOW_ON_TOP
        WS_EX_TOPMOST | WS_EX_LAYERED,
#else
        0,
#endif
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
    SoundOutput.SafetyBytes = (uint32) (1.5f * SoundOutput.BytesPerSoundFrame * SoundOutput.SamplesPerSecond / GameUpdateHz);

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

#if ASUKA_PLAYBACK_LOOP
    uint64 InitialGameMemorySize = GameMemory.PermanentStorageSize;

    Global_DebugInputRecording.InitialGameMemorySize = InitialGameMemorySize;
    Global_DebugInputRecording.InputRecordingSize = MEGABYTES(1);

    Global_DebugInputRecording.InitialGameMemory = VirtualAlloc(
        (uint8*)GameMemory.PermanentStorage + TotalSize,
        InitialGameMemorySize + Global_DebugInputRecording.InputRecordingSize, // GameMemory size + size of the recorded inputs
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Global_DebugInputRecording.InputRecording = (uint8 *)Global_DebugInputRecording.InitialGameMemory + InitialGameMemorySize;

    Global_DebugInputRecording.RecordedInputsCount = 0;
    Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
    Global_DebugInputRecording.PlaybackLoopState = PLAYBACK_LOOP_IDLE;
#endif // ASUKA_PLAYBACK_LOOP

#if DRAW_DEBUG_SOUND_CURSORS
    Win32_DebugSoundCursors Debug_Cursors[30] {};
    uint32 Debug_SoundCursorIndex = 0;
#endif // DRAW_DEBUG_SOUND_CURSORS

    Running = true;
    int FrameCounter = 0;

    ThreadContext GameThread {};

    Game_Input Input[2] {};
    Game_Input* OldInput = &Input[0];
    Game_Input* NewInput = &Input[1];

    OldInput->dt = TargetSecondsPerFrame;
    NewInput->dt = TargetSecondsPerFrame;

    char GameDllFilename[] = "asuka.dll";
    char GameTempDllFilename[] = "asuka_running.dll";
    char LockFilename[] = "lock.tmp";

    char GameDllFilepath [256];
    char GameTempDllFilepath [256];
    char LockFilepath [256];

    Win32_DebugCatStrings(
        ProgramPath, ProgramPathNoFilenameSize,
        GameDllFilename, sizeof(GameDllFilename),
        GameDllFilepath, sizeof(GameDllFilepath));
    Win32_DebugCatStrings(
        ProgramPath, ProgramPathNoFilenameSize,
        GameTempDllFilename, sizeof(GameTempDllFilename),
        GameTempDllFilepath, sizeof(GameTempDllFilepath));
    Win32_DebugCatStrings(
        ProgramPath, ProgramPathNoFilenameSize,
        LockFilename, sizeof(LockFilename),
        LockFilepath, sizeof(LockFilepath));

    Win32_GameDLL Game = Win32_LoadGameDLL(GameDllFilepath, GameTempDllFilepath, LockFilepath);

    int64 LastClockTimepoint = os::get_wall_clock();
    while (Running) {
        FrameCounter += 1;

        FILETIME GameDllTimestamp = Win32_GetFileTimestamp(GameDllFilepath);
        if (CompareFileTime(&GameDllTimestamp, &Game.Timestamp) != 0) {
            Win32_UnloadGameDLL(&Game);
            Game = Win32_LoadGameDLL(GameDllFilepath, GameTempDllFilepath, LockFilepath);
        }

        Game_ControllerInput* OldKeyboardController = &OldInput->KeyboardInput;
        Game_ControllerInput* NewKeyboardController = &NewInput->KeyboardInput;

        *NewKeyboardController = *OldKeyboardController;

        for (uint32 ButtonIndex = 0; ButtonIndex < ARRAY_COUNT(NewKeyboardController->Buttons); ButtonIndex++) {
            NewKeyboardController->Buttons[ButtonIndex].HalfTransitionCount = 0;
        }

        // Save EndedDown state between frames so that we could hold buttons
        // for (uint32 ButtonIndex = 0; ButtonIndex < ARRAY_COUNT(OldKeyboardController->Buttons); ButtonIndex++) {
        //     NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
        // }

        Win32_ProcessPendingMessages(NewKeyboardController, &NewInput->Mouse);
        OldInput->Mouse = NewInput->Mouse;

        DWORD MaxControllerCount = XUSER_MAX_COUNT;
        if (MaxControllerCount > ARRAY_COUNT(Input[0].ControllerInputs)) {
            MaxControllerCount = ARRAY_COUNT(Input[0].ControllerInputs);
        }

        for (DWORD GamepadXInputIndex = 0; GamepadXInputIndex < MaxControllerCount; GamepadXInputIndex++ ) {
            DWORD GamepadIndex = GamepadXInputIndex;

            XINPUT_STATE InputState;
            auto CONTROLLER_STATE_EC = XInputGetState(GamepadXInputIndex, &InputState);
            if (CONTROLLER_STATE_EC == ERROR_SUCCESS) {
                XINPUT_GAMEPAD Gamepad = InputState.Gamepad;

                Game_ControllerInput* OldGamepadInput = GetGamepadInput(OldInput, GamepadIndex);
                Game_ControllerInput* NewGamepadInput = GetGamepadInput(NewInput, GamepadIndex);
                *NewGamepadInput = {};

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

                NewGamepadInput->LeftStickStarted = OldGamepadInput->LeftStickEnded;

                NewGamepadInput->LeftStickEnded.x = Win32_ProcessXInputStick(Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                NewGamepadInput->LeftStickEnded.y = Win32_ProcessXInputStick(Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                NewGamepadInput->RightStickStarted = OldGamepadInput->RightStickEnded;

                NewGamepadInput->RightStickEnded.x = Win32_ProcessXInputStick(Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
                NewGamepadInput->RightStickEnded.y = Win32_ProcessXInputStick(Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

                NewGamepadInput->TriggerLeftStarted = OldGamepadInput->TriggerLeftEnded;
                NewGamepadInput->TriggerLeftEnded = Win32_ProcessXInputTrigger(Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

                NewGamepadInput->TriggerRightStarted = OldGamepadInput->TriggerRightEnded;
                NewGamepadInput->TriggerRightEnded = Win32_ProcessXInputTrigger(Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

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

#if ASUKA_PLAYBACK_LOOP
        NewInput->PlaybackLoopState = Global_DebugInputRecording.PlaybackLoopState;

        if (Global_DebugInputRecording.PlaybackLoopState == PLAYBACK_LOOP_RECORDING) {
            // Just started recording, save initial game memory
            if (Global_DebugInputRecording.RecordedInputsCount == 0) {
                CopyMemory(
                    Global_DebugInputRecording.InitialGameMemory,
                    GameMemory.PermanentStorage,
                    Global_DebugInputRecording.InitialGameMemorySize);
            }

            // Checking if there's room for one more Game_Input
            if (((Global_DebugInputRecording.RecordedInputsCount + 1) * sizeof(Game_Input)) < Global_DebugInputRecording.InputRecordingSize) {
                Game_Input* RecordedInputs = (Game_Input*)Global_DebugInputRecording.InputRecording;
                RecordedInputs[Global_DebugInputRecording.RecordedInputsCount] = *NewInput;
                RecordedInputs[Global_DebugInputRecording.RecordedInputsCount].PlaybackLoopState = PLAYBACK_LOOP_PLAYBACK;
                Global_DebugInputRecording.RecordedInputsCount++;
            } else {
                ASSERT_FAIL("The space for storing input is ran out. Increase it in code above (it's debug code, nobody cares)\n");
            }
        }

        if (Global_DebugInputRecording.PlaybackLoopState == PLAYBACK_LOOP_PLAYBACK) {
            // If the playback started over, copy initial game memory back to the game memory storage
            if (Global_DebugInputRecording.CurrentPlaybackInputIndex == 0) {
                CopyMemory(
                    GameMemory.PermanentStorage,
                    Global_DebugInputRecording.InitialGameMemory,
                    Global_DebugInputRecording.InitialGameMemorySize);
            }

            Game_Input* RecordedInputs = (Game_Input*)Global_DebugInputRecording.InputRecording;
            *NewInput = RecordedInputs[Global_DebugInputRecording.CurrentPlaybackInputIndex++];

            if (Global_DebugInputRecording.CurrentPlaybackInputIndex == Global_DebugInputRecording.RecordedInputsCount) {
                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
            }
        }
#endif // ASUKA_PLAYBACK_LOOP

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
                    Sleep(SleepMS);
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

#if DRAW_DEBUG_SOUND_CURSORS
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
