#ifndef ASUKA_ALLOCATOR_HPP
#define ASUKA_ALLOCATOR_HPP

#include <defines.hpp>
#include <os/memory.hpp>

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


namespace Asuka {
namespace memory {


struct page_allocator
{
    struct memory_block {
        byte *memory;
        usize size;
        memory_block *next_block;
    };

    memory_block *blocks;
};


INLINE
void initialize(page_allocator *allocator, void *memory = NULL, usize size = 0)
{
    // @note: Ignore arguments, they're here just to comply to the allocator interface
    allocator->blocks = NULL;
}


INLINE
void *allocate_(page_allocator *allocator, usize requested_size, usize alignment)
{
    STATIC_ASSERT(alignof(page_allocator::memory_block) == 8);

    void *result = NULL;
    auto memory = (byte *) allocate_pages(sizeof(page_allocator::memory_block) + requested_size);

    if (memory)
    {
        auto new_block = (page_allocator::memory_block *) memory;
        new_block->memory = memory + sizeof(page_allocator::memory_block);
        new_block->size = requested_size;
        new_block->next_block = allocator->blocks;
        allocator->blocks = new_block;

        result = new_block->memory;
    }

    return result;
}


INLINE
void free(page_allocator *allocator, void *memory_to_free, usize size)
{
    page_allocator::memory_block *previous_block = NULL;
    for (page_allocator::memory_block *block = allocator->blocks;
         block;
         block = block->next_block)
    {
        if (block->memory == memory_to_free)
        {
            free_pages(memory_to_free);

            if (previous_block)
            {
                previous_block->next_block = block->next_block;
            }

            break;
        }

        previous_block = block;
    }
}


struct arena_allocator {
    byte *memory;
    usize size; // bytes
    usize used; // bytes
};


INLINE
void initialize(arena_allocator *allocator, void* memory, usize size)
{
    // @todo: make sure memory is aligned

    allocator->memory = (byte *)memory;
    allocator->size = size;
    allocator->used = 0;
}

INLINE
void reset(arena_allocator *allocator)
{
    allocator->used = 0;
}

INLINE
void* allocate_(arena_allocator *allocator, usize requested_size, usize alignment)
{
    ASSERT_MSG(allocator->memory, "Arena allocator is not initialized!");

    void *result = NULL;

    byte* free_memory = allocator->memory + allocator->used;
    usize padding = (alignment - (((u64) free_memory) % alignment)) % alignment;

    if (requested_size <= (allocator->size - allocator->used - padding))
    {
        result = free_memory + padding;
        allocator->used += requested_size + padding;
    }

    return result;
}

INLINE
void free(arena_allocator *allocator, void *memory_to_free) {}


template <typename A>
struct freelist_allocator
{
    A nested_allocator;
    struct free_memory_chunk
    {
        byte *memory;
        free_memory_chunk *next_chunk;
    };
};


template <typename A>
void initialize(freelist_allocator<A> *allocator)
{
    initialize(allocator->nested_allocator);
}

template <typename A>
void reset(freelist_allocator<A> *allocator)
{
    reset(allocator->nested_allocator);
}

template <typename A>
void *allocate_(freelist_allocator<A> *allocator, usize requested_size, usize alignment)
{

}

template <typename A>
void free(freelist_allocator<A> *allocator, void *memory_to_free);


template <usize CHUNK_SIZE>
struct pool_allocator
{
    byte *memory;
    usize size; // bytes
    usize used; // bytes

    struct free_memory_chunk
    {
        byte *memory;
        free_memory_chunk *next_chunk;
    };

    byte *free_memory;
    free_memory_chunk *free_chunk;
};


template <usize CHUNK_SIZE>
void initialize(pool_allocator<CHUNK_SIZE> *allocator, void *memory, usize size)
{
    allocator->memory = memory;
    allocator->free_memory = memory;
    allocator->size = size;
    allocator->used = 0;
    allocator->free_block = NULL;
}

template <usize CHUNK_SIZE>
void reset(pool_allocator<CHUNK_SIZE> *allocator)
{
    allocator->used = 0;
    allocator->free_block = NULL;
}

template <usize CHUNK_SIZE>
void *allocate_(pool_allocator<CHUNK_SIZE> *allocator, usize requested_size, usize alignment)
{

}

template <usize BLOCK_SIZE>
void free(pool_allocator<BLOCK_SIZE> *allocator, void *memory_to_free)
{

}


struct heap_allocator
{
    void *memory;
    usize size; // bytes
    usize used; // bytes

    struct trailer {
        usize size;
    };

    struct header {
        usize occupied_and_size_bits;

