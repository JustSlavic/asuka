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
#include <intrin.h>

// Windows
#include <windows.h>

// XInput
#include <xinput.h>

// Direct Sound
#include <dsound.h>


#define DRAW_DEBUG_SOUND_CURSORS (ASUKA_DEBUG && 0)
#define DEBUG_WINDOW_ON_TOP (ASUKA_DEBUG && 0)

#define THREAD_FUNCTION(NAME) DWORD NAME(LPVOID Parameter)


namespace Platform {


typedef THREAD_FUNCTION(ThreadFunction);


struct Thread
{
    DWORD  Id;
    HANDLE Handle;
};


INLINE
Thread CreateThread(ThreadFunction *Function, LPVOID Parameter = NULL)
{
    Thread Result = {};
    Result.Handle = ::CreateThread(
        NULL,             // ThreadAttributes
        0,                // StackSize
        Function,         // StartAddress
        Parameter,        // Parameter
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


INLINE
void DetatchThread(Thread ChildThread)
{
    CloseHandle(ChildThread.Handle);
}


// I made these memory routines just to hide the Flag arguments from the main code.
INLINE
void *AllocateMemory(void *BaseAddress, usize Size)
{
    void *Result = VirtualAlloc(BaseAddress, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    ASSERT_MSG(Result, "VirtualAlloc failed.");
    return Result;
}

INLINE
void *AllocateMemory(usize Size)
{
    void *Result = AllocateMemory(NULL, Size);
    return Result;
}

INLINE
void FreeMemory(void *Memory)
{
    BOOL Success = VirtualFree(Memory, 0, MEM_RELEASE);
    ASSERT_MSG(Success, "VirtualFree failed.");
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


void EnableVTCodes()
{
    HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD ConsoleMode;
    GetConsoleMode(Console, &ConsoleMode);
    ConsoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(Console, ConsoleMode);
}


//  =========== Saved this for later ===============
// WIN32_FIND_DATAA FoundFile = {};
// HANDLE FileHandle = FindFirstFile("C:\\Projects\\asuka\\tests\\acf\\positive\\*", &FoundFile);

// if (FileHandle == INVALID_HANDLE_VALUE)
// {
//     printf("Filepath is wrong.\n");
// }
// else
// {
//     do {
//         auto FileAttributes = FoundFile.dwFileAttributes;
//         if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//         {
//             // Skip directories.
//         }
//         else
//         {
//             char filename_buffer[256];
//             memory::set(filename_buffer, 0, sizeof(filename_buffer));
//             sprintf(filename_buffer, "positive\\%s", FoundFile.cFileName);

//             auto filename = string::from(filename_buffer);
//             if ((filename[filename.size - 4] != '.') ||
//                 (filename[filename.size - 3] != 'a') ||
//                 (filename[filename.size - 2] != 'c') ||
//                 (filename[filename.size - 1] != 'f'))
//             {
//                 continue;
//             }

//             if (run_acf_test(filename))
//             {
//                 result.successfull += 1;
//             }
//             else
//             {
//                 result.failed += 1;
//             }
//         }
//     } while (FindNextFile(FileHandle, &FoundFile));

//     if (GetLastError() != ERROR_NO_MORE_FILES)
//     {
//         printf("Could not real whole directory for some reason.\n");
//     }

//     FindClose(FileHandle);
// }
//  =========== Saved this for later ===============


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
