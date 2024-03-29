#include "file.hpp"

// @todo: get rid of windows.h here
#include <windows.h>

#include <defines.hpp>

namespace os {
namespace internal {


byte_array load_entire_file(const char* filename)
{
    byte_array result = {};

    HANDLE FileHandle = CreateFileA(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        // @todo: log error
        return result;
    }

    defer { CloseHandle(FileHandle); };

    LARGE_INTEGER FileSize;
    BOOL GetSizeResult = GetFileSizeEx(FileHandle, &FileSize);
    if (GetSizeResult == 0)
    {
        // @todo: log error
        return result;
    }

    ASSERT(FileSize.QuadPart <= UINT32_MAX);

    void* Memory = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    DWORD BytesRead;
    BOOL ReadFileResult = ReadFile(FileHandle, Memory, (DWORD)FileSize.QuadPart, &BytesRead, NULL);

    if (ReadFileResult == FALSE && BytesRead == FileSize.QuadPart)
    {
        // @todo: log error
        VirtualFree(Memory, 0, MEM_RELEASE);
        return result;
    }

    result.data = (memory::byte *) Memory;
    result.size = FileSize.QuadPart;
    result.capacity = FileSize.QuadPart;
    return result;
}


bool write_file(const char* filename, byte_array file)
{
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

    return file.get_size() == BytesWritten;
}


} // internal
} // os
