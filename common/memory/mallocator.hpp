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

namespace memory
{


struct mallocator
{
    char const *name;

#if ASUKA_DEBUG
    AllocationLog log;
#endif // ASUKA_DEBUG
};


INLINE void initialize__(mallocator *allocator, void *memory, usize size, char const *name = "mallocator")
{
    allocator->name = name;
}

#if ASUKA_DEBUG
INLINE void *allocate__(mallocator *allocator, usize requested_size, usize alignment, CodeLocation cl)
#else
INLINE void *allocate__(mallocator *allocator, usize requested_size, usize alignment)
#endif
{
    void *result = malloc(requested_size);

#if ASUKA_DEBUG
    push_allocation_entry(&allocator->log, {cl, result, requested_size});
#endif // ASUKA_DEBUG

    return result;
}

#if ASUKA_DEBUG
INLINE void deallocate__(mallocator *allocator, void *memory_to_free, CodeLocation cl)
#else
INLINE void deallocate__(mallocator *allocator, void *memory_to_free)
#endif
{
#if ASUKA_DEBUG
    pop_allocation_entry(&allocator->log, memory_to_free);
#endif // ASUKA_DEBUG

    free(memory_to_free);
}


} // namespace memory
