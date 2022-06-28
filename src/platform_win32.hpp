/*
    Platform Layer:

    - Saved game locations
    - Getting a handle to our own executable file
    - Asset loading path
    - Threading (launch a thread)
    - Raw input (support for multiple keyboards)
    - ClipCursor() (multimonitor support)
    - Fullscreen support
    - WM_SETCURSOR (control cursor visibility)
    - QueryCancelAutoplay
    - WM_ACTIVATEAPP (for when we are not the active application)
    - Blit speed improvements (BitBlt)
    - Hardware acceleration (OpenGL or Direct3D)
    - GetKeyboardLayout (for French keyboards, international WASD support)

    - ChangeDisplaySettings ?

    Just a partial list of stuff
*/


#include <defines.hpp>

// Standrad headers
#include <stdio.h>

// Windows
#include <windows.h>

// XInput
#include <xinput.h>

// Direct Sound
#include <dsound.h>


#define DRAW_DEBUG_SOUND_CURSORS (ASUKA_DEBUG && 0)
#define DEBUG_WINDOW_ON_TOP (ASUKA_DEBUG && 0)

#define THREAD_FUNCTION(NAME) DWORD NAME(LPVOID Parameters)


namespace Platform {


typedef THREAD_FUNCTION(ThreadFunction);


struct Thread
{
    DWORD  Id;
    HANDLE Handle;
};


INLINE
Thread CreateThread(ThreadFunction *Function)
{
    Thread Result = {};
    Result.Handle = ::CreateThread(
        NULL,             // ThreadAttributes
        0,                // StackSize
        Function,         // StartAddress
        NULL,             // Parameter
        0,                // CreationFlags
        &Result.Id);      // pThreadId

    return Result;
}


INLINE
void JoinThread(Thread ChildThread, DWORD Milliseconds = INFINITE)
{
    WaitForSingleObject(ChildThread.Handle, Milliseconds);
    CloseHandle(ChildThread.Handle);
}


struct MemoryBlock
{
    void *Memory;
    usize Size;
};


MemoryBlock AllocateMemoryBlock(void *BaseAddress, usize Size)
{
    MemoryBlock Result = {};
    Result.Memory = VirtualAlloc(BaseAddress, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (Result.Memory) {
        Result.Size = Size;
    }

    return Result;
}


void DeallocateMemoryBlock(MemoryBlock Block)
{
    VirtualFree(Block.Memory, 0, MEM_RELEASE);
}


struct GameDLL
{
    HMODULE GameDLL;
    Game_InitializeMemoryT *InitializeMemory;
    Game_UpdateAndRenderT  *UpdateAndRender;
    Game_OutputSoundT      *OutputSound;

    FILETIME Timestamp;

    b32 IsValid;
};


struct OffscreenBuffer
{
    // Pixels are always 32-bits wide Little Endian, Memory Order BBGGRRxx
    BITMAPINFO Info;
    void* Memory;
    i32 Width;
    i32 Height;
    i32 Pitch;
    i32 BytesPerPixel;
};


struct WindowDimensions
{
    i32 Width;
    i32 Height;
};


struct SoundOutput
{
    u32 SamplesPerSecond;
    u32 RunningSoundCursor;
    u32 ChannelCount;
    u32 BytesPerSoundFrame;
    u32 SecondaryBufferSize;
    u32 SafetyBytes;
    sound_sample_t* Samples;
};


#ifdef DRAW_DEBUG_SOUND_CURSORS
struct DebugSoundCursors
{
    u32 PlayCursor;
    u32 WriteCursor;

    u32 OutputLocationStart;
    u32 OutputLocationEnd;

    u32 PageFlip;
    u32 ExpectedNextPageFlip;
};
#endif // DRAW_DEBUG_SOUND_CURSORS


#if ASUKA_PLAYBACK_LOOP
struct DebugInputRecording
{
    u64 InitialGameMemorySize;
    void*  InitialGameMemory;

    // Storage of recorded inputs
    u64 InputRecordingSize;
    void * InputRecording;

    // Where playback is currently replaying input
    u64 CurrentPlaybackInputIndex;
    // How many input frames recorded to playback
    u64 RecordedInputsCount;

    Game::Debug_PlaybackLoopState PlaybackLoopState;
};
#endif // ASUKA_PLAYBACK_LOOP


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


void ErrorPopup(char const *Message)
{
    MessageBeep(MB_ICONERROR);
    MessageBoxA(0, Message, "Win32 Platform Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
}


} // namespace Platform

GLOBAL Platform::DebugInputRecording Global_DebugInputRecording;
