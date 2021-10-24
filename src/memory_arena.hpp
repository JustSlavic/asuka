#ifndef ASUKA_MEMORY_ARENA_HPP
#define ASUKA_MEMORY_ARENA_HPP

#include <defines.hpp>


struct memory_arena {
    void *memory;
    usize size; // bytes
    usize used; // bytes
};


INLINE_FUNCTION
void initialize_arena(memory_arena *arena, void* memory, usize size) {
    arena->memory = memory;
    arena->size = size;
    arena->used = 0;
}


#define push_struct(ARENA, TYPE) (TYPE*)push_memory(ARENA, sizeof(TYPE))
#define push_array(ARENA, TYPE, SIZE) (TYPE*)push_memory(ARENA, sizeof(TYPE)*SIZE)

INLINE_FUNCTION
void* push_memory(memory_arena *arena, usize size) {
    ASSERT((arena->used + size) < arena->size);

    void* result = (uint8*)arena->memory + arena->used;
    arena->used += size;

    return result;
}


#endif // ASUKA_MEMORY_ARENA_HPP
