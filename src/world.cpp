#include "world.hpp"
#include <os/memory.hpp>


#define INVALID_CHUNK_POSITION INT32_MAX


void initialize_world(World *world, f32 tile_side_in_meters, f32 chunk_side_in_meters) {
    memory::set(world, 0, sizeof(World));

    world->tile_side_in_meters = tile_side_in_meters;
    world->chunk_side_in_meters = chunk_side_in_meters;
}


WorldPosition null_position()
{
    WorldPosition result {};
    result.chunk_x = INVALID_CHUNK_POSITION;

    return result;
}

b32 is_valid(WorldPosition p)
{
    b32 result = (p.chunk_x != INVALID_CHUNK_POSITION) && (p.chunk_y != INVALID_CHUNK_POSITION) && (p.chunk_z != INVALID_CHUNK_POSITION);
    return result;
}


INTERNAL_FUNCTION INLINE
Chunk* get_chunk(World* world, i32 chunk_x, i32 chunk_y, i32 chunk_z, memory::arena_allocator *arena = NULL)
{
    // @todo: MAKE BETTER HASH FUNCTION!!!
    hash_t hash = chunk_x * 3 + chunk_y * 13 + chunk_z * 53;
    u32 index = hash & (ARRAY_COUNT(world->chunks_hash_table) - 1);
    ASSERT(index < ARRAY_COUNT(world->chunks_hash_table));

    Chunk *chunk = world->chunks_hash_table + index;
    ASSERT(chunk);

    while (chunk) {
        // If initialized and coordinates match up, break from the loop.
        if ((chunk->entities == NULL) ||
            (chunk->chunk_x == chunk_x &&
             chunk->chunk_y == chunk_y &&
             chunk->chunk_z == chunk_z))
        {
            break;
        }

        if (arena && chunk->next_in_hashtable == NULL)
        {
            osOutputDebugString("Allocate Chunk at (%d, %d, %d)\n", chunk_x, chunk_y, chunk_z);

            chunk->next_in_hashtable = push_struct(arena, Chunk);
        }

        chunk = chunk->next_in_hashtable;
    }

    if (arena && chunk && (chunk->entities == NULL)) {
        chunk->chunk_x = chunk_x;
        chunk->chunk_y = chunk_y;
        chunk->chunk_z = chunk_z;

        chunk->entities = push_struct(arena, EntityBlock);

        chunk->next_in_hashtable = 0;
    }

    return chunk;
}


INLINE INTERNAL_FUNCTION
Chunk *get_chunk(World *world, WorldPosition position, memory::arena_allocator *arena = NULL) {
    Chunk *result = get_chunk(world, position.chunk_x, position.chunk_y, position.chunk_z, arena);
    return result;
}


v2 position_difference(World *world, WorldPosition p1, WorldPosition p2) {
    v2 result {};

    if (p1.chunk_z != p2.chunk_z) {
        return result;
    }

    result.x =
        (p1.chunk_x * world->chunk_side_in_meters + p1.relative_position_in_chunk.x) -
        (p2.chunk_x * world->chunk_side_in_meters + p2.relative_position_in_chunk.x);
    result.y =
        (p1.chunk_y * world->chunk_side_in_meters + p1.relative_position_in_chunk.y) -
        (p2.chunk_y * world->chunk_side_in_meters + p2.relative_position_in_chunk.y);

    return result;
}


b32 in_same_chunk(World *world, WorldPosition p1, WorldPosition p2) {
    b32 result = ((p1.chunk_x == p2.chunk_x) &&
                     (p1.chunk_x == p2.chunk_x) &&
                     (p1.chunk_x == p2.chunk_x));

    return result;
}


INTERNAL_FUNCTION
EntityBlock *add_entity_block_to_chunk(World *world, Chunk *chunk, memory::arena_allocator *arena) {
    EntityBlock *block = chunk->entities;

    if (world->next_free_block) {
        osOutputDebugString("Reused free entity block from world.\n");

        EntityBlock *next_free_block = world->next_free_block;
        world->next_free_block = world->next_free_block->next_block;

        chunk->entities = next_free_block;
    } else {
        osOutputDebugString("Allocated entity block from memory arena.\n");

        chunk->entities = push_struct(arena, EntityBlock);
    }

    chunk->entities->next_block = block;
    return chunk->entities;
}


