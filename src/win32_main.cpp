// Project headers
#include <asuka.hpp>
#include <os/time.hpp>
#include <debug/casts.hpp>

// Platform Layer
#include <platform_win32.hpp>


GLOBAL bool Running;
GLOBAL Platform::OffscreenBuffer Global_BackBuffer;
GLOBAL LPDIRECTSOUNDBUFFER Global_SecondaryBuffer;
GLOBAL bool Global_CursorIsVisible;
GLOBAL WINDOWPLACEMENT Global_WindowPosition = { sizeof(Global_WindowPosition) };
GLOBAL bool Global_IsFullscreen;
GLOBAL Asuka::Vec2I Global_ResolutionPresets[12] = {
    { 800, 600 },
    { 960, 540 }, // Test Resolution
    { 1024, 768 },
    { 1152, 864 },
    { 1280, 1024 },
    { 1440, 900 },
    { 1600, 900 },
    { 1600, 1200 },
    { 1680, 1050 },
    { 1920, 1080 },
    { 1920, 1200 },
    { 2560, 1440 },
};


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(Win32_XInputGetStateT);
X_INPUT_GET_STATE(Win32_XInputGetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
GLOBAL Win32_XInputGetStateT* XInputGetState_ = Win32_XInputGetStateStub;
#define XInputGetState XInputGetState_


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(Win32_XInputSetStateT);
X_INPUT_SET_STATE(Win32_XInputSetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
GLOBAL Win32_XInputSetStateT* XInputSetState_ = Win32_XInputSetStateStub;
#define XInputSetState XInputSetState_


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(Win32_DirectSoundCreateT);


INTERNAL
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


INTERNAL
FILETIME Win32_GetFileTimestamp(const char* Filename) {
    FILETIME Result {};

    WIN32_FILE_ATTRIBUTE_DATA FileData;
    GetFileAttributesExA(Filename, GetFileExInfoStandard, &FileData);
    Result = FileData.ftLastWriteTime;

    return Result;
}


INTERNAL
Platform::GameDLL Win32_LoadGameDLL(const char* DllPath, const char* TempDllPath, const char *LockFilename)
{
#if ASUKA_DLL_BUILD
    Platform::GameDLL Result {};

    DWORD dwAttrib = GetFileAttributes(LockFilename);
    BOOL LockFileExists = (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

    if (!LockFileExists) {
        Result.Timestamp = Win32_GetFileTimestamp(DllPath);
        CopyFile(DllPath, TempDllPath, FALSE);
        Result.GameDLL = LoadLibraryA(TempDllPath);

        if (Result.GameDLL) {
            Result.InitializeMemory = (Game_InitializeMemoryT *)GetProcAddress(Result.GameDLL, "Game_InitializeMemory");
            Result.UpdateAndRender = (Game_UpdateAndRenderT *)GetProcAddress(Result.GameDLL, "Game_UpdateAndRender");
            Result.OutputSound = (Game_OutputSoundT *) GetProcAddress(Result.GameDLL, "Game_OutputSound");

            Result.IsValid = (Result.UpdateAndRender != NULL) && (Result.OutputSound != NULL);
        }
    }

    return Result;
#else
    Platform::GameDLL Result {};
    Result.IsValid = true;
    Result.UpdateAndRender = Game_UpdateAndRender;
    Result.OutputSound = Game_OutputSound;

    return Result;
#endif
}


INTERNAL
void Win32_UnloadGameDLL(Platform::GameDLL* GameCode) {
#if defined(ASUKA_DLL_BUILD)
    if (GameCode->GameDLL) {
        FreeLibrary(GameCode->GameDLL);
        GameCode->GameDLL = NULL;
    }

    GameCode->IsValid = false;
    GameCode->UpdateAndRender = NULL;
#endif
}


INTERNAL
void Win32_ProcessKeyEvent(Game::ButtonState* NewState, b32 IsDownNow) {
    if (NewState->EndedDown != IsDownNow) {
        NewState->EndedDown = IsDownNow;
        NewState->HalfTransitionCount++;
    }
}


INTERNAL
void Win32_ProcessXInputButton(
    Game::ButtonState* OldState,
    Game::ButtonState* NewState,
    DWORD XInputButtonState,
    DWORD ButtonBit)
{
    NewState->EndedDown = (XInputButtonState & ButtonBit) == ButtonBit;
    NewState->HalfTransitionCount = ((NewState->EndedDown != OldState->EndedDown) ? 1 : 0);
}


INTERNAL
f32 Win32_ProcessXInputStick(i16 value, i16 deadzone) {
    if (value < -deadzone) {
        return (f32)(value + deadzone) / (f32)(32768 - deadzone);
    } else if (value > deadzone) {
        return (f32)(value - deadzone) / (f32)(32767 - deadzone);
    }

    return 0.f;
}


INTERNAL
f32 Win32_ProcessXInputTrigger(u8 value, u8 deadzone) {
    if (value < deadzone) {
        return (f32)(value + deadzone) / (f32)(255 - deadzone);
    }

    return 0.f;
}


INTERNAL
void Win32_InitDirectSound(HWND Window, i32 SamplesPerSecond, i32 BufferSize) {
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


INTERNAL
Platform::WindowDimensions Win32_GetWindowDimention(HWND Window) {
    Platform::WindowDimensions Result;

    RECT ClientRectangle;
    GetClientRect(Window, &ClientRectangle);

    Result.Width = ClientRectangle.right - ClientRectangle.left;
    Result.Height = ClientRectangle.bottom - ClientRectangle.top;

    return Result;
}


INTERNAL
void Win32_ResizeDIBSection(Platform::OffscreenBuffer* Buffer, LONG Width, LONG Height) {
    if (Buffer->Memory) {
        Platform::FreeMemory(Buffer->Memory);
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
    Buffer->Memory = Platform::AllocateMemory(BitmapMemorySize);
}


INTERNAL
void Win32_CopyBufferToWindow(Platform::OffscreenBuffer *Buffer, HDC device_context, int WindowWidth, int WindowHeight) {
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


INTERNAL
void Win32_ClearSoundBuffer(Platform::SoundOutput* SoundOutput) {
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
        u8* DestMemory = (u8*)Region1;
        for (DWORD ByteIndex = 0; ByteIndex < Region1_Size; ByteIndex++) {
            *DestMemory++ = 0;
        }
        DestMemory = (u8*)Region2;
        for (DWORD ByteIndex = 0; ByteIndex < Region2_Size; ByteIndex++) {
            *DestMemory++ = 0;
        }

        Global_SecondaryBuffer->Unlock(Region1, Region1_Size, Region2, Region2_Size);
    }
}


INTERNAL
void Win32_FillSoundBuffer(
    Platform::SoundOutput* SoundOutput, DWORD BytesToLock, DWORD BytesToWrite,
    Game::SoundOutputBuffer* SourceBuffer)
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


INTERNAL
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

            Platform::WindowDimensions WindowSize = Win32_GetWindowDimention(Window);
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


void Win32_ProcessPendingMessages(HWND Window, Game::ControllerInput *Controller, Game::KeyboardState *Keyboard, Game::MouseState *Mouse)
{
    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        if (Message.message == WM_QUIT) Running = false;
        TranslateMessage(&Message);

        switch (Message.message)
        {
            case WM_LBUTTONDOWN:
            {
                Win32_ProcessKeyEvent(&Mouse->LMB, true);
                SetCapture(Window);
                DispatchMessageA(&Message);
            }
            break;

            case WM_LBUTTONUP:
            {
                Win32_ProcessKeyEvent(&Mouse->LMB, false);
                ReleaseCapture();
                DispatchMessageA(&Message);
            }
            break;

            // case WM_LBUTTONDBLCLK:

            case WM_MBUTTONDOWN:
            {
                Win32_ProcessKeyEvent(&Mouse->MMB, true);
                SetCapture(Window);
                DispatchMessageA(&Message);
            }
            break;

            case WM_MBUTTONUP:
            {
                Win32_ProcessKeyEvent(&Mouse->MMB, false);
                ReleaseCapture();
                DispatchMessageA(&Message);
            }
            break;

            // case WM_MBUTTONDBLCLK:

            case WM_RBUTTONDOWN:
            {
                Win32_ProcessKeyEvent(&Mouse->RMB, true);
                SetCapture(Window);
                DispatchMessageA(&Message);
            }
            break;

            case WM_RBUTTONUP:
            {
                Win32_ProcessKeyEvent(&Mouse->RMB, false);
                ReleaseCapture();
                DispatchMessageA(&Message);
            }
            break;


            // case WM_RBUTTONDBLCLK:

            // case WM_XBUTTONUP:
            // case WM_XBUTTONDOWN:
            // case WM_XBUTTONDBLCLK:

            // case WM_MOUSEHWHEEL:
            // case WM_MOUSEWHEEL:
            // case WM_NCLBUTTONUP:

            // case WM_NCLBUTTONDOWN:
            // case WM_NCLBUTTONDBLCLK:

            // case WM_NCMBUTTONUP:
            // case WM_NCMBUTTONDOWN:
            // case WM_NCMBUTTONDBLCLK:

            // case WM_NCRBUTTONUP:
            // case WM_NCRBUTTONDOWN:
            // case WM_NCRBUTTONDBLCLK:


            // case WM_NCMOUSEHOVER:
            // case WM_NCMOUSELEAVE:
            // case WM_NCMOUSEMOVE:

            // case WM_NCXBUTTONDBLCLK:
            // case WM_NCXBUTTONDOWN:
            // case WM_NCXBUTTONUP:
            //     break;

            // case WM_MOUSEMOVE:
            // {
            //     Mouse->PreviousPosition = Mouse->Position;
            //     Mouse->Position.x = (Message.lParam & 0x0000FFFF);
            //     Mouse->Position.y = (Message.lParam & 0xFFFF0000) >> 16;
            //     break;
            // }

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 VKCode  = (u32)Message.wParam;
                b32 AltDown = (Message.lParam & (1 << 29));
                b32 WasDown = (Message.lParam & (1 << 30)) != 0;
                b32 IsDown  = (Message.lParam & (1 << 31)) == 0;

                if (WasDown != IsDown) {
                    switch (VKCode)
                    {
                        case VK_ESCAPE:
                        {
                            Running = false;
                            Win32_ProcessKeyEvent(&Controller->Back, IsDown);
                        }
                        break;

                        case VK_SPACE:
                        {
                            Win32_ProcessKeyEvent(&Controller->Start, IsDown);
                        }
                        break;

                        case 'W':
                        {
                            if (IsDown) {
                                Controller->LeftStickEnded.y = Asuka::clamp(Controller->LeftStickEnded.y + 1, -1, 1);
                            } else {
                                Controller->LeftStickEnded.y = Asuka::clamp(Controller->LeftStickEnded.y - 1, -1, 1);
                            }
                        }
                        break;

                        case 'A':
                        {
                            if (IsDown) {
                                Controller->LeftStickEnded.x = Asuka::clamp(Controller->LeftStickEnded.x - 1, -1, 1);
                            } else {
                                Controller->LeftStickEnded.x = Asuka::clamp(Controller->LeftStickEnded.x + 1, -1, 1);
                            }
                        }
                        break;

                        case 'S':
                        {
                            if (IsDown) {
                                Controller->LeftStickEnded.y = Asuka::clamp(Controller->LeftStickEnded.y - 1, -1, 1);
                            } else {
                                Controller->LeftStickEnded.y = Asuka::clamp(Controller->LeftStickEnded.y + 1, -1, 1);
                            }
                        }
                        break;

                        case 'D':
                        {
                            if (IsDown) {
                                Controller->LeftStickEnded.x = Asuka::clamp(Controller->LeftStickEnded.x + 1, -1, 1);
                            } else {
                                Controller->LeftStickEnded.x = Asuka::clamp(Controller->LeftStickEnded.x - 1, -1, 1);
                            }
                        }
                        break;

                        case 'Q':
                        {
                            Win32_ProcessKeyEvent(&Controller->ShoulderLeft, IsDown);
                        }
                        break;

                        case 'E':
                        {
                            Win32_ProcessKeyEvent(&Controller->ShoulderRight, IsDown);
                        }
                        break;

                        case 'Z':
                        {
                            Win32_ProcessKeyEvent(&Keyboard->Z, IsDown);
                        }
                        break;

                        case VK_UP:
                        {
                            Win32_ProcessKeyEvent(&Controller->Y, IsDown);
                        }
                        break;

                        case VK_DOWN:
                        {
                            Win32_ProcessKeyEvent(&Controller->A, IsDown);
                        }
                        break;

                        case VK_LEFT:
                        {
                            Win32_ProcessKeyEvent(&Controller->X, IsDown);
                        }
                        break;

                        case VK_RIGHT:
                        {
                            Win32_ProcessKeyEvent(&Controller->B, IsDown);
                        }
                        break;

                        case VK_F1:
                        {
                            Win32_ProcessKeyEvent(&Keyboard->F1, IsDown);
                        }
                        break;

                        case VK_F2:
                        {
                            Win32_ProcessKeyEvent(&Keyboard->F2, IsDown);
                        }
                        break;

                        case VK_F3:
                        {
                            Win32_ProcessKeyEvent(&Keyboard->F3, IsDown);
                        }
                        break;

                        case VK_F4:
                        {
                            Win32_ProcessKeyEvent(&Keyboard->F4, IsDown);
                        }
                        break;
                    }

#if ASUKA_PLAYBACK_LOOP
                    if (VKCode == 'L') {
                        if (IsDown == FALSE) {
                            if (Global_DebugInputRecording.PlaybackLoopState == Game::PLAYBACK_LOOP_IDLE) {
                                Global_DebugInputRecording.RecordedInputsCount = 0;
                                Global_DebugInputRecording.PlaybackLoopState = Game::PLAYBACK_LOOP_RECORDING;
                            } else if (Global_DebugInputRecording.PlaybackLoopState == Game::PLAYBACK_LOOP_RECORDING) {
                                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
                                Global_DebugInputRecording.PlaybackLoopState = Game::PLAYBACK_LOOP_PLAYBACK;
                            } else if (Global_DebugInputRecording.PlaybackLoopState == Game::PLAYBACK_LOOP_PLAYBACK) {
                                // Nullify keyboard controller such as nothing is pressed on stoping the playback loop
                                // because if there's something left pressed, it will stay pressed although nothing is
                                // pressed on the actual keyboard
                                *Controller = {};
                                Global_DebugInputRecording.PlaybackLoopState = Game::PLAYBACK_LOOP_IDLE;
                            }
                        }
                    } else if (VKCode == 'K') {
                        if (IsDown == FALSE) {
                            if ((Global_DebugInputRecording.PlaybackLoopState == Game::PLAYBACK_LOOP_IDLE) &&
                                (Global_DebugInputRecording.RecordedInputsCount > 0))
                            {
                                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
                                Global_DebugInputRecording.PlaybackLoopState = Game::PLAYBACK_LOOP_PLAYBACK;
                            }
                            else if (Global_DebugInputRecording.PlaybackLoopState == Game::PLAYBACK_LOOP_PLAYBACK)
                            {
                                // Nullify keyboard controller such as nothing is pressed on stoping the playback loop
                                // because if there's something left pressed, it will stay pressed although nothing is
                                // pressed on the actual keyboard
                                *Controller = {};
                                Global_DebugInputRecording.PlaybackLoopState = Game::PLAYBACK_LOOP_IDLE;
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


Asuka::Vec2I Win32_GetMousePosition(HWND Window)
{
    Asuka::Vec2I Position;

    POINT MousePosition;
    GetCursorPos(&MousePosition);
    ScreenToClient(Window, &MousePosition);

    Position.x = MousePosition.x;
    Position.y = MousePosition.y;

    return Position;
}


#if DRAW_DEBUG_SOUND_CURSORS
INTERNAL
void Win32_DebugDrawRectangle(
    Platform::OffscreenBuffer* ScreenBuffer,
    Asuka::Vec2F MinCorner,
    Asuka::Vec2F MaxCorner,
    u32 Color,
    bool Dotted = false)
{
    Asuka::Vec2I tl = Asuka::round_to_v2i(MinCorner);
    Asuka::Vec2I br = Asuka::round_to_v2i(MaxCorner);

    if (tl.x < 0) tl.x = 0;
    if (tl.y < 0) tl.y = 0;
    if (br.x > ScreenBuffer->Width)  br.x = ScreenBuffer->Width;
    if (br.y > ScreenBuffer->Height) br.y = ScreenBuffer->Height;

    Asuka::Vec2I dimensions = br - tl;

    u8* Row = (u8*)ScreenBuffer->Memory + tl.y*ScreenBuffer->Pitch + tl.x*ScreenBuffer->BytesPerPixel;

    if (Dotted)
    {
        u8* Pixel = (u8*) ScreenBuffer->Memory
                     + tl.x*ScreenBuffer->BytesPerPixel
                     + tl.y*ScreenBuffer->Pitch;
        int DashHeight_InPixels = 4;
        for (int Y = tl.y; Y < br.y; Y += 2*DashHeight_InPixels) {
            for (int DashY = 0; DashY < DashHeight_InPixels / 2; DashY++) {
                *(u32*)Pixel = Color;
                Pixel += ScreenBuffer->Pitch;
            }
            Pixel += ScreenBuffer->Pitch * (DashHeight_InPixels + DashHeight_InPixels / 2 + DashHeight_InPixels % 2);
        }
    }
    else
    {
        for (int y = 0; y < dimensions.y; y++) {
            u32* Pixel = (u32*) Row;

            for (int x = 0; x < dimensions.x; x++) {
                *Pixel = Color;
                Pixel++;
            }

            Row += ScreenBuffer->Pitch;
        }
    }

    // int Bottom = int(MaxCorner.y);
    // int X = int(MinCorner.x);

    // if (Bottom >= ScreenBuffer->Height) {
    //     Bottom = ScreenBuffer->Height;
    // }


    // if (Dotted) {
    // } else {
    //     for (int Y = Top; Y < Bottom; Y++) {
    //         *(u32*)Pixel = Color;
    //         Pixel += ScreenBuffer->Pitch;
    //     }
    // }
}


INTERNAL
Float32 Win32_DebugFourierSound(Game::SoundOutputBuffer *SoundBuffer, Float32 Frequency)
{
    Float32 IntegralValue = 0;

    sound_sample_t *Sample = SoundBuffer->Samples;
    for (Int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++)
    {
        IntegralValue += (Asuka::absolute(*Sample++) * 1.0f);
        IntegralValue += (Asuka::absolute(*Sample++) * 1.0f);
    }
}


INTERNAL
void Win32_DebugDrawVolume(
    Platform::OffscreenBuffer *ScreenBuffer,
    Game::SoundOutputBuffer *SoundBuffer)
{
    Float32 VolumeL = 0;
    Float32 VolumeR = 0;

    sound_sample_t *Sample = SoundBuffer->Samples;
    for (Int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++)
    {
        VolumeL += Asuka::absolute(*Sample++);
        VolumeR += Asuka::absolute(*Sample++);
    }

    PERSIST Float32 VolumeL_[10] = {};
    PERSIST Float32 VolumeR_[10] = {};
    PERSIST Int32   VolumeIndex = 0;

    VolumeL /= (SoundBuffer->SampleCount);
    VolumeR /= (SoundBuffer->SampleCount);

    VolumeL_[VolumeIndex] = VolumeL;
    VolumeR_[VolumeIndex] = VolumeR;

    VolumeIndex = (VolumeIndex + 1) % 10;

    // Draw sound volume for left and right channels
    {
        Float32 L = 0;
        Float32 R = 0;
        for (Int32 i = 0; i < 10; i++)
        {
            L += VolumeL_[i];
            R += VolumeR_[i];
        }

        L /= 1000;
        R /= 1000;

        Win32_DebugDrawRectangle(ScreenBuffer, Asuka::V2(0, ScreenBuffer->Height - L), Asuka::V2(50, ScreenBuffer->Height), 0xFFFFFFFF);
        Win32_DebugDrawRectangle(ScreenBuffer, Asuka::V2(ScreenBuffer->Width - 50, ScreenBuffer->Height - R), Asuka::V2(ScreenBuffer->Width, ScreenBuffer->Height), 0xFFFFFFFF);
    }
}

INTERNAL
void Win32_DebugSoundDisplay(
    Platform::OffscreenBuffer* ScreenBuffer,
    Platform::SoundOutput* SoundOutput,
    Platform::DebugSoundCursors* Cursors,
    u32  CursorCount,
    u32  CurrentCursorIndex,
    f32 SecondsPerFrame)
{
    int PadX = 16;
    int PadY = 16;
    int LineHeight = 20;

    f32 C = (f32) (ScreenBuffer->Width - 2*PadX) / (f32) SoundOutput->SecondaryBufferSize;

    for (u32 CursorIndex = 0; CursorIndex < CursorCount; CursorIndex++) {
        int Top    = PadY + LineHeight*CursorIndex;
        int Bottom = PadY + LineHeight*(CursorIndex + 1);

        {
            int X = PadX + (int)(C * (f32)Cursors[CursorIndex].PlayCursor);
            u32 Color = CursorIndex == CurrentCursorIndex ? 0xFFFF00FF : 0xFFFFFFFF;
            Win32_DebugDrawRectangle(ScreenBuffer, Asuka::V2(X, Top), Asuka::V2(X+1, Bottom), Color, false);
        }
        {
            int X = PadX + (int)(C * (f32)Cursors[CursorIndex].WriteCursor);
            Win32_DebugDrawRectangle(ScreenBuffer, Asuka::V2(X, Top), Asuka::V2(X+1, Bottom), 0xFFFF0000, false);
        }
        {
            int X = PadX + (int)(C * (f32)Cursors[CursorIndex].OutputLocationStart);
            Win32_DebugDrawRectangle(ScreenBuffer, Asuka::V2(X, Top), Asuka::V2(X+1, Bottom), 0xFF00FF00, false);
        }
        {
            int X = PadX + (int)(C * (f32)Cursors[CursorIndex].OutputLocationEnd);
            Win32_DebugDrawRectangle(ScreenBuffer, Asuka::V2(X, Top), Asuka::V2(X+1, Bottom), 0xFF0000FF, false);
        }
        {
            int X = PadX + (int)(C * (f32)Cursors[CursorIndex].PageFlip);
            Win32_DebugDrawRectangle(ScreenBuffer, Asuka::V2(X, Top), Asuka::V2(X+1, Bottom), 0xFFFFFF00, true);
        }
        {
            int X = PadX + (int)(C * (f32)Cursors[CursorIndex].ExpectedNextPageFlip);
            Win32_DebugDrawRectangle(ScreenBuffer, Asuka::V2(X, Top), Asuka::V2(X+1, Bottom), 0xFF00FFFF, true);
        }

    }
}
#endif // DRAW_DEBUG_SOUND_CURSORS


INTERNAL
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


f32 my_sin(f32 x)
{
    f32 result = x - x * x * x / 6 + x * x * x * x * x / 120 - x * x * x * x * x * x * x / 5040;
    return result;
}


THREAD_FUNCTION(ThreadTest)
{
    for (int i = 0; i < 10; i++)
    {
        osOutputDebugString("%d\n", i);
    }

    return 0;
}


int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CmdLine,
    int CmdShow)
{
#if 0
    u32 dct = GetDoubleClickTime();
    printf("dct=%u\n", dct);
#endif

#if 0
    for (f32 x = -1000.0f; x < 1050.0f; x += 10)
    {
        f32 std_result = sinf(x);
        f32  my_result = my_sin(x);

        osOutputDebugString("x=%f; sin=%f; my_sin=%f; err=%f\n", x, std_result, my_result, Asuka::absolute(std_result-my_result));
    }

    return 0;
#endif

    UINT DesiredSchedulerGranularityMS = 1; // ms
    MMRESULT TimeBeginPeriodResult = timeBeginPeriod(DesiredSchedulerGranularityMS); // Set this so that sleep granularity
    bool SleepIsGranular = (TimeBeginPeriodResult == TIMERR_NOERROR);

    char ProgramPath [256] {};
    DWORD ProgramPathSize = GetModuleFileNameA(Instance, ProgramPath, 256);

    DWORD IndexOfLastSlash = 0;
    for (DWORD CharIndex = 0; CharIndex < ProgramPathSize; CharIndex++) {
        if (ProgramPath[CharIndex] == '\\') {
            IndexOfLastSlash = CharIndex;
        }
    }
    DWORD ProgramPathNoFilenameSize = IndexOfLastSlash + 1;

    const i32 ResolutionIndex = 1;
    STATIC_ASSERT_MSG(ResolutionIndex < ARRAY_COUNT(Global_ResolutionPresets), "Resolution Index is too high");
    Asuka::Vector2i Resolution = Global_ResolutionPresets[ResolutionIndex];

    Win32_LoadXInputFunctions();
    Win32_ResizeDIBSection(&Global_BackBuffer, Resolution[0], Resolution[1]);

#ifdef ASUKA_DEBUG
    Global_CursorIsVisible = true;
#endif

    WNDCLASSA WindowClass{};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "AsukaWindowClass";
    // HICON     hIcon;
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    ATOM ClassAtomResult = RegisterClassA(&WindowClass);
    if (!ClassAtomResult) {
        Platform::ErrorPopup("Could not register window class");
        return 1; // @todo: Should make up error codes for errors?
    }

    RECT WindowRectangle { 0, 0, Resolution[0], Resolution[1] };
    if (!AdjustWindowRect(&WindowRectangle, WS_OVERLAPPEDWINDOW, false)) {
        Platform::ErrorPopup("Could not adjust window rectangle for some reason");
        return 1;
    }

#if DEBUG_WINDOW_ON_TOP
    DWORD WindowExStyle = WS_EX_TOPMOST | WS_EX_LAYERED;
#else
    DWORD WindowExStyle = 0;
#endif
    DWORD WindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE; // WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    HWND Window = CreateWindowExA(
        WindowExStyle,                    // ExStyle
        WindowClass.lpszClassName,        // ClassName
        "AsukaWindow",                    // WindowName
        WindowStyle,                      // Style
        CW_USEDEFAULT,                    // X,
        CW_USEDEFAULT,                    // Y,
        Platform::Width(WindowRectangle),           // Width
        Platform::Height(WindowRectangle),          // Height
        0,                                // WndParent
        0,                                // Menu
        Instance,                         // Instance
        NULL);                            // Param

    if (!Window) {
        Platform::ErrorPopup("Could not create window");
        return 1;
    }

    int MonitorRefreshHz;
    {
        HDC DeviceContext = GetDC(Window);
        MonitorRefreshHz = GetDeviceCaps(DeviceContext, VREFRESH);
        ReleaseDC(Window, DeviceContext);
    }

    // @todo: do something with that something intelligent when we will add vsync.
    int GameUpdateHz = 30; // MonitorRefreshHz / 2;
    f32 TargetSecondsPerFrame = 1.f / GameUpdateHz;

    Platform::SoundOutput SoundOutput {};

    SoundOutput.SamplesPerSecond = 48000;
    SoundOutput.RunningSoundCursor = 0;
    SoundOutput.ChannelCount = 2;
    SoundOutput.BytesPerSoundFrame = sizeof(sound_sample_t) * SoundOutput.ChannelCount;
    SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSoundFrame;
    SoundOutput.SafetyBytes = (u32) (1.5f * SoundOutput.BytesPerSoundFrame * SoundOutput.SamplesPerSecond / GameUpdateHz);

    Win32_InitDirectSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
    Win32_ClearSoundBuffer(&SoundOutput);
    Global_SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

    SoundOutput.Samples = (sound_sample_t*) Platform::AllocateMemory(SoundOutput.SecondaryBufferSize);

#ifdef ASUKA_DEBUG
    LPVOID BaseAddress = (LPVOID)TERABYTES(1);
#else
    LPVOID BaseAddress = 0;
#endif

    Game::Memory GameMemory{};
    GameMemory.PermanentStorageSize = MEGABYTES(256);
    GameMemory.TransientStorageSize = GIGABYTES(1);

    u64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    GameMemory.PermanentStorage = Platform::AllocateMemory(BaseAddress, TotalSize);
    GameMemory.TransientStorage = (u8*)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;

#if ASUKA_PLAYBACK_LOOP
    u64 InitialGameMemorySize = GameMemory.PermanentStorageSize;

    Global_DebugInputRecording.InitialGameMemorySize = InitialGameMemorySize;
    Global_DebugInputRecording.InputRecordingSize = MEGABYTES(1);

    Global_DebugInputRecording.InitialGameMemory = VirtualAlloc(
        (u8*)GameMemory.PermanentStorage + TotalSize,
        InitialGameMemorySize + Global_DebugInputRecording.InputRecordingSize, // GameMemory size + size of the recorded inputs
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Global_DebugInputRecording.InputRecording = (u8 *)Global_DebugInputRecording.InitialGameMemory + InitialGameMemorySize;

    Global_DebugInputRecording.RecordedInputsCount = 0;
    Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
    Global_DebugInputRecording.PlaybackLoopState = Game::PLAYBACK_LOOP_IDLE;
#endif // ASUKA_PLAYBACK_LOOP

#if DRAW_DEBUG_SOUND_CURSORS
    Platform::DebugSoundCursors Debug_Cursors[30] {};
    u32 Debug_SoundCursorIndex = 0;
#endif // DRAW_DEBUG_SOUND_CURSORS

    Running = true;
    int FrameCounter = 0;

    Thread GameThread {};
    Thread SoundThread {};

#if 0
    Platform::Thread SoundThread_ = Platform::CreateThread(ThreadTest);
    osOutputDebugString("ThreadId=%ld; ThreadHandle=%p\n", SoundThread_.Id, SoundThread_.Handle);
    JoinThread(SoundThread_);

    return 0;
#endif

    Game::Input Input[2] {};
    Game::Input* OldInput = &Input[0];
    Game::Input* NewInput = &Input[1];

    OldInput->dt = TargetSecondsPerFrame;
    NewInput->dt = TargetSecondsPerFrame;

    char GameDllFilename[] = "Asuka.dll";
    char GameTempDllFilename[] = "asuka_running.dll";
    char LockFilename[] = "lock.tmp";

    char GameDllFilepath [256];
    char GameTempDllFilepath [256];
    char LockFilepath [256];
    char WindowText [256];

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

    Platform::GameDLL Game = Win32_LoadGameDLL(GameDllFilepath, GameTempDllFilepath, LockFilepath);

    os::timepoint LastClockTimepoint = os::get_wall_clock();
    while (Running) {
        FrameCounter += 1;

        FILETIME GameDllTimestamp = Win32_GetFileTimestamp(GameDllFilepath);
        if (CompareFileTime(&GameDllTimestamp, &Game.Timestamp) != 0) {
            Win32_UnloadGameDLL(&Game);
            Game = Win32_LoadGameDLL(GameDllFilepath, GameTempDllFilepath, LockFilepath);
        }

        Game::ControllerInput* OldKeyboardController = &OldInput->KeyboardInput;
        Game::ControllerInput* NewKeyboardController = &NewInput->KeyboardInput;
        NewInput->mouse.previous_position = NewInput->mouse.position;
        NewInput->mouse.position = Win32_GetMousePosition(Window);

        NewInput->keyboard = OldInput->keyboard;
        *NewKeyboardController = *OldKeyboardController;

        for (u32 ButtonIndex = 0; ButtonIndex < ARRAY_COUNT(NewInput->mouse.buttons); ButtonIndex++)
        {
            NewInput->mouse.buttons[ButtonIndex].HalfTransitionCount = 0;
        }
        for (u32 ButtonIndex = 0; ButtonIndex < ARRAY_COUNT(NewKeyboardController->Buttons); ButtonIndex++)
        {
            NewKeyboardController->Buttons[ButtonIndex].HalfTransitionCount = 0;
        }
        for (u32 ButtonIndex = 0; ButtonIndex < ARRAY_COUNT(NewInput->keyboard.buttons); ButtonIndex++)
        {
            NewInput->keyboard.buttons[ButtonIndex].HalfTransitionCount = 0;
        }

        Win32_ProcessPendingMessages(Window, NewKeyboardController, &NewInput->keyboard, &NewInput->mouse);
        OldInput->mouse = NewInput->mouse;

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

                Game::ControllerInput* OldGamepadInput = GetGamepadInput(OldInput, GamepadIndex);
                Game::ControllerInput* NewGamepadInput = GetGamepadInput(NewInput, GamepadIndex);
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
        // f32 AudioLatencySeconds = 0;

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
            // AudioLatencySeconds = ((f32)AudioLatencyBytes / (f32)SoundOutput.BytesPerSoundFrame) / (f32) SoundOutput.SamplesPerSecond;

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

        Game::SoundOutputBuffer SoundBuffer {};
        SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
        SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSoundFrame;
        SoundBuffer.Samples = SoundOutput.Samples;

        Game::OffscreenBuffer ScreenBuffer;
        ScreenBuffer.Memory = Global_BackBuffer.Memory;
        ScreenBuffer.Width  = Global_BackBuffer.Width;
        ScreenBuffer.Height = Global_BackBuffer.Height;
        ScreenBuffer.Pitch  = Global_BackBuffer.Pitch;
        ScreenBuffer.BytesPerPixel = Global_BackBuffer.BytesPerPixel;

#if ASUKA_PLAYBACK_LOOP
        NewInput->PlaybackLoopState = Global_DebugInputRecording.PlaybackLoopState;

        if (Global_DebugInputRecording.PlaybackLoopState == Game::PLAYBACK_LOOP_RECORDING) {
            // Just started recording, save initial game memory
            if (Global_DebugInputRecording.RecordedInputsCount == 0) {
                CopyMemory(
                    Global_DebugInputRecording.InitialGameMemory,
                    GameMemory.PermanentStorage,
                    Global_DebugInputRecording.InitialGameMemorySize);
            }

            // Checking if there's room for one more Game_Input
            if (((Global_DebugInputRecording.RecordedInputsCount + 1) * sizeof(Game::Input)) < Global_DebugInputRecording.InputRecordingSize) {
                Game::Input* RecordedInputs = (Game::Input*)Global_DebugInputRecording.InputRecording;
                RecordedInputs[Global_DebugInputRecording.RecordedInputsCount] = *NewInput;
                RecordedInputs[Global_DebugInputRecording.RecordedInputsCount].PlaybackLoopState = Game::PLAYBACK_LOOP_PLAYBACK;
                Global_DebugInputRecording.RecordedInputsCount++;
            } else {
                ASSERT_FAIL("The space for storing input is ran out. Increase it in code above (it's debug code, nobody cares)\n");
            }
        }

        if (Global_DebugInputRecording.PlaybackLoopState == Game::PLAYBACK_LOOP_PLAYBACK) {
            // If the playback started over, copy initial game memory back to the game memory storage
            if (Global_DebugInputRecording.CurrentPlaybackInputIndex == 0) {
                CopyMemory(
                    GameMemory.PermanentStorage,
                    Global_DebugInputRecording.InitialGameMemory,
                    Global_DebugInputRecording.InitialGameMemorySize);
            }

            Game::Input* RecordedInputs = (Game::Input*)Global_DebugInputRecording.InputRecording;
            *NewInput = RecordedInputs[Global_DebugInputRecording.CurrentPlaybackInputIndex++];

            if (Global_DebugInputRecording.CurrentPlaybackInputIndex == Global_DebugInputRecording.RecordedInputsCount) {
                Global_DebugInputRecording.CurrentPlaybackInputIndex = 0;
            }
        }
#endif // ASUKA_PLAYBACK_LOOP

        if (Game.UpdateAndRender)
        {
            Game.UpdateAndRender(&GameThread, &GameMemory, NewInput, &ScreenBuffer);
        }

        if (Game.OutputSound)
        {
            Game.OutputSound(&SoundThread, &GameMemory, &SoundBuffer);
        }

        Win32_FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);

        os::timepoint WorkCounter = os::get_wall_clock();
        f32 SecondsElapsedForWork = get_seconds(WorkCounter - LastClockTimepoint);

        f32 SecondsElapsedForFrame = SecondsElapsedForWork;
        if (SecondsElapsedForFrame < TargetSecondsPerFrame) {
            if (SleepIsGranular) {
                DWORD SleepMS = truncate_cast_to_uint32(1000 * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                if (SleepMS > 0) {
                    // @todo
                    // Sleep(SleepMS);
                }
            }

            // while (SecondsElapsedForFrame < TargetSecondsPerFrame) {
            //     SecondsElapsedForFrame = os::get_seconds(os::get_wall_clock() - LastClockTimepoint);
            // }

            if (SecondsElapsedForFrame < TargetSecondsPerFrame) {
                // @todo: Slept for good time!
            } else {
                // OutputDebugStringA("MISSED FRAME!\n");
                // @todo: handle missed frame rate!
            }
        } else {
            // OutputDebugStringA("MISSED FRAME!\n");
            // @todo: handle missed frame rate!
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
            Win32_DebugDrawVolume(&Global_BackBuffer, &SoundBuffer);

            Debug_SoundCursorIndex = (Debug_SoundCursorIndex + 1) % ARRAY_COUNT(Debug_Cursors);
        }
#endif

        {
            HDC DeviceContext = GetDC(Window);

            Platform::WindowDimensions WindowSize = Win32_GetWindowDimention(Window);
            Win32_CopyBufferToWindow(&Global_BackBuffer, DeviceContext, WindowSize.Width, WindowSize.Height);

            ReleaseDC(Window, DeviceContext);
        }

        {
            f32 MilliSecondsElapsed = 1000.f * SecondsElapsedForFrame;
            f32 FPS = (f32)1.f / SecondsElapsedForFrame;

            memset(WindowText, 0, sizeof(WindowText));
            // sprintf(WindowText, "MouseP (%d, %d)", NewInput->mouse.position.x, NewInput->mouse.position.y);
            sprintf(WindowText, "%f ms/f; FPS: %f\n", MilliSecondsElapsed, FPS);

            SetWindowText(Window, WindowText);
        }

        Game::Input* TmpInput = NewInput;
        NewInput = OldInput;
        OldInput = TmpInput;
    }

    return 0;
}
