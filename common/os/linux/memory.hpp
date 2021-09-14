#ifndef ASUKA_COMMON_OS_LINUX_MEMORY_HPP
#define ASUKA_COMMON_OS_LINUX_MEMORY_HPP

namespace os {
namespace internal {

void* allocate_pages(uint64 size);
void* allocate_pages(void* base_address, uint64 size);
void  free_pages(void* memory, uint64 size);
const char* get_allocate_pages_error();

} // internal

inline void* allocate_pages(uint64 size) {
    return internal::allocate_pages(size);
}

inline void* allocate_pages(void* base_address, uint64 size) {
    return internal::allocate_pages(base_address, size);
}

inline void free_pages(void* memory, uint64 size) {
    return internal::free_pages(memory, size);
}

inline const char* get_allocate_pages_error() {
    return internal::get_allocate_pages_error();
}

} // os

#ifdef UNITY_BUILD
#include "memory.cpp"
#endif

#endif // ASUKA_COMMON_OS_LINUX_MEMORY_HPP
