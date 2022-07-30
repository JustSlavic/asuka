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
#if ASUKA_DEBUG
    char const *name;
    AllocationLogEntry hash_table[2048];
    usize allocation_count = 0;
#endif // ASUKA_DEBUG
};

#if ASUKA_DEBUG
GLOBAL mallocator global_mallocator_instance = {"malloc"};
#else
GLOBAL mallocator global_mallocator_instance = {};
#endif

#if ASUKA_DEBUG

INLINE
AllocationLogEntry *get_allocation_entry(mallocator *allocator, void *pointer)
{
    AllocationLogEntry *result = NULL;

    uint64 hash = (uint64) pointer;
    for (uint64 offset = 0; offset < ARRAY_COUNT(allocator->hash_table); offset++)
    {
        uint64 index = (hash + offset) % ARRAY_COUNT(allocator->hash_table);
        AllocationLogEntry *entry = allocator->hash_table + index;
        if (entry->pointer == pointer || entry->pointer == NULL)
        {
            result = entry;
            break;
        }
    }
    return result;
}

INLINE
void push_allocation_entry(mallocator *allocator, AllocationLogEntry entry)
{
    ASSERT(allocator->allocation_count < ARRAY_COUNT(allocator->hash_table));

    AllocationLogEntry *hash_slot = NULL;
    uint64 hash = (uint64) entry.pointer;
    for (uint64 offset = 0; offset < ARRAY_COUNT(allocator->hash_table); offset++)
    {
        uint64 index = (hash + offset) % ARRAY_COUNT(allocator->hash_table);
        AllocationLogEntry *slot = allocator->hash_table + index;
        if (slot->pointer == NULL)
        {
            hash_slot = slot;
            break;
        }
    }

    ASSERT(hash_slot);
    *hash_slot = entry;
}


INLINE
void pop_allocation_entry(mallocator *allocator, void *pointer)
{
    AllocationLogEntry *hash_slot = NULL;
    uint64 hash = (uint64) pointer;
    for (uint64 offset = 0; offset < ARRAY_COUNT(allocator->hash_table); offset++)
    {
        uint64 index = (hash + offset) % ARRAY_COUNT(allocator->hash_table);
        AllocationLogEntry *entry = allocator->hash_table + index;
        if (entry->pointer == pointer)
        {
            hash_slot = entry;
            break;
        }
    }

    ASSERT(hash_slot->pointer);
    *hash_slot = null_allocation_entry();
}

#endif // ASUKA_DEBUG

#if ASUKA_DEBUG
INLINE void initialize__(mallocator *allocator, void *memory, usize size, char const *name = "mallocator")
#else
INLINE void initialize__(mallocator *allocator, void *memory, usize size)
#endif // ASUKA_DEBUG
{
#if ASUKA_DEBUG
    allocator->name = name;
    set(allocator->hash_table, 0, sizeof(allocator->hash_table));
    allocator->allocation_count = 0;
#endif // ASUKA_DEBUG
}

#if ASUKA_DEBUG
INLINE void *allocate__(mallocator *allocator, usize requested_size, usize alignment, CodeLocation cl)
#else
INLINE void *allocate__(mallocator *allocator, usize requested_size, usize alignment)
#endif
{
    void *result = malloc(requested_size);

#if ASUKA_DEBUG
    push_allocation_entry(allocator, {cl, result, requested_size});
    allocator->allocation_count += 1;
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
    pop_allocation_entry(allocator, memory_to_free);
    allocator->allocation_count -= 1;
#endif // ASUKA_DEBUG

    free(memory_to_free);
}


} // namespace memory
