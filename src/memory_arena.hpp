#ifndef ASUKA_MEMORY_ARENA_HPP
#define ASUKA_MEMORY_ARENA_HPP

#include <defines.hpp>


struct memory_arena {
    void*  base;
    uint64 size; // bytes
    uint64 occupied; // bytes
};


inline void initialize_arena(memory_arena *arena, void* memory, uint64 size) {
    arena->base = memory;
    arena->size = size;
    arena->occupied = 0;
}


#define push_struct(ARENA, TYPE) (TYPE*)push_memory(ARENA, sizeof(TYPE))
#define push_array(ARENA, TYPE, SIZE) (TYPE*)push_memory(ARENA, sizeof(TYPE)*SIZE)

inline void* push_memory(memory_arena *arena, uint64 struct_size) {
    ASSERT((arena->occupied + struct_size) < arena->size);

    void* result = (uint8*)arena->base + arena->occupied;
    arena->occupied += struct_size;

    return result;
}


#endif // ASUKA_MEMORY_ARENA_HPP
