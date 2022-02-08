#ifndef ASUKA_COMMON_OS_FILE_HPP
#define ASUKA_COMMON_OS_FILE_HPP

#include <string.hpp>


namespace os {

string load_entire_file(const char* filename);
bool write_file(const char* filename, string contents);

} // os


#if UNITY_BUILD
#include "file.cpp"
#endif

#if ASUKA_OS_LINUX
#include "linux/file.hpp"
#endif

#endif // ASUKA_COMMON_OS_FILE_HPP
