#ifndef ASUKA_COMMON_OS_MEMORY_HPP
#define ASUKA_COMMON_OS_MEMORY_HPP

namespace os {

void* allocate_pages(uint64 size);
void* allocate_pages(void* base_address, uint64 size);
void  free_pages(void* memory, uint64 size);
const char* get_allocate_pages_error();

} // os

#if ASUKA_OS_LINUX
#include "linux/memory.hpp"
#endif

#endif // ASUKA_COMMON_OS_MEMORY_HPP
