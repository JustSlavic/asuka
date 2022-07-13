#pragma once

//
//                              Arena Allocator
//
// Arena allocates memory in one big chunk, but at the choosen moment
// in time, it frees all its content at once. It is useful for scratchpads
// when you need to calculate something, but it does not live longer than
// the length of frame or scope of the function.
//

#include <defines.hpp>
#include <os/memory.hpp>

namespace memory
{


struct arena_allocator {
    byte *memory;
    usize size; // bytes
    usize used; // bytes
};


INLINE
void initialize(arena_allocator *allocator, void* memory, usize size)
{
    // @todo: should I align this pointer?
    allocator->memory = (byte *) memory;
    allocator->size = size;
    allocator->used = 0;
}


INLINE
void* allocate_(arena_allocator *allocator, usize requested_size, usize alignment)
{
    void *result = NULL;

    usize padding = get_padding(allocator->memory + allocator->used, alignment);
    if ((allocator->used + padding + requested_size) <= allocator->size)
    {
        result = (allocator->memory + allocator->used + padding);
        allocator->used += requested_size + padding;
    }

    return result;
}

INLINE
void deallocate(arena_allocator *allocator, void *memory_to_free)
{
    // @note: deallocate does nothing in the arena allocation strategy!
}


} // namespace memory
