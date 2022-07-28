#pragma once

namespace os {
namespace internal {

byte_array load_entire_file(const char* filename);
bool write_file(const char* filename, byte_array contents);

} // internal
} // os


#if UNITY_BUILD
#include "file.cpp"
#endif // UNITY_BUILD