INTERNAL_FUNCTION
void push_entity_into_chunk(World *world, Chunk *chunk, LowEntityIndex entity, memory::arena_allocator *arena) {
    EntityBlock *block = chunk->entities;

    if ((block == NULL) ||
        (block->entity_count == ARRAY_COUNT(block->entities)))
    {
        block = add_entity_block_to_chunk(world, chunk, arena);
    }

    block->entities[block->entity_count++] = entity;
}


INTERNAL_FUNCTION
void remove_entity_from_chunk(World *world, Chunk *chunk, LowEntityIndex entity) {
    EntityBlock *first_block = chunk->entities;

    for (EntityBlock *block = chunk->entities; block; block = block->next_block) {
        for (u32 idx = 0; idx < block->entity_count; idx++) {
            if (block->entities[idx] == entity) {
                // Found entity to remove!
                block->entities[idx] = first_block->entities[first_block->entity_count - 1];
                first_block->entity_count -= 1;

                if (first_block->entity_count == 0) {
                    // Free block.
                    chunk->entities = first_block->next_block;

                    first_block->next_block = world->next_free_block;
                    world->next_free_block = first_block;
                    memory::set(first_block, 0, sizeof(EntityBlock));
                }

                return;
            }
        }
    }
}


WorldPosition change_entity_location_internal(World *world, LowEntityIndex low_index, LowEntity *low, WorldPosition *new_position, memory::arena_allocator *arena)
{
    WorldPosition result {};

    if (low && new_position && in_same_chunk(world, low->world_position, *new_position))
    {
        // Leave entity where it is.
    }
    else
    {
        if (is_valid(low->world_position))
        {
            Chunk *old_chunk = get_chunk(world, low->world_position, arena);
            remove_entity_from_chunk(world, old_chunk, low_index);
        }

        if (new_position)
        {
            Chunk *chunk = get_chunk(world, *new_position, arena);
            push_entity_into_chunk(world, chunk, low_index, arena);
        }
    }

    return result;
}


WorldPosition change_entity_location(World *world, LowEntityIndex low_index, LowEntity *low, WorldPosition *new_position, memory::arena_allocator *arena)
{
    WorldPosition result = change_entity_location_internal(world, low_index, low, new_position, arena);

    if (low)
    {
        if (new_position)
        {
            low->world_position = *new_position;
        }
        else
        {
            low->world_position = null_position();
        }
    }

    return result;
}



b32 is_canonical(World *world, v2 p)
{
    b32 result = (p.x >= -0.5f * world->chunk_side_in_meters) &&
                 (p.x <=  0.5f * world->chunk_side_in_meters) &&
                 (p.y >= -0.5f * world->chunk_side_in_meters) &&
                 (p.y <=  0.5f * world->chunk_side_in_meters);

    return result;
}


b32 is_canonical(World *world, WorldPosition p) {
    b32 result = is_canonical(world, p.relative_position_in_chunk);

    return result;
}


WorldPosition canonicalize_position(World *world, WorldPosition p) {
    WorldPosition result = p;

    v2i chunk_offset = round_to_vector2i(p.relative_position_in_chunk / world->chunk_side_in_meters);

    result.chunk_x += chunk_offset.x;
    result.chunk_y += chunk_offset.y;
    result.relative_position_in_chunk -= upcast_to_vector2(chunk_offset) * world->chunk_side_in_meters;

    ASSERT(is_canonical(world, result.relative_position_in_chunk));
    return result;
}


WorldPosition map_into_world_space(World *world, WorldPosition base_position, v2 offset) {
    WorldPosition result = base_position;
    result.relative_position_in_chunk += offset;

    result = canonicalize_position(world, result);
    return result;
}
