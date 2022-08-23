#ifndef ASUKA_COMMON_OS_FILE_HPP
#define ASUKA_COMMON_OS_FILE_HPP

#include <array.hpp>
#include <tprint.hpp>

namespace os {

// @todo: Reallocation of the buffer
// @todo: operator / with the OS-specific separator

byte_array load_entire_file(const char* filepath);
bool write_file(const char* filepath, byte_array contents);

} // os

#if UNITY_BUILD
#include "file.cpp"
#endif

#endif // ASUKA_COMMON_OS_FILE_HPP
