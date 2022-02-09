#include "world.hpp"
#include <os/memory.hpp>



void initialize_world(World *world, f32 tile_side_in_meters, f32 chunk_side_in_meters) {
    memory::set(world, 0, sizeof(World));

    world->tile_side_in_meters = tile_side_in_meters;
    world->chunk_side_in_meters = chunk_side_in_meters;
}


INTERNAL_FUNCTION INLINE
Chunk* get_chunk(World* world, int32 chunk_x, int32 chunk_y, int32 chunk_z, MemoryArena *arena = NULL)
{
    // @todo: MAKE BETTER HASH FUNCTION!!!
    hash_t hash = chunk_x * 3 + chunk_y * 13 + chunk_z * 53;
    uint32 index = hash & (ARRAY_COUNT(world->chunks_hash_table) - 1);
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
Chunk *get_chunk(World *world, WorldPosition position, MemoryArena *arena = NULL) {
    Chunk *result = get_chunk(world, position.chunk_x, position.chunk_y, position.chunk_z, arena);
    return result;
}


math::vector2 position_difference(World *world, WorldPosition p1, WorldPosition p2) {
    math::v2 result {};

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


bool32 in_same_chunk(World *world, WorldPosition p1, WorldPosition p2) {
    bool32 result = ((p1.chunk_x == p2.chunk_x) &&
                     (p1.chunk_x == p2.chunk_x) &&
                     (p1.chunk_x == p2.chunk_x));

    return result;
}


INTERNAL_FUNCTION
EntityBlock *add_entity_block_to_chunk(World *world, Chunk *chunk, MemoryArena *arena) {
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
void push_entity_into_chunk(World *world, Chunk *chunk, LowEntityIndex entity, MemoryArena *arena) {
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
        for (uint32 idx = 0; idx < block->entity_count; idx++) {
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


WorldPosition change_entity_location(World *world, LowEntityIndex entity, WorldPosition *old_position, WorldPosition *new_position, MemoryArena *arena) {
    WorldPosition result {};

    ASSERT(new_position);

    if (old_position && in_same_chunk(world, *old_position, *new_position)) {
        // Leave entity where it is.
    } else {
        if (old_position) {
            Chunk *old_chunk = get_chunk(world, *old_position, arena);
            remove_entity_from_chunk(world, old_chunk, entity);
        }

        Chunk *chunk = get_chunk(world, *new_position, arena);
        push_entity_into_chunk(world, chunk, entity, arena);
    }

    return result;
}


bool32 is_canonical(World *world, math::v2 p) {
    bool32 result = (p.x >= -0.5f * world->chunk_side_in_meters) &&
                    (p.x <=   0.5f * world->chunk_side_in_meters) &&
                    (p.y >= -0.5f * world->chunk_side_in_meters) &&
                    (p.y <=   0.5f * world->chunk_side_in_meters);

    return result;
}


bool32 is_canonical(World *world, WorldPosition p) {
    bool32 result = is_canonical(world, p.relative_position_in_chunk);

    return result;
}


WorldPosition canonicalize_position(World *world, WorldPosition p) {
    WorldPosition result = p;

    math::v2i chunk_offset = math::round_to_vector2i(p.relative_position_in_chunk / world->chunk_side_in_meters);

    result.chunk_x += chunk_offset.x;
    result.chunk_y += chunk_offset.y;
    result.relative_position_in_chunk -= math::upcast_to_vector2(chunk_offset) * world->chunk_side_in_meters;

    ASSERT(is_canonical(world, result.relative_position_in_chunk));
    return result;
}


WorldPosition map_into_world_space(World *world, WorldPosition base_position, math::v2 offset) {
    WorldPosition result = base_position;
    result.relative_position_in_chunk += offset;

    result = canonicalize_position(world, result);
    return result;
}
