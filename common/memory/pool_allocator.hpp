#pragma once

//
//                              Pool Allocator
//
// Pool allocator divides memory into equally sized chunks. All chunks are
// preceded with the 64-bit header, which keeps a pointer to the next free
// memory chunk.
//
//                           ┌───────────────────────┐
//                           │                       ↓
//  ┌────────┬──────────────┬────────┬──────────────┬────────┬─────
//  │ 64 bit │ memory chunk │ 64 bit │ memory chunk │ 64 bit │ ...
//  └────────┴──────────────┴────────┴──────────────┴────────┴─────────
//   ↑                       ↑                       │
//   memory                  free_list               └─────→
//

#include <defines.hpp>
#include <os/memory.hpp>

namespace memory
{


template <usize ChunkSize>
struct pool_allocator
{
    struct memory_chunk
    {
        union
        {
            struct {
                memory_chunk *next_chunk;
                byte padding[ChunkSize - sizeof(memory_chunk *)];
            };
            byte memory[ChunkSize];
        };
    };

    static_assert(ChunkSize > sizeof(memory_chunk *));

    byte *memory;
    usize size; // bytes
    usize used; // bytes

    memory_chunk *free_list;

#ifdef ASUKA_DEBUG
    char const *name;
    AllocationLogEntry hash_table[1024];
    usize allocation_count = 0;
#endif // ASUKA_DEBUG
};


#if ASUKA_DEBUG

template <usize ChunkSize>
AllocationLogEntry *get_allocation_entry(pool_allocator<ChunkSize> *allocator, void *pointer)
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

template <usize ChunkSize>
void push_allocation_entry(pool_allocator<ChunkSize> *allocator, AllocationLogEntry entry)
{
    AllocationLogEntry *hash_slot = get_allocation_entry(allocator, entry.pointer);
    *hash_slot = entry;
}

template <usize ChunkSize>
void pop_allocation_entry(pool_allocator<ChunkSize> *allocator, void *pointer)
{
    AllocationLogEntry *hash_slot = get_allocation_entry(allocator, pointer);
    *hash_slot = null_allocation_entry();
}

#endif // ASUKA_DEBUG


template <usize ChunkSize>
#ifdef ASUKA_DEBUG
void initialize__(pool_allocator<ChunkSize> *allocator, void *memory, usize size, char const *name = "pool")
#else // ASUKA_DEBUG
void initialize__(pool_allocator<ChunkSize> *allocator, void *memory, usize size)
#endif // ASUKA_DEBUG
{
    using chunk_t = pool_allocator<ChunkSize>::memory_chunk;

    auto aligned = get_aligned_pointer(memory, 8); // @todo: what is MAX_ALIGN should be?

    allocator->memory = aligned.pointer;
    allocator->size   = size - aligned.padding;
    allocator->used   = 0;

    chunk_t *first_header = (chunk_t *) allocator->memory;

    chunk_t *header = first_header;
    for (usize chunk_index = 1; chunk_index < (size / sizeof(chunk_t)); chunk_index++)
    {
        chunk_t *next_chunk = header + 1;
        header->next_chunk  = next_chunk;

#if ASUKA_ASAN
        ASAN_POISON_MEMORY_REGION(header, sizeof(chunk_t));
#endif // ASUKA_ASAN

        header = next_chunk;
    }

    header->next_chunk = 0;
    allocator->free_list = first_header;

#if ASUKA_DEBUG
    allocator->name = name;
#endif // ASUKA_DEBUG
}

template <usize ChunkSize>
#if ASUKA_DEBUG
void *allocate__(pool_allocator<ChunkSize> *allocator, usize requested_size, usize alignment, CodeLocation cl)
#else
void *allocate__(pool_allocator<ChunkSize> *allocator, usize requested_size, usize alignment)
#endif // ASUKA_DEBUG
{
    using chunk_t = pool_allocator<ChunkSize>::memory_chunk;

    byte *result = NULL;
    if (requested_size <= ChunkSize)
    {
        if (allocator->free_list)
        {
            result = allocator->free_list->memory;
            allocator->free_list = allocator->free_list->next_chunk;

            allocator->used += sizeof(chunk_t);

#if ASUKA_ASAN
            ASAN_UNPOISON_MEMORY_REGION(result, ChunkSize);
#endif // ASUKA_ASAN
        }
    }

#if ASUKA_DEBUG
    push_allocation_entry(allocator, {cl, result, requested_size});
#endif // ASUKA_DEBUG
    return result;
}

template <usize ChunkSize>
#if ASUKA_DEBUG
void deallocate__(pool_allocator<ChunkSize> *allocator, void *memory_to_free, CodeLocation cl)
#else
void deallocate__(pool_allocator<ChunkSize> *allocator, void *memory_to_free)
#endif
{
    using chunk_t = pool_allocator<ChunkSize>::memory_chunk;

    chunk_t *chunk = (chunk_t *) memory_to_free;
    chunk->next_chunk = allocator->free_list;
    allocator->free_list = chunk;

    allocator->used -= sizeof(chunk_t);

#if ASUKA_ASAN
    // ASAN_POISON_MEMORY_REGION(header->padding, sizeof(header->padding));
#endif // ASUKA_ASAN

#if ASUKA_DEBUG
    pop_allocation_entry(allocator, memory_to_free);
    allocator->allocation_count -= 1;
#endif // ASUKA_DEBUG
}


} // namespace memory
