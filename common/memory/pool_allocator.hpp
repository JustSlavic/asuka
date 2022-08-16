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
    AllocationLog log;
#endif // ASUKA_DEBUG
};


template <usize ChunkSize>
#ifdef ASUKA_DEBUG
void initialize__(pool_allocator<ChunkSize> *allocator, void *memory, usize size, char const *name = "pool")
#else // ASUKA_DEBUG
void initialize__(pool_allocator<ChunkSize> *allocator, void *memory, usize size)
#endif // ASUKA_DEBUG
{
    ASSERT_MSG(memory, "Initializing allocator with NULL!\n");

    using chunk_t = typename pool_allocator<ChunkSize>::memory_chunk;

    allocator->memory = (byte *) memory;
    allocator->size   = size;
    allocator->used   = 0;

    auto aligned = get_aligned_pointer(memory, 8); // @todo: What is MAX_ALIGN should be?
    chunk_t *first_header = (chunk_t *) aligned.pointer;
    usize chunk_count = (size - aligned.padding) / sizeof(chunk_t);

    chunk_t *header = first_header;
    for (usize chunk_index = 1; chunk_index < chunk_count; chunk_index++)
    {
        chunk_t *next_chunk = header + 1;
        header->next_chunk  = next_chunk;

// #if ASUKA_ASAN
//         ASAN_POISON_MEMORY_REGION(header, sizeof(chunk_t));
// #endif // ASUKA_ASAN

        header = next_chunk;
    }

    header->next_chunk = 0;
    allocator->free_list = first_header;

#if ASUKA_DEBUG
    allocator->name = name;
    initialize_allocation_log(&allocator->log);
#endif // ASUKA_DEBUG
}

template <usize ChunkSize>
#if ASUKA_DEBUG
void *allocate__(pool_allocator<ChunkSize> *allocator, usize requested_size, usize alignment, CodeLocation cl)
#else
void *allocate__(pool_allocator<ChunkSize> *allocator, usize requested_size, usize alignment)
#endif // ASUKA_DEBUG
{
    using chunk_t = typename pool_allocator<ChunkSize>::memory_chunk;

    byte *result = NULL;
    if (requested_size <= ChunkSize)
    {
        if (allocator->free_list)
        {
            result = allocator->free_list->memory;
            allocator->free_list = allocator->free_list->next_chunk;

            allocator->used += sizeof(chunk_t);

#if ASUKA_DEBUG
            push_allocation_entry(&allocator->log, {cl, result, requested_size});
#endif // ASUKA_DEBUG

// #if ASUKA_ASAN
//             ASAN_UNPOISON_MEMORY_REGION(result, ChunkSize);
// #endif // ASUKA_ASAN
        }
    }

    return result;
}

template <usize ChunkSize>
#if ASUKA_DEBUG
void deallocate__(pool_allocator<ChunkSize> *allocator, void *memory_to_free, CodeLocation cl)
#else
void deallocate__(pool_allocator<ChunkSize> *allocator, void *memory_to_free)
#endif
{
    using chunk_t = typename pool_allocator<ChunkSize>::memory_chunk;

    chunk_t *chunk = (chunk_t *) memory_to_free;
    chunk->next_chunk = allocator->free_list;
    allocator->free_list = chunk;

    allocator->used -= sizeof(chunk_t);

#if ASUKA_DEBUG
    pop_allocation_entry(&allocator->log, memory_to_free);
#endif // ASUKA_DEBUG

// #if ASUKA_ASAN
//     ASAN_POISON_MEMORY_REGION(header->padding, sizeof(header->padding));
// #endif // ASUKA_ASAN
}


} // namespace memory
