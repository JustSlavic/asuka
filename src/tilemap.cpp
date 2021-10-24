#include "tilemap.hpp"


tile_chunk_position GetChunkPosition(tile_map *map, int32 abs_tile_x, int32 abs_tile_y) {
    tile_chunk_position result {};

    result.tilechunk_x = abs_tile_x >> map->chunk_shift;
    result.tilechunk_y = abs_tile_y >> map->chunk_shift;

    result.chunk_relative_x = abs_tile_x & map->chunk_mask;
    result.chunk_relative_y = abs_tile_y & map->chunk_mask;

    return result;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
uint32 GetTileValue_Unchecked(tile_map* map, tile_chunk* tilechunk, uint32 tile_x, uint32 tile_y) {
    ASSERT(tile_x < map->tile_count_x);
    ASSERT(tile_y < map->tile_count_y);

    uint32 result = tilechunk->tiles[tile_y*map->tile_count_x + tile_x];
    return result;
}

INTERNAL_FUNCTION
INLINE_FUNCTION
tile_chunk* GetTileChunk(tile_map* map, int32 tilechunk_x, int32 tilechunk_y) {
    // Allow negative coordinates of chunks so that chunks can grow
    // from the center of the map in any direction.

    // @todo: what asserts should I use here?
    // ASSERT(world->tilechunk_count_x - tilechunk_x > 0);
    // ASSERT(world->tilechunk_count_y - tilechunk_y > 0);

    tile_chunk* result = NULL;

    if (tilechunk_x >= 0 && tilechunk_x < (int32)map->tilechunk_count_x &&
        tilechunk_y >= 0 && tilechunk_y < (int32)map->tilechunk_count_y)
    {
        result = &map->tilechunks[tilechunk_y * map->tilechunk_count_x + tilechunk_x];
    }

    return result;
}

int32 GetTileValue(tile_map* map, int32 abs_tile_x, int32 abs_tile_y) {
    int32 result = -1;

    tile_chunk_position chunk_pos = GetChunkPosition(map, abs_tile_x, abs_tile_y);
    tile_chunk *chunk = GetTileChunk(map, chunk_pos.tilechunk_x, chunk_pos.tilechunk_y);

    if (chunk) {
        result = GetTileValue_Unchecked(map, chunk, chunk_pos.chunk_relative_x, chunk_pos.chunk_relative_y);
    }

    return result;
}

bool32 IsWorldPointEmpty(tile_map *map, tile_map_position pos) {
    bool32 is_empty = false;

    int32 tile_value = GetTileValue(map, pos.absolute_tile_x, pos.absolute_tile_y);

    is_empty = (tile_value == 0);
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
