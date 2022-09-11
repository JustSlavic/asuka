#pragma once

//
//                             Mallocator
//
// Mallocator is just a proxy memory allocator, which you can use in the
// functions and template structures that use them. For example,
// it is the default allocator for dynamic_array, because it requires
// reallocation when buffer is overflown.
//

#include <defines.hpp>

namespace memory {


struct mallocator
{
    char const *name;

#if ASUKA_DEBUG
    AllocationLog log;
#endif // ASUKA_DEBUG
};


INLINE
void initialize__(mallocator *allocator, void *memory, usize size, char const *name = "mallocator")
{
    allocator->name = name;
}


INLINE
void *allocate__(mallocator *allocator, usize size, usize alignment, CodeLocation cl)
{
    void *result = malloc(size);

#if ASUKA_DEBUG
    push_allocation_entry(&allocator->log, {cl, result, size});
#endif // ASUKA_DEBUG

    return result;
}


INLINE
void deallocate__(mallocator *allocator, void *memory_to_free, CodeLocation cl)
{
    free(memory_to_free);

#if ASUKA_DEBUG
    pop_allocation_entry(&allocator->log, memory_to_free);
#endif // ASUKA_DEBUG
}


INLINE
void *reallocate__(mallocator *allocator, void *pointer, usize new_size, usize alignment, CodeLocation cl)
{
    void *result = realloc(pointer, new_size);

#if ASUKA_DEBUG
    if (result != pointer)
    {
        pop_allocation_entry(&allocator->log, pointer);
        push_allocation_entry(&allocator->log, {cl, result, new_size});
    }
#endif // ASUKA_DEBUG

    return result;
}


} // namespace memory