        b32   is_occupied();
        void  set_occupied(b32);

        usize get_size();
        void  set_size(usize);

        void *get_memory_block();
        trailer *get_trailer();

        b32 is_terminator();
        header *get_next_entry();
    };

    header *entry;
    usize entry_count;

    void  initialize(void *, usize);
    void *allocate(usize);
    void  free(void *);

    GLOBAL constexpr usize MIN_BLOCK_SIZE = 16; // bytes;
};

b32 heap_allocator::header::is_occupied() {
    b32 occupied = (occupied_and_size_bits >> 63); // @note: Should I do 8*sizeof() - 1 ?
    return occupied;
}

void heap_allocator::header::set_occupied(b32 occupied) {
    occupied_and_size_bits = (occupied_and_size_bits & (UINT64_MAX >> 1)) | (((usize)(occupied > 0)) << 63);
}

usize heap_allocator::header::get_size() {
    usize result = occupied_and_size_bits & (UINT64_MAX >> 1);
    return result;
}

void heap_allocator::header::set_size(usize size) {
    ASSERT(size < (1ULL << 63));
    occupied_and_size_bits = (occupied_and_size_bits & (1ULL << 63)) | size;
}

b32 heap_allocator::header::is_terminator() {
    return (occupied_and_size_bits == 0);
}

void *heap_allocator::header::get_memory_block() {
    void *result = (void *) (this + 1);
    return result;
}

heap_allocator::header *heap_allocator::header::get_next_entry() {
    usize memory_size = get_size();
    byte *bytes = (byte *)(this + 1);
    // @note: memory_size already takes padding into account, so result will be aligned properly.
    heap_allocator::header *result = (heap_allocator::header *)(bytes + memory_size + sizeof(heap_allocator::trailer));
    return result;
}

void heap_allocator::initialize(void *memory_, usize size_) {
    // @todo: check for memory alignment
    byte *bytes = (byte *)memory_;

    for (usize i = 0; i < size_; i++)
    {
        bytes[i] = (byte) 0xFE;
    }

    usize alignment = alignof(heap_allocator::header);
    usize aligned_size = size_ - size_ % alignment;

    {
        //
        // +------------+----------+------------------------------+-----------+------------+
        // | terminator |  header  |            . . .             |  trailer  | terminator |
        // +------------+----------+------------------------------+-----------+------------+
        //       ↑            ↑                                         ↑            ↑
        //       │            └──────────────────┐                      │            │
        //       └───────────────────────────┐   │                      │            │
        //     1) Set guard terminator to 0 ─┘   │                      │            │
        //     2) Set size into header ──────────┘                      │            │
        //            and into trailer ─────────────────────────────────┘            │
        //     3) Set guard terminator to 0 ─────────────────────────────────────────┘
        //

        usize memory_block_size = aligned_size - 4*sizeof(heap_allocator::header);

        auto *start_terminator = (heap_allocator::header *) bytes;
        set(start_terminator, 0, sizeof(heap_allocator::header));

        auto *header = (heap_allocator::header *) bytes + 1;
        header->occupied_and_size_bits = memory_block_size;

        entry = header;
        entry_count = 1;

        auto *trailer = (heap_allocator::header *) (bytes + aligned_size - 2*sizeof(heap_allocator::header));
        trailer->occupied_and_size_bits = memory_block_size;

        auto *end_terminator = (heap_allocator::header *) (bytes + aligned_size - sizeof(heap_allocator::header));
        set(end_terminator, 0, sizeof(heap_allocator::header));
    }

    memory = memory_;
    size = size_;
    used = 4 * sizeof(heap_allocator::header);
}

void *heap_allocator::allocate(usize requested_size) {
    ASSERT_MSG(memory, "Allocator is not initialized!");

    void *result = NULL;

    usize alignment = alignof(heap_allocator::header);
    requested_size += (alignment - (requested_size % alignment)) % alignment;

    for (heap_allocator::header *current = entry;
         !current->is_terminator();
         current = current->get_next_entry())
    {
        usize memory_size = current->get_size();
        b32   is_occupied = current->is_occupied();

        if (!is_occupied && (memory_size >= requested_size)) {
            usize residual_size = memory_size - requested_size;
            current->set_occupied(true);

            if (residual_size >= (2*sizeof(heap_allocator::header) + heap_allocator::MIN_BLOCK_SIZE)) {
                //
                //     --+--------+-------------+---------+--------+-------------+---------+--
                //       | header |    . . .    | trailer | header |    . . .    | trailer |
                //     --+--------+-------------+---------+--------+-------------+---------+--
                //           ↑                       ↑         ↑                      ↑
                //           └───────────────────┐   │         │                      │
                //     1) Update size in header ─┘   │         │                      │
                //     2) Create new trailer ────────┘         │                      │
                //           and new header  ──────────────────┘                      │
                //     3) Update size in trailer ─────────────────────────────────────┘
                //

                // 1)
                current->set_size(requested_size);

                // 2)
                byte *bytes = (byte *) (current + 1);
                auto *trailer = (heap_allocator::trailer *)(bytes + requested_size); // Trailer for occupied block
                trailer->size = requested_size;

                usize free_size = residual_size - 2*sizeof(u64);

                auto *header = (heap_allocator::header *)(trailer + 1);
                header->occupied_and_size_bits = free_size;

                // 3)
                auto *old_trailer = (heap_allocator::trailer *) (bytes + memory_size);
                old_trailer->size = header->get_size();

                used += sizeof(heap_allocator::header) + sizeof(heap_allocator::trailer);
            } else {
                used += requested_size;
            }

            result = current->get_memory_block();
            entry_count += 1;
            used += requested_size;

            break;
        }
    }

    return result;
}

void merge_with_next(heap_allocator *a, heap_allocator::header *header)
{
    usize size = header->get_size();

    byte *bytes = (byte *)header;
    auto *trailer = (heap_allocator::trailer *) (bytes + sizeof(u64));
    auto *next_header = (heap_allocator::header *) (bytes + sizeof(u64) + size + sizeof(u64));
    usize new_size = size + 2*sizeof(u64) + next_header->get_size();

    header->set_size(new_size);
    trailer->size = new_size;

    a->used -= (new_size - size);
}

void heap_allocator::free(void *memory_to_free) {
    byte *bytes = (byte *) memory_to_free;

    auto *current_header  = (heap_allocator::header *) (bytes - sizeof(u64));
    usize current_size    = current_header->get_size();
    auto *current_trailer = (heap_allocator::trailer *) (bytes + current_size);

    current_header->set_occupied(false);
    used -= current_size;

    auto *previous_trailer = (heap_allocator::trailer *) (bytes - 2*sizeof(u64));
    auto *next_header      = (heap_allocator::header *)  (bytes + current_size + sizeof(u64));

    //
    //     --+---------+--------+----------
    //       | trailer | header |    . . .
    //     --+---------+--------+----------
    //            ↑
    //            └────┐
    // Ask if trailer ─┘ is zero, meaning that I reached the beginning of memory
    //
    if (previous_trailer->size != 0) {
        auto *previous_header  = (heap_allocator::header *) (bytes - 3*sizeof(u64) - previous_trailer->size);
        if (!previous_header->is_occupied()) {
            current_size = previous_header->get_size() + 2*sizeof(u64) + current_header->get_size();

            previous_header->set_size(current_size);
            current_trailer->size = current_size;

            current_header = previous_header;
            used -= 2 * sizeof(u64);
        }
    }

    //
    //   --------+---------+--------+--
    //    . . .  | trailer | header |
    //   --------+---------+--------+--
    //                          ↑
    //                ┌─────────┘
    // Ask if header ─┘ is zero, meaning that I reached the end of memory
    //
    if (next_header->occupied_and_size_bits != 0) {
        auto *next_trailer = (heap_allocator::trailer *) ((byte *)next_header + sizeof(u64) + next_header->get_size());
        if (!next_header->is_occupied()) {
            usize new_size = current_header->get_size() + 2*sizeof(u64) + next_header->get_size();

            current_header->set_size(new_size);
            next_trailer->size = new_size;

            current_trailer = next_trailer;
            used -= 2 * sizeof(u64);
        }
    }
}


#define allocate_struct_(ARENA, TYPE) (TYPE*)allocate_(ARENA, sizeof(TYPE), alignof(TYPE))
#define allocate_struct(ARENA, TYPE) (TYPE*)allocate(ARENA, sizeof(TYPE), alignof(TYPE))

#define allocate_array_(ARENA, TYPE, SIZE) (TYPE*)allocate_(ARENA, sizeof(TYPE)*SIZE, alignof(TYPE))
#define allocate_array(ARENA, TYPE, SIZE) (TYPE*)allocate(ARENA, sizeof(TYPE)*SIZE, alignof(TYPE))


template <typename Allocator>
void* allocate(Allocator *allocator, usize requested_size, usize alignment) {
    void *result = allocate_(allocator, requested_size, alignment);
    if (result)
    {
        memory::set(result, 0, requested_size);
    }
    return result;
}


} // namespace memory
} // namespace Asuka


#endif // ASUKA_ALLOCATOR_HPP
