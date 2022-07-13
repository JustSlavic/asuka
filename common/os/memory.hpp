#pragma once


namespace memory {

INLINE
void *align_pointer(void *pointer, usize alignment)
{
    ASSERT(alignment == 1 || alignment == 2 || alignment == 4 || alignment == 8);

    uint8 *result = (uint8 *)pointer + ((alignment - ((usize)pointer & (alignment - 1))) & (alignment - 1));
    return result;
}

INLINE
usize get_padding(void *pointer, usize alignment)
{
    ASSERT(alignment == 1 || alignment == 2 || alignment == 4 || alignment == 8);

    usize result = (alignment - ((usize)pointer & (alignment - 1))) & (alignment - 1);
    return result;
}

void *allocate_pages(u64 size);
void *allocate_pages(void *base_address, u64 size);
void  free_pages(void *memory);

void set(void *memory, u8 value, usize size);
void copy(void *destination, void *source, usize size);

} // namespace memory


#if UNITY_BUILD
#include "memory.cpp"
#endif
