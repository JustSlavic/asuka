#include "world.hpp"
#include <os/memory.hpp>


ChunkPosition GetChunkPosition(World *world, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z) {
    ChunkPosition result {};

    result.chunk_x = abs_tile_x >> world->chunk_shift;
    result.chunk_y = abs_tile_y >> world->chunk_shift;
    result.chunk_z = abs_tile_z;

    uint32 tile_x = abs_tile_x & world->chunk_mask;
    uint32 tile_y = abs_tile_y & world->chunk_mask;

    result.relative_position_in_chunk = math::v2{ 0, 0 };

    return result;
}


ChunkPosition GetChunkPosition(World *world, AbsoluteWorldPosition pos) {
    ChunkPosition result = GetChunkPosition(world, pos.absolute_tile_x, pos.absolute_tile_y, pos.absolute_tile_z);

    return result;
}


INTERNAL_FUNCTION
bool32 is_chunk_valid(Chunk *chunk) {
    bool32 result = (chunk->entities != NULL);

    return result;
}


INTERNAL_FUNCTION
INLINE_FUNCTION
Chunk* GetTileChunk(World* world, int32 chunk_x, int32 chunk_y, int32 chunk_z, MemoryArena *arena = NULL)
{
    // @todo: MAKE BETTER HASH FUNCTION!!!
    hash_t hash = chunk_x * 3 + chunk_y * 13 + chunk_z * 53;
    uint32 index = hash & (ARRAY_COUNT(world->chunks_hash_table) - 1);
    ASSERT(index < ARRAY_COUNT(world->chunks_hash_table));

    Chunk *chunk = world->chunks_hash_table + index;
    ASSERT(chunk);

    while (chunk) {
        // If initialized and coordinates match up, break from the loop.
        if ((!is_chunk_valid(chunk)) ||
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

    if (chunk && !is_chunk_valid(chunk)) {
        chunk->chunk_x = chunk_x;
        chunk->chunk_y = chunk_y;
        chunk->chunk_z = chunk_z;

        chunk->entities = push_struct(arena, EntityBlock);

        chunk->next_in_hashtable = 0;
    }

    return chunk;
}


INTERNAL_FUNCTION
INLINE_FUNCTION
void NormalizeCoordinate(World *world, i32 *tile, f32 *relative_coord) {
    f32 coord = *relative_coord + 0.5f * world->tile_side_in_meters;
    i32 offset = math::floor_to_int32(coord / world->tile_side_in_meters);

    *tile += offset;
    *relative_coord -= offset * world->tile_side_in_meters;

    ASSERT(*relative_coord - (0.5f * world->tile_side_in_meters) < EPSILON);
    ASSERT(*relative_coord + (0.5f * world->tile_side_in_meters) > -EPSILON);
}


AbsoluteWorldPosition map_into_tile_space(World* world, AbsoluteWorldPosition base_position, math::v2 offset) {
    AbsoluteWorldPosition result = base_position;
    result.relative_position_on_tile += offset;

    math::rectangle2 tile = math::rect2::from_center_dim(math::v2::zero(), math::v2::make(world->tile_side_in_meters));

    NormalizeCoordinate(world, &result.absolute_tile_x, &result.relative_position_on_tile.x);
    NormalizeCoordinate(world, &result.absolute_tile_y, &result.relative_position_on_tile.y);

    ASSERT(in_rectangle(tile, result.relative_position_on_tile));

    // ASSERT((tile_left <= result.relative_position_on_tile.x) && (result.relative_position_on_tile.x < tile_right));
    // ASSERT((tile_top  <= result.relative_position_on_tile.y) && (result.relative_position_on_tile.y < tile_bottom));

    return result;
}


math::vector2 PositionDifference(World *world, AbsoluteWorldPosition p1, AbsoluteWorldPosition p2) {
    math::v2 result {};

    if (p1.absolute_tile_z != p2.absolute_tile_z) {
        return result;
    }

    result.x =
        (p1.absolute_tile_x * world->tile_side_in_meters + p1.relative_position_on_tile.x) -
        (p2.absolute_tile_x * world->tile_side_in_meters + p2.relative_position_on_tile.x);
    result.y =
        (p1.absolute_tile_y * world->tile_side_in_meters + p1.relative_position_on_tile.y) -
        (p2.absolute_tile_y * world->tile_side_in_meters + p2.relative_position_on_tile.y);

    return result;
}


ChunkPosition change_entity_location(World *world, LowEntityIndex index, ChunkPosition old_position, ChunkPosition new_position) {
    ChunkPosition result {};

    ASSERT_FAIL("Not implemented!");

    return result;
}
