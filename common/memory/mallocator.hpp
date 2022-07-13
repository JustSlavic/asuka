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
{};


GLOBAL mallocator mallocator_instance;


INLINE
void initialize(mallocator *allocator, void *memory, usize size)
{
}


INLINE
void *allocate_(mallocator *allocator, usize requested_size, usize alignment)
{
    return malloc(requested_size);
}


INLINE
void deallocate(mallocator *allocator, void *memory_to_free)
{
    free(memory_to_free);
}


} // namespace memory
