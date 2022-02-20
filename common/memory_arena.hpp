#ifndef ASUKA_MEMORY_ARENA_HPP
#define ASUKA_MEMORY_ARENA_HPP

#include <defines.hpp>
#include <os/memory.hpp>


namespace memory {

struct arena_allocator {
    void *memory;
    usize size; // bytes
    usize used; // bytes
};


INLINE
void initialize_arena(arena_allocator *arena, void* memory, usize size) {
    arena->memory = memory;
    arena->size = size;
    arena->used = 0;

    memory::set(memory, 0, size);
}


#define allocate_struct(ARENA, TYPE) (TYPE*)push_memory(ARENA, sizeof(TYPE))
#define allocate_array(ARENA, TYPE, SIZE) (TYPE*)push_memory(ARENA, sizeof(TYPE)*SIZE)

#define push_struct(ARENA, TYPE) allocate_struct(ARENA, TYPE)
#define push_array(ARENA, TYPE, SIZE) allocate_array(ARENA, TYPE, SIZE)


INLINE
void* push_memory(arena_allocator *arena, usize size) {
    ASSERT((arena->used + size) < arena->size);

    void* result = (uint8*)arena->memory + arena->used;
    arena->used += size;

    memory::set(result, 0, size);

    return result;
}


} // namespace memory


#endif // ASUKA_MEMORY_ARENA_HPP
