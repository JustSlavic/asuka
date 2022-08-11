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

#if ASUKA_DEBUG
    char const *name;
    AllocationLogEntry hash_table[1024];
    usize allocation_count = 0;
#endif // ASUKA_DEBUG
};


#if ASUKA_DEBUG

INLINE
AllocationLogEntry *get_allocation_entry(arena_allocator *allocator, void *pointer)
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
void push_allocation_entry(arena_allocator *allocator, AllocationLogEntry entry)
{
    AllocationLogEntry *hash_slot = get_allocation_entry(allocator, entry.pointer);
    *hash_slot = entry;
}


INLINE
void pop_allocation_entry(arena_allocator *allocator, void *pointer)
{
    AllocationLogEntry *hash_slot = get_allocation_entry(allocator, pointer);
    *hash_slot = null_allocation_entry();
}

#endif // ASUKA_DEBUG


INLINE
#if ASUKA_DEBUG
void initialize__(arena_allocator *allocator, void* memory, usize size, char const* name = "arena")
#else // ASUKA_DEBUG
void initialize__(arena_allocator *allocator, void* memory, usize size)
#endif // ASUKA_DEBUG
{
    auto aligned = get_aligned_pointer(memory, 8); // @todo: what is MAX_ALIGN should be?

    allocator->memory = aligned.pointer;
    allocator->size = size - aligned.padding;
    allocator->used = 0;

#if ASUKA_DEBUG
    allocator->name = name;
    set(allocator->hash_table, 0, sizeof(allocator->hash_table));
    allocator->allocation_count = 0;
#endif // ASUKA_DEBUG

#if ASUKA_ASAN
    ASAN_POISON_MEMORY_REGION(memory, size);
#endif // ASUKA_ASAN
}


INLINE
#if ASUKA_DEBUG
void* allocate__(arena_allocator *allocator, usize requested_size, usize alignment, CodeLocation cl)
#else // ASUKA_DEBUG
void* allocate__(arena_allocator *allocator, usize requested_size, usize alignment)
#endif // ASUKA_DEBUG
{
    void *result = NULL;

    usize padding = get_padding(allocator->memory + allocator->used, alignment);
    if ((allocator->used + padding + requested_size) <= allocator->size)
    {
        result = (allocator->memory + allocator->used + padding);
        allocator->used += requested_size + padding;

#if ASUKA_DEBUG
        allocator->allocation_count += 1;
#endif // ASUKA_DEBUG

#if ASUKA_ASAN
        ASAN_UNPOISON_MEMORY_REGION(result, requested_size + padding);
#endif // ASUKA_ASAN
    }

#if ASUKA_DEBUG
    push_allocation_entry(allocator, {cl, result, requested_size});
#endif // ASUKA_DEBUG
    return result;
}

INLINE
#if ASUKA_DEBUG
void deallocate__(arena_allocator *allocator, void *memory_to_free, CodeLocation cl)
#else // ASUKA_DEBUG
void deallocate__(arena_allocator *allocator, void *memory_to_free)
#endif // ASUKA_DEBUG
{
    // @note: deallocate does nothing in the arena allocation strategy!
#if ASUKA_DEBUG
    pop_allocation_entry(allocator, memory_to_free);
    allocator->allocation_count -= 1;
#endif // ASUKA_DEBUG
}


} // namespace memory
