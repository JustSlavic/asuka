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

#define ALLOCATE_(ALLOCATOR, SIZE, ALIGNMENT) \
    memory::allocate_(ALLOCATOR, SIZE, ALIGNMENT, CODE_LOCATION_FUNC)
#define ALLOCATE(ALLOCATOR, SIZE, ALIGNMENT) \
    memory::allocate(ALLOCATOR, SIZE, ALIGNMENT, CODE_LOCATION_FUNC)
#define DEALLOCATE(ALLOCATOR, POINTER) \
    memory::deallocate(ALLOCATOR, POINTER, CODE_LOCATION_FUNC)

#define ALLOCATE_STRUCT_(ALLOCATOR, TYPE) \
    memory::allocate_struct_<TYPE>(ALLOCATOR, CODE_LOCATION_FUNC)
#define ALLOCATE_STRUCT(ALLOCATOR, TYPE) \
    memory::allocate_struct<TYPE>(ALLOCATOR, CODE_LOCATION_FUNC)

#define ALLOCATE_BUFFER_(ALLOCATOR, TYPE, COUNT) \
    memory::allocate_buffer_<TYPE>(ALLOCATOR, COUNT, CODE_LOCATION_FUNC);
#define ALLOCATE_BUFFER(ALLOCATOR, TYPE, COUNT) \
    memory::allocate_buffer<TYPE>(ALLOCATOR, COUNT, CODE_LOCATION_FUNC);
#define DEALLOCATE_BUFFER(ALLOCATOR, POINTER) \
    memory::deallocate_buffer(ALLOCATOR, POINTER, CODE_LOCATION_FUNC);

template <typename Allocator>
void initialize(Allocator *allocator, void *memory, usize size, char const *name = NULL)
{
    if (name)
    {
        initialize__(allocator, memory, size, name);
    }
    else
    {
        initialize__(allocator, memory, size);
    }
    osOutputDebugString("Initialize allocator %s: %p of size %llu\n", allocator->name, memory, size);
}


template <typename Allocator>
void *allocate_(Allocator *allocator, usize requested_size, usize alignment, CodeLocation cl)
{
    void *result = allocate__(allocator, requested_size, alignment, cl);
    if (cl.function)
    {
        osOutputDebugString("Allocation (%s): %p of size %llu at %s:%d (%s)\n", allocator->name, result, requested_size, cl.filename, cl.line, cl.function);
    }
    else
    {
        osOutputDebugString("Allocation: %p of size %llu at %s:%d\n", result, requested_size, cl.filename, cl.line);
    }
    return result;
}


template <typename Allocator>
void *allocate(Allocator *allocator, usize requested_size, usize alignment, CodeLocation cl)
{
    void *result = allocate_(allocator, requested_size, alignment, cl);
    if (result)
    {
        memory::set(result, 0, requested_size);
    }
    return result;
}


template <typename Allocator>
void deallocate(Allocator *allocator, void *memory_to_free, CodeLocation cl)
{
    osOutputDebugString("Deallocaion: %p\n", memory_to_free);
    deallocate__(allocator, memory_to_free, cl);
}


template <typename T, typename Allocator>
T *allocate_struct_(Allocator *allocator, CodeLocation cl)
{
    T *result = (T *)allocate_(allocator, sizeof(T), alignof(T), cl);
    return result;
}


template <typename T, typename Allocator>
T *allocate_struct(Allocator *allocator, CodeLocation cl)
{
    T *result = (T *)allocate(allocator, sizeof(T), alignof(T), cl);
    return result;
}


template <typename T, typename Allocator>
T *allocate_buffer_(Allocator *allocator, usize count, CodeLocation cl)
{
    T *result = (T *)allocate_(allocator, sizeof(T) * count, alignof(T), cl);
    return result;
}


template <typename T, typename Allocator>
T *allocate_buffer(Allocator *allocator, usize count, CodeLocation cl)
{
    T *result = (T *)allocate(allocator, sizeof(T) * count, alignof(T), cl);
    return result;
}


template <typename Allocator>
void deallocate_buffer(Allocator *allocator, void *memory, CodeLocation cl)
{
    deallocate(allocator, memory, cl);
}


} // namespace memory


#endif // ASUKA_ALLOCATOR_HPP
