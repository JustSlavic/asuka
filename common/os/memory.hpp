#pragma once

#include <byte.hpp>

namespace memory {

INLINE
usize get_padding(void *pointer, usize alignment)
{
    usize result = (alignment - ((usize)pointer & (alignment - 1))) & (alignment - 1);
    return result;
}


INLINE
void *align_pointer(void *pointer, usize alignment)
{
    byte *result = (byte *)pointer + get_padding(pointer, alignment);
    return result;
}


INLINE
void *align_pointer(void *pointer, usize alignment, usize *padding)
{
    ASSERT_MSG(padding, "Padding argument is not optional, use 'void *align_pointer(void *pointer, usize alignment)' insted.\n");

    *padding = get_padding(pointer, alignment);
    uint8 *result = (uint8 *)pointer + *padding;
    return result;
}


struct AlignPointerResult
{
    byte *pointer;
    usize padding;
};

INLINE
AlignPointerResult get_aligned_pointer(void *pointer, usize alignment)
{
    AlignPointerResult result;

    result.padding = get_padding(pointer, alignment);
    result.pointer = (byte *)pointer + result.padding;
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
