#include "file.hpp"

// @todo: get rid of windows.h here
#include <windows.h>


namespace os {
namespace internal {


string load_entire_file(const char* filename) {
    string result {};

    HANDLE FileHandle = CreateFileA(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (FileHandle == INVALID_HANDLE_VALUE) {
        // @todo: log error
        return result;
    }

    LARGE_INTEGER FileSize;
    BOOL GetSizeResult = GetFileSizeEx(FileHandle, &FileSize);
    if (GetSizeResult == 0) {
        // @todo: log error
        CloseHandle(FileHandle);
        return result;
    }

    ASSERT(FileSize.QuadPart <= UINT32_MAX);

    void* Memory = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    DWORD BytesRead;
    BOOL ReadFileResult = ReadFile(FileHandle, Memory, (DWORD)FileSize.QuadPart, &BytesRead, NULL);

    CloseHandle(FileHandle);

    if (ReadFileResult == FALSE && BytesRead == FileSize.QuadPart) {
        // @todo: log error
        VirtualFree(Memory, 0, MEM_RELEASE);
        return result;
    }

    result.data = (uint8 *) Memory;
    result.size = FileSize.QuadPart;
    return result;
}


bool write_file(const char* filename, string file) {
    HANDLE FileHandle = CreateFileA(
        filename,
        GENERIC_WRITE,
        NULL,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (FileHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD BytesWritten = 0;
    // WriteFile(FileHandle, file.memory, file.size, &BytesWritten, NULL);

    CloseHandle(FileHandle);

    return file.size == BytesWritten;
}


} // internal
} // os
