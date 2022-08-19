#pragma once

#include <defines.hpp>
#include <os/memory.hpp>
#include <code_location.hpp>


#ifdef ASUKA_DEBUG
struct AllocationLogEntry
{
    CodeLocation cl;
    void *pointer;
    usize size;
    usize index;
};

AllocationLogEntry null_allocation_entry()
{
    AllocationLogEntry result = {};
    return result;
}

struct AllocationLog
{
    AllocationLogEntry hash_table[2048];
    usize allocation_count = 0;
};

INLINE
void initialize_allocation_log(AllocationLog *log)
{
    memory::set(log->hash_table, 0, sizeof(log->hash_table));
    log->allocation_count = 0;
}

INLINE
AllocationLogEntry *get_allocation_entry(AllocationLog *log, void *pointer)
{
    AllocationLogEntry *result = NULL;

    uint64 hash = (uint64) pointer;
    for (uint64 offset = 0; offset < ARRAY_COUNT(log->hash_table); offset++)
    {
        uint64 index = (hash + offset) % ARRAY_COUNT(log->hash_table);
        AllocationLogEntry *entry = log->hash_table + index;
        if (entry->pointer == pointer || entry->pointer == NULL)
        {
            result = entry;
            break;
        }
    }
    return result;
}

INLINE
void push_allocation_entry(AllocationLog *log, AllocationLogEntry entry)
{
    ASSERT(log->allocation_count < ARRAY_COUNT(log->hash_table));

    AllocationLogEntry *hash_slot = NULL;
    uint64 hash = (uint64) entry.pointer;
    for (uint64 offset = 0; offset < ARRAY_COUNT(log->hash_table); offset++)
    {
        uint64 index = (hash + offset) % ARRAY_COUNT(log->hash_table);
        AllocationLogEntry *slot = log->hash_table + index;
        if (slot->pointer == NULL)
        {
            hash_slot = slot;
            log->allocation_count += 1;
            break;
        }
    }

    ASSERT(hash_slot);
    *hash_slot = entry;
}

INLINE
void pop_allocation_entry(AllocationLog *log, void *pointer)
{
    AllocationLogEntry *hash_slot = NULL;
    uint64 hash = (uint64) pointer;
    for (uint64 offset = 0; offset < ARRAY_COUNT(log->hash_table); offset++)
    {
        uint64 index = (hash + offset) % ARRAY_COUNT(log->hash_table);
        AllocationLogEntry *entry = log->hash_table + index;
        if (entry->pointer == pointer)
        {
            hash_slot = entry;
            log->allocation_count -= 1;
            break;
        }
    }

    ASSERT(hash_slot->pointer);
    *hash_slot = null_allocation_entry();
}
#endif // ASUKA_DEBUG


#include <memory/arena_allocator.hpp>
#include <memory/pool_allocator.hpp>
#include <memory/mallocator.hpp>


/*
                                 Allocators

    This module implements various allocator strategies.
    Following allocators are implemented:
      - arena (linear) allocator
      - pool allocator
      - mallocator
    Yet to be implemented
      - stack allocator
      - heap allocator (needs rewrite)

    All allocators should provide functions to use withing this file:

      - initialize__ gives memory to the allocator and initializes it
      - allocate__ allocates memory of given size withing the initialized memory buffer
      - reallocate__ extends memory chunk if it's possible, but reallocates otherwise
      - deallocate__ frees allocated memory

    You should not call these functions directly, but rather use macroses, to
    ensure that all meta-information about location of the call in the source
    file is stored within the allocation to later debug capabilities.
*/


#ifdef ASUKA_ASAN
extern "C" void __asan_poison_memory_region(void *, size_t);
extern "C" void __asan_unpoison_memory_region(void *, size_t);
#endif // ASUKA_ASAN


namespace memory {


#define ALLOCATE_(ALLOCATOR, SIZE, ALIGNMENT) \
    memory::allocate_(ALLOCATOR, SIZE, ALIGNMENT, CODE_LOCATION_FUNC)
#define ALLOCATE(ALLOCATOR, SIZE, ALIGNMENT) \
    memory::allocate(ALLOCATOR, SIZE, ALIGNMENT, CODE_LOCATION_FUNC)
#define DEALLOCATE(ALLOCATOR, POINTER) \
    memory::deallocate(ALLOCATOR, POINTER, CODE_LOCATION_FUNC)
#define REALLOCATE(ALLOCATOR, POINTER, NEW_SIZE, ALIGNMENT) \
    memory::reallocate(ALLOCATOR, POINTER, NEW_SIZE, CODE_LOCATION_FUNC)

#define ALLOCATE_STRUCT_(ALLOCATOR, TYPE) \
    memory::allocate_struct_<TYPE>(ALLOCATOR, CODE_LOCATION_FUNC)
#define ALLOCATE_STRUCT(ALLOCATOR, TYPE) \
    memory::allocate_struct<TYPE>(ALLOCATOR, CODE_LOCATION_FUNC)

#define ALLOCATE_BUFFER_(ALLOCATOR, TYPE, COUNT) \
    memory::allocate_buffer_<TYPE>(ALLOCATOR, COUNT, CODE_LOCATION_FUNC)
#define ALLOCATE_BUFFER(ALLOCATOR, TYPE, COUNT) \
    memory::allocate_buffer<TYPE>(ALLOCATOR, COUNT, CODE_LOCATION_FUNC)
#define DEALLOCATE_BUFFER(ALLOCATOR, POINTER) \
    memory::deallocate_buffer(ALLOCATOR, POINTER, CODE_LOCATION_FUNC)
#define REALLOCATE_BUFFER(ALLOCATOR, POINTER, NEW_COUNT) \
    memory::reallocate_buffer(ALLOCATOR, POINTER, NEW_COUNT, CODE_LOCATION_FUNC)


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
}


template <typename Allocator>
void *allocate_(Allocator *allocator, usize requested_size, usize alignment, CodeLocation cl)
{
    void *result = allocate__(allocator, requested_size, alignment, cl);
    return result;
}


template <typename Allocator>
void *allocate(Allocator *allocator, usize requested_size, usize alignment, CodeLocation cl)
{
    void *result = allocate__(allocator, requested_size, alignment, cl);
    if (result)
    {
        memory::set(result, 0, requested_size);
    }
    return result;
}


template <typename Allocator>
void *reallocate(Allocator *allocator, void *pointer, usize new_size, usize alignment, CodeLocation cl)
{
    void *result = reallocate__(allocator, pointer, new_size, alignment, cl);
    return result;
}


template <typename Allocator>
void deallocate(Allocator *allocator, void *memory_to_free, CodeLocation cl)
{
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


template <typename T, typename Allocator>
T *reallocate_buffer(Allocator *allocator, T *pointer, usize new_count, CodeLocation cl)
{
    T *result = (T *)reallocate(allocator, pointer, sizeof(T) * new_count, alignof(T), cl);
    return result;
}


template <typename Allocator>
void deallocate_buffer(Allocator *allocator, void *memory, CodeLocation cl)
{
    deallocate(allocator, memory, cl);
}


} // namespace memory
