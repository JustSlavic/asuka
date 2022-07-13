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
            memory_chunk *next_chunk;
            byte memory[ChunkSize];
        };
    };

    byte *memory;
    usize size; // bytes
    usize used; // bytes

    memory_chunk *free_list;
};

template <usize ChunkSize>
void initialize(pool_allocator<ChunkSize> *allocator, void *memory, usize size)
{
    using chunk_t = pool_allocator<ChunkSize>::memory_chunk;

    allocator->memory = (byte *) memory;
    allocator->size   = size;
    allocator->used   = 0;
    // @todo: test if I should align this pointer
    allocator->free_list = (chunk_t *) memory;

    chunk_t *header = (chunk_t *) &allocator->free_list;
    chunk_t *pointer = (chunk_t *) allocator->memory;
    for (usize chunk_index = 0; chunk_index < (size / sizeof(chunk_t)); chunk_index++)
    {
        chunk_t *chunk = pointer + chunk_index;
        header->next_chunk = chunk;
        header = chunk;
    }

    header->next_chunk = 0;
}

template <usize ChunkSize>
void *allocate(pool_allocator<ChunkSize> *allocator, usize requested_size, usize alignment)
{
    using chunk_t = pool_allocator<ChunkSize>::memory_chunk;

    if (ChunkSize < requested_size)
    {
        return NULL;
    }

    byte *result = NULL;
    chunk_t *header = allocator->free_list;
    if (allocator->free_list)
    {
        result = allocator->free_list->memory;
        allocator->free_list = allocator->free_list->next_chunk;

        allocator->used += sizeof(chunk_t);
    }

    return result;
}

template <usize ChunkSize>
void deallocate(pool_allocator<ChunkSize> *allocator, void *memory_to_free)
{
    using chunk_t = pool_allocator<ChunkSize>::memory_chunk;

    chunk_t *chunk = (chunk_t *) memory_to_free;
    chunk->next_chunk = allocator->free_list;
    allocator->free_list = chunk;

    allocator->used -= sizeof(chunk_t);
}

} // namespace memory
