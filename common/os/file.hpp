#ifndef ASUKA_COMMON_OS_FILE_HPP
#define ASUKA_COMMON_OS_FILE_HPP

#include <array.hpp>
#include <tprint.hpp>

namespace os {

// @todo: Reallocation of the buffer
// @todo: operator / with the OS-specific separator

struct filepath
{
    string buffer;

    STATIC
    filepath from(char const *name)
    {
        filepath result;
        result.buffer = string::from(name);
        return result;
    }

    STATIC
    filepath from(string name)
    {
        filepath result;
        result.buffer = name;
        return result;
    }
};

byte_array load_entire_file(const char* filepath);
bool write_file(const char* filepath, byte_array contents);

} // os

template <>
void tprint_helper<os::filepath>(os::filepath p)
{
    printf("%.*s", (int) p.buffer.size, p.buffer.data);
}

struct filepath_iterator
{
    // @todo: OS-defined implementation?
};


#if UNITY_BUILD
#include "file.cpp"
#endif

#endif // ASUKA_COMMON_OS_FILE_HPP
