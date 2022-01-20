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
tile_chunk* GetTileChunk(Tilemap* tilemap, int32 chunk_x, int32 chunk_y, int32 chunk_z) {
    // Allow negative coordinates of chunks so that chunks can grow
    // from the center of the tilemap in any direction.

    tile_chunk* result = NULL;

    if (chunk_x >= 0 && chunk_x < (int32)tilemap->chunk_count_x &&
        chunk_y >= 0 && chunk_y < (int32)tilemap->chunk_count_y &&
        chunk_z >= 0 && chunk_z < (int32)tilemap->chunk_count_z)
    {
        result = &tilemap->chunks[
            chunk_z * tilemap->chunk_count_x * tilemap->chunk_count_y +
            chunk_y * tilemap->chunk_count_x +
            chunk_x];
    }

    return result;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
tile_t GetTileValue_Unchecked(Tilemap* tilemap, tile_chunk* tilechunk, uint32 tile_x, uint32 tile_y) {
    ASSERT(tile_x < tilemap->tile_count_x);
    ASSERT(tile_y < tilemap->tile_count_y);

    tile_t result = tilechunk->tiles[tile_y*tilemap->tile_count_x + tile_x];
    return result;
}

tile_t GetTileValue(Tilemap* tilemap, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z) {
    tile_t result = TILE_INVALID;

    TileChunkPosition chunk_pos = GetChunkPosition(tilemap, abs_tile_x, abs_tile_y, abs_tile_z);
    tile_chunk *chunk = GetTileChunk(tilemap, chunk_pos.chunk_x, chunk_pos.chunk_y, chunk_pos.chunk_z);

    if (chunk != NULL && chunk->tiles != NULL) {
        result = GetTileValue_Unchecked(tilemap, chunk, chunk_pos.chunk_relative_x, chunk_pos.chunk_relative_y);
    }

    return result;
}

void SetTileValue(MemoryArena *arena, Tilemap *tilemap, int32 abs_x, int32 abs_y, int32 abs_z, tile_t tile_value) {
    TileChunkPosition chunk_pos = GetChunkPosition(tilemap, abs_x, abs_y, abs_z);
    tile_chunk *chunk = GetTileChunk(tilemap, chunk_pos.chunk_x, chunk_pos.chunk_y, chunk_pos.chunk_z);

    ASSERT(chunk);

    if (chunk->tiles == NULL) {
        chunk->tiles = push_array(arena, tile_t, tilemap->tile_count_x * tilemap->tile_count_y);
    }

    chunk->tiles[chunk_pos.chunk_relative_y * tilemap->tile_count_x + chunk_pos.chunk_relative_x] = tile_value;
}

bool32 IsTileValueEmpty(tile_t tile_value) {
    bool32 result = (tile_value == TILE_FREE || tile_value == TILE_DOOR_UP || tile_value == TILE_DOOR_DOWN || tile_value == TILE_WIN);
    return result;
}

bool32 IsWorldPointEmpty(Tilemap *tilemap, TilemapPosition pos) {
    bool32 is_empty = false;

    tile_t tile_value = GetTileValue(tilemap, pos.absolute_tile_x, pos.absolute_tile_y, pos.absolute_tile_z);
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

    float32 tile_top   = -0.5f * tilemap->tile_side_in_meters;
    float32 tile_bottom = 0.5f * tilemap->tile_side_in_meters;

    float32 tile_left = -0.5f * tilemap->tile_side_in_meters;
    float32 tile_right = 0.5f * tilemap->tile_side_in_meters;

    NormalizeCoordinate(tilemap, &result.absolute_tile_x, &result.relative_position_on_tile.x);
    NormalizeCoordinate(tilemap, &result.absolute_tile_y, &result.relative_position_on_tile.y);

    ASSERT((tile_left <= result.relative_position_on_tile.x) && (result.relative_position_on_tile.x < tile_right));
    ASSERT((tile_top  <= result.relative_position_on_tile.y) && (result.relative_position_on_tile.y < tile_bottom));

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
