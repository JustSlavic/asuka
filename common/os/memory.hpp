#pragma once


namespace memory {


void *allocate_pages(u64 size);
void *allocate_pages(void *base_address, u64 size);
void  free_pages(void *memory);

void set(void *memory, u8 value, usize size);
void copy(void *destination, void *source, usize size);


} // namespace memory


#if UNITY_BUILD
#include "memory.cpp"
#endif
