#ifndef ASUKA_MEMORY_ARENA_HPP
#define ASUKA_MEMORY_ARENA_HPP

#include <defines.hpp>


struct MemoryArena {
    void *memory;
    usize size; // bytes
    usize used; // bytes
};


INLINE_FUNCTION
void initialize_arena(MemoryArena *arena, void* memory, usize size) {
    arena->memory = memory;
    arena->size = size;
    arena->used = 0;

    uint8 *zero_memory = (uint8 *)memory;
    for (usize idx = 0; idx < size; idx++) {
        zero_memory[idx] = 0;
    }
}


#define push_struct(ARENA, TYPE) (TYPE*)push_memory(ARENA, sizeof(TYPE))
#define push_array(ARENA, TYPE, SIZE) (TYPE*)push_memory(ARENA, sizeof(TYPE)*SIZE)

INLINE_FUNCTION
void* push_memory(MemoryArena *arena, usize size) {
    ASSERT((arena->used + size) < arena->size);

    void* result = (uint8*)arena->memory + arena->used;
    arena->used += size;

    return result;
}


#endif // ASUKA_MEMORY_ARENA_HPP
