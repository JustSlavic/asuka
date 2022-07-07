#pragma once

namespace memory {
namespace internal {


void *allocate_pages(u64 size);
void *allocate_pages(void *base_address, u64 size);
void  free_pages(void *memory, u64 size);


} // namespace internal
} // namespace memory


#if UNITY_BUILD
#include "memory.cpp"
#endif
