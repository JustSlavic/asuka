#ifndef ASUKA_COMMON_OS_WINDOWS_FILE_HPP
#define ASUKA_COMMON_OS_WINDOWS_FILE_HPP


namespace os {
namespace windows {

file_read_result load_entire_file(const char* filename);
bool write_file(const char* filename, file_read_result file);

} // windows

file_read_result load_entire_file(const char* filename) {
    return windows::load_entire_file(filename);
}

bool write_file(const char* filename, file_read_result file) {
    return windows::write_file(filename, file);
}

} // os


#ifdef UNITY_BUILD
#include "file.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_OS_WINDOWS_FILE_HPP
