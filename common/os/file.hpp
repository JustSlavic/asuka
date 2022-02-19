#ifndef ASUKA_COMMON_OS_FILE_HPP
#define ASUKA_COMMON_OS_FILE_HPP

#include <string.hpp>


namespace os {

using asuka::string;

string load_entire_file(const char* filename);
bool write_file(const char* filename, string contents);

} // os


#if UNITY_BUILD
#include "file.cpp"
#endif

#endif // ASUKA_COMMON_OS_FILE_HPP
