#ifndef ASUKA_ALLOCATOR_HPP
#define ASUKA_ALLOCATOR_HPP

#include <defines.hpp>
#include <os/memory.hpp>

#ifdef ASUKA_MALLOCATOR
#include <memory/mallocator.hpp>
#endif // ASUKA_MALLOCATOR
#include <memory/arena_allocator.hpp>
#include <memory/pool_allocator.hpp>

/*
                    Allocators

    This module implements various allocator strategies.
    Following allocators are implemented:
      - page allocator
      - arena allocator
    Yet to be implemented
      - stack allocator
      - pool allocator
      - heap allocator (needs rewrite)

    All allocators should implement common interface, like 'traits' do.
    Functions:
        void initialize(Allocator *allocator, void *memory, usize size);
        void reset(Allocator *allocator);
        void *allocate_(Allocator *allocator, usize requested_size, usize alignment);
        void free(Allocator *allocator, void *memory_to_free);

        void *allocate<Allocator>(usize, usize);

    There are also macros to help to use those:
      - allocate_struct_(ARENA, TYPE)
      - allocate_struct(ARENA, TYPE)

      - allocate_array_(ARENA, TYPE, SIZE)
      - allocate_array(ARENA, TYPE, SIZE)


*/


namespace memory {


template <typename Allocator>
void* allocate(Allocator *allocator, usize requested_size, usize alignment) {
    void *result = allocate_(allocator, requested_size, alignment);
    if (result)
    {
        memory::set(result, 0, requested_size);
    }
    return result;
}


template <typename T, typename Allocator>
T *allocate_struct_(Allocator *allocator)
{
    T *result = (T *)allocate_(allocator, sizeof(T), alignof(T));
    return result;
}


template <typename T, typename Allocator>
T *allocate_struct(Allocator *allocator)
{
    T *result = (T *)allocate(allocator, sizeof(T), alignof(T));
    return result;
}


template <typename T, typename Allocator>
T *allocate_buffer_(Allocator *allocator, usize count)
{
    T *result = (T *)allocate_(allocator, sizeof(T) * count, alignof(T));
    return result;
}


template <typename T, typename Allocator>
T *allocate_buffer(Allocator *allocator, usize count)
{
    T *result = (T *)allocate(allocator, sizeof(T) * count, alignof(T));
    return result;
}


template <typename Allocator>
void deallocate_buffer(Allocator *allocator, void *memory)
{
    deallocate(allocator, memory);
}


} // namespace memory


#endif // ASUKA_ALLOCATOR_HPP
