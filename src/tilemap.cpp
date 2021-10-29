#include "tilemap.hpp"


tile_chunk_position GetChunkPosition(tile_map *map, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z) {
    tile_chunk_position result {};

    result.chunk_x = abs_tile_x >> map->chunk_shift;
    result.chunk_y = abs_tile_y >> map->chunk_shift;
    result.chunk_z = abs_tile_z;

    result.chunk_relative_x = abs_tile_x & map->chunk_mask;
    result.chunk_relative_y = abs_tile_y & map->chunk_mask;

    return result;
}

tile_chunk_position GetChunkPosition(tile_map *map, tile_map_position pos) {
    tile_chunk_position result;
    result = GetChunkPosition(map, pos.absolute_tile_x, pos.absolute_tile_y, pos.absolute_tile_z);
    return result;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
tile_chunk* GetTileChunk(tile_map* map, int32 chunk_x, int32 chunk_y, int32 chunk_z) {
    // Allow negative coordinates of chunks so that chunks can grow
    // from the center of the map in any direction.

    // @todo: what asserts should I use here?
    // ASSERT(world->tilechunk_count_x - tilechunk_x > 0);
    // ASSERT(world->tilechunk_count_y - tilechunk_y > 0);

    tile_chunk* result = NULL;

    if (chunk_x >= 0 && chunk_x < (int32)map->chunk_count_x &&
        chunk_y >= 0 && chunk_y < (int32)map->chunk_count_y &&
        chunk_z >= 0 && chunk_z < (int32)map->chunk_count_z)
    {
        result = &map->chunks[
            chunk_z * map->chunk_count_x * map->chunk_count_y +
            chunk_y * map->chunk_count_x +
            chunk_x];
    }

    return result;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
tile_t GetTileValue_Unchecked(tile_map* map, tile_chunk* tilechunk, uint32 tile_x, uint32 tile_y) {
    ASSERT(tile_x < map->tile_count_x);
    ASSERT(tile_y < map->tile_count_y);

    tile_t result = tilechunk->tiles[tile_y*map->tile_count_x + tile_x];
    return result;
}

tile_t GetTileValue(tile_map* map, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z) {
    tile_t result = TILE_INVALID;

    tile_chunk_position chunk_pos = GetChunkPosition(map, abs_tile_x, abs_tile_y, abs_tile_z);
    tile_chunk *chunk = GetTileChunk(map, chunk_pos.chunk_x, chunk_pos.chunk_y, chunk_pos.chunk_z);

    if (chunk != NULL && chunk->tiles != NULL) {
        result = GetTileValue_Unchecked(map, chunk, chunk_pos.chunk_relative_x, chunk_pos.chunk_relative_y);
    }

    return result;
}

void SetTileValue(memory_arena *arena, tile_map *tilemap, int32 abs_x, int32 abs_y, int32 abs_z, tile_t tile_value) {
    tile_chunk_position chunk_pos = GetChunkPosition(tilemap, abs_x, abs_y, abs_z);
    tile_chunk *chunk = GetTileChunk(tilemap, chunk_pos.chunk_x, chunk_pos.chunk_y, chunk_pos.chunk_z);

    ASSERT(chunk);

    if (chunk->tiles == NULL) {
        chunk->tiles = push_array(arena, tile_t, tilemap->tile_count_x * tilemap->tile_count_y);
    }

    chunk->tiles[chunk_pos.chunk_relative_y * tilemap->tile_count_x + chunk_pos.chunk_relative_x] = tile_value;
}

bool32 IsWorldPointEmpty(tile_map *map, tile_map_position pos) {
    bool32 is_empty = false;

    int32 tile_value = GetTileValue(map, pos.absolute_tile_x, pos.absolute_tile_y, pos.absolute_tile_z);

    is_empty = (tile_value == TILE_FREE || tile_value == TILE_DOOR_UP || tile_value == TILE_DOOR_DOWN || tile_value == TILE_WIN);
    return is_empty;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
void NormalizeCoordinate(tile_map *map, i32 *tile, f32 *relative_coord) {
    f32 coord = *relative_coord + 0.5f * map->tile_side_in_meters;
    i32 offset = floor_to_int32(coord / map->tile_side_in_meters);

    *tile += offset;
    *relative_coord -= offset * map->tile_side_in_meters;

    ASSERT(*relative_coord - (0.5f * map->tile_side_in_meters) < ASUKA_EPS);
    ASSERT(*relative_coord + (0.5f * map->tile_side_in_meters) > -ASUKA_EPS);
}

tile_map_position NormalizeTilemapPosition(tile_map* map, tile_map_position position) {
    tile_map_position result = position;

    float32 tile_top   = -0.5f * map->tile_side_in_meters;
    float32 tile_bottom = 0.5f * map->tile_side_in_meters;

    float32 tile_left = -0.5f * map->tile_side_in_meters;
    float32 tile_right = 0.5f * map->tile_side_in_meters;

    NormalizeCoordinate(map, &result.absolute_tile_x, &result.relative_position_on_tile.x);
    NormalizeCoordinate(map, &result.absolute_tile_y, &result.relative_position_on_tile.y);

    ASSERT((tile_left <= result.relative_position_on_tile.x) && (result.relative_position_on_tile.x < tile_right));
    ASSERT((tile_top  <= result.relative_position_on_tile.y) && (result.relative_position_on_tile.y < tile_bottom));

    return result;
}
