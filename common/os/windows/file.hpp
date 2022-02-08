#ifndef ASUKA_COMMON_OS_WINDOWS_FILE_HPP
#define ASUKA_COMMON_OS_WINDOWS_FILE_HPP


namespace os {
namespace internal {

string load_entire_file(const char* filename);
bool write_file(const char* filename, string file);

} // windows
} // os


#if UNITY_BUILD
#include "file.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_OS_WINDOWS_FILE_HPP
