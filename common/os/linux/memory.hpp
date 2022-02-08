#ifndef ASUKA_COMMON_OS_LINUX_MEMORY_HPP
#define ASUKA_COMMON_OS_LINUX_MEMORY_HPP

namespace memory {
namespace internal {

void* allocate_pages(uint64 size);
void* allocate_pages(void* base_address, uint64 size);
void  free_pages(void* memory, uint64 size);
const char* get_allocate_pages_error();

} // internal
} // memory

#ifdef UNITY_BUILD
#include "memory.cpp"
#endif

#endif // ASUKA_COMMON_OS_LINUX_MEMORY_HPP
