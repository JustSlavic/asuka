#ifndef ASUKA_COMMON_OS_FILE_HPP
#define ASUKA_COMMON_OS_FILE_HPP

#include <array.hpp>

namespace Asuka {
namespace os {

byte_array load_entire_file(const char* filename);
bool write_file(const char* filename, byte_array contents);

} // os
} // namespace Asuka


#if UNITY_BUILD
#include "file.cpp"
#endif

#endif // ASUKA_COMMON_OS_FILE_HPP
