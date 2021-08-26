#include "file.hpp"
#include <windows.h>


namespace os {
namespace windows {


file_read_result load_entire_file(const char* filename) {
    file_read_result result {};

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
    BOOL ReadFileResult = ReadFile(FileHandle, Memory, FileSize.QuadPart, &BytesRead, NULL);

    CloseHandle(FileHandle);

    if (ReadFileResult == FALSE && BytesRead == FileSize.QuadPart) {
        // @todo: log error
        VirtualFree(Memory, 0, MEM_RELEASE);
        return result;
    }

    result.memory = Memory;
    result.size = FileSize.QuadPart;
    return result;
}


bool write_file(const char* filename, file_read_result file) {
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

    DWORD BytesWritten;
    BOOL WriteFile(FileHandle, file.memory, file.size, &BytesWritten, NULL);

    CloseHandle(FileHandle);

    return file.size == BytesWritten;
}


} // windows
} // os
