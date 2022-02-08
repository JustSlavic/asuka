#ifndef ASUKA_COMMON_OS_LINUX_FILE_HPP
#define ASUKA_COMMON_OS_LINUX_FILE_HPP

#include <string.hpp>


namespace os {
namespace internal {

string load_entire_file(const char* filename);
bool write_file(const char* filename, string file);

} // internal
} // os


#if UNITY_BUILD
#include "file.cpp"
#endif

#endif // ASUKA_COMMON_OS_LINUX_FILE_HPP
