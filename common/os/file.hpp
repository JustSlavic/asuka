#ifndef ASUKA_COMMON_OS_FILE_HPP
#define ASUKA_COMMON_OS_FILE_HPP


namespace os {

struct file_read_result {
    uint8* memory;
    uint64 size;
};

file_read_result load_entire_file(const char* filename);
bool write_file(const char* filename, file_read_result file);

} // os


#if ASUKA_OS_WINDOWS
#include "windows/file.hpp"
#endif

#endif // ASUKA_COMMON_OS_FILE_HPP
