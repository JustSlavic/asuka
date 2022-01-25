#include "tilemap.hpp"


TileChunkPosition GetChunkPosition(Tilemap *tilemap, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z) {
    TileChunkPosition result {};

    result.chunk_x = abs_tile_x >> tilemap->chunk_shift;
    result.chunk_y = abs_tile_y >> tilemap->chunk_shift;
    result.chunk_z = abs_tile_z;

    result.chunk_relative_x = abs_tile_x & tilemap->chunk_mask;
    result.chunk_relative_y = abs_tile_y & tilemap->chunk_mask;

    return result;
}

TileChunkPosition GetChunkPosition(Tilemap *tilemap, TilemapPosition pos) {
    TileChunkPosition result;
    result = GetChunkPosition(tilemap, pos.absolute_tile_x, pos.absolute_tile_y, pos.absolute_tile_z);
    return result;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
TileChunk* GetTileChunk(Tilemap* tilemap, int32 chunk_x, int32 chunk_y, int32 chunk_z, MemoryArena *arena = NULL)
{
    // @todo: MAKE BETTER HASH FUNCTION!!!
    hash_t hash = chunk_x * 3 + chunk_y * 13 + chunk_z * 53;
    uint32 index = hash & (ARRAY_COUNT(tilemap->chunks_hash_table) - 1);
    ASSERT(index < ARRAY_COUNT(tilemap->chunks_hash_table));

    TileChunk *chunk = tilemap->chunks_hash_table + index;
    ASSERT(chunk);

    while (chunk) {
        // If initialized and coordinates match up, break from the loop.
        if ((chunk->tiles == NULL) ||
            (chunk->chunk_x == chunk_x &&
             chunk->chunk_y == chunk_y &&
             chunk->chunk_z == chunk_z))
        {
            break;
        }

        if (arena && chunk->next_bucket_in_hashtable == NULL)
        {
            osOutputDebugString("Allocate TileChunk at (%d, %d, %d)\n", chunk_x, chunk_y, chunk_z);

            chunk->next_bucket_in_hashtable = push_struct(arena, TileChunk);
        }

        chunk = chunk->next_bucket_in_hashtable;
    }

    if (chunk && chunk->tiles == NULL) {
        if (arena) {
            osOutputDebugString("Allocate Tiles for Chunk at (%d, %d, %d)\n", chunk_x, chunk_y, chunk_z);

            chunk->chunk_x = chunk_x;
            chunk->chunk_y = chunk_y;
            chunk->chunk_z = chunk_z;

            uint32 tile_count = tilemap->tile_count_x * tilemap->tile_count_y;
            chunk->tiles = push_array(arena, Tile, tile_count);

            // Clear memory just in case.
            for (uint32 tile_index = 0; tile_index < tile_count; tile_index++) {
                chunk->tiles[tile_index] = Tile(0);
            }

            chunk->next_bucket_in_hashtable = 0;
        } else {
            chunk = NULL;
        }
    }

    return chunk;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
Tile GetTileValue_Unchecked(Tilemap* tilemap, TileChunk* tilechunk, uint32 tile_x, uint32 tile_y) {
    ASSERT(tile_x < tilemap->tile_count_x);
    ASSERT(tile_y < tilemap->tile_count_y);

    Tile result = tilechunk->tiles[tile_y*tilemap->tile_count_x + tile_x];
    return result;
}

Tile GetTileValue(Tilemap* tilemap, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z) {
    Tile result = TILE_INVALID;

    TileChunkPosition chunk_pos = GetChunkPosition(tilemap, abs_tile_x, abs_tile_y, abs_tile_z);
    TileChunk *chunk = GetTileChunk(tilemap, chunk_pos.chunk_x, chunk_pos.chunk_y, chunk_pos.chunk_z);

    if (chunk != NULL && chunk->tiles != NULL) {
        result = GetTileValue_Unchecked(tilemap, chunk, chunk_pos.chunk_relative_x, chunk_pos.chunk_relative_y);
    }

    return result;
}

void SetTileValue(MemoryArena *arena, Tilemap *tilemap, int32 abs_x, int32 abs_y, int32 abs_z, Tile tile_value) {
    TileChunkPosition chunk_pos = GetChunkPosition(tilemap, abs_x, abs_y, abs_z);
    TileChunk *chunk = GetTileChunk(tilemap, chunk_pos.chunk_x, chunk_pos.chunk_y, chunk_pos.chunk_z, arena);

    ASSERT(chunk);
    ASSERT(chunk->tiles);

    chunk->tiles[chunk_pos.chunk_relative_y * tilemap->tile_count_x + chunk_pos.chunk_relative_x] = tile_value;
}

bool32 IsTileValueEmpty(Tile tile_value) {
    bool32 result = (tile_value == TILE_FREE || tile_value == TILE_DOOR_UP || tile_value == TILE_DOOR_DOWN || tile_value == TILE_WIN);
    return result;
}

bool32 IsWorldPointEmpty(Tilemap *tilemap, TilemapPosition pos) {
    bool32 is_empty = false;

    Tile tile_value = GetTileValue(tilemap, pos.absolute_tile_x, pos.absolute_tile_y, pos.absolute_tile_z);
    is_empty = IsTileValueEmpty(tile_value);
    return is_empty;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
void NormalizeCoordinate(Tilemap *tilemap, i32 *tile, f32 *relative_coord) {
    f32 coord = *relative_coord + 0.5f * tilemap->tile_side_in_meters;
    i32 offset = math::floor_to_int32(coord / tilemap->tile_side_in_meters);

    *tile += offset;
    *relative_coord -= offset * tilemap->tile_side_in_meters;

    ASSERT(*relative_coord - (0.5f * tilemap->tile_side_in_meters) < EPSILON);
    ASSERT(*relative_coord + (0.5f * tilemap->tile_side_in_meters) > -EPSILON);
}


TilemapPosition map_into_tile_space(Tilemap* tilemap, TilemapPosition base_position, math::v2 offset) {
    TilemapPosition result = base_position;
    result.relative_position_on_tile += offset;

    float32 Tileop   = -0.5f * tilemap->tile_side_in_meters;
    float32 tile_bottom = 0.5f * tilemap->tile_side_in_meters;

    float32 tile_left = -0.5f * tilemap->tile_side_in_meters;
    float32 tile_right = 0.5f * tilemap->tile_side_in_meters;

    NormalizeCoordinate(tilemap, &result.absolute_tile_x, &result.relative_position_on_tile.x);
    NormalizeCoordinate(tilemap, &result.absolute_tile_y, &result.relative_position_on_tile.y);

    ASSERT((tile_left <= result.relative_position_on_tile.x) && (result.relative_position_on_tile.x < tile_right));
    ASSERT((Tileop  <= result.relative_position_on_tile.y) && (result.relative_position_on_tile.y < tile_bottom));

    return result;
}

math::vector2 PositionDifference(Tilemap *tilemap, TilemapPosition p1, TilemapPosition p2) {
    math::v2 result {};

    if (p1.absolute_tile_z != p2.absolute_tile_z) {
        return result;
    }

    result.x =
        (p1.absolute_tile_x * tilemap->tile_side_in_meters + p1.relative_position_on_tile.x) -
        (p2.absolute_tile_x * tilemap->tile_side_in_meters + p2.relative_position_on_tile.x);
    result.y =
        (p1.absolute_tile_y * tilemap->tile_side_in_meters + p1.relative_position_on_tile.y) -
        (p2.absolute_tile_y * tilemap->tile_side_in_meters + p2.relative_position_on_tile.y);

    return result;
}
