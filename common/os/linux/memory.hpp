#pragma once

#include <defines.hpp>

namespace memory {
namespace internal {


void *allocate_pages(usize size);
void *allocate_pages(void *base_address, usize size);
void  free_pages(void *memory, usize size);


} // namespace internal
} // namespace memory


#if UNITY_BUILD
#include "memory.cpp"
#endif
