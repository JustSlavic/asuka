#ifndef ASUKA_COMMON_OS_WINDOWS_FILE_HPP
#define ASUKA_COMMON_OS_WINDOWS_FILE_HPP

namespace Asuka {
namespace os {
namespace internal {

byte_array load_entire_file(const char* filename);
bool write_file(const char* filename, byte_array contents);

} // internal
} // os
} // namespace Asuka


#if UNITY_BUILD
#include "file.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_OS_WINDOWS_FILE_HPP
