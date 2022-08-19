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
    usize last_allocation_size; // @note: saved last allocation size for reallocation capability

    char const *name;

#if ASUKA_DEBUG
    AllocationLog log;
#endif // ASUKA_DEBUG
};


INLINE
void initialize__(arena_allocator *allocator, void* memory, usize size, char const* name = "arena")
{
    allocator->memory = (byte *) memory;
    allocator->size = size;
    allocator->used = 0;
    allocator->last_allocation_size = 0;

    allocator->name = name;

#if ASUKA_DEBUG
    initialize_allocation_log(&allocator->log);
#endif // ASUKA_DEBUG
}


INLINE
void* allocate__(arena_allocator *allocator, usize requested_size, usize alignment, CodeLocation cl)
{
    byte *result = NULL;

    usize padding = get_padding(allocator->memory + allocator->used, alignment);
    if ((allocator->used + padding + requested_size) <= allocator->size)
    {
        result = (allocator->memory + allocator->used + padding);
        allocator->used += requested_size + padding;
        allocator->last_allocation_size = requested_size;

#if ASUKA_DEBUG
        push_allocation_entry(&allocator->log, {cl, result, requested_size});
#endif // ASUKA_DEBUG
    }

    return result;
}


INLINE
void deallocate__(arena_allocator *allocator, void *memory_to_free, CodeLocation cl)
{
    // @note: deallocate does nothing in the arena allocator!
#if ASUKA_DEBUG
    pop_allocation_entry(&allocator->log, memory_to_free);
#endif // ASUKA_DEBUG
}


INLINE
void *reallocate__(arena_allocator *allocator, void *pointer, usize new_size, usize alignment, CodeLocation cl)
{
    void *result = NULL;

    usize last_allocation_size = allocator->last_allocation_size;
    byte *last_allocation = allocator->memory + allocator->used - last_allocation_size;

    if (pointer == last_allocation)
    {
        result = pointer;

        if (new_size > last_allocation_size)
        {
            allocator->used += (new_size - last_allocation_size);
            allocator->last_allocation_size = new_size;
        }
    }
    else
    {
        result = allocate__(allocator, new_size, alignment, cl);
        memory::copy(result, pointer, last_allocation_size);
        deallocate__(allocator, pointer, cl);
    }

    return result;
}


} // namespace memory
