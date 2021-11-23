#ifndef ASUKA_TILEMAP_HPP
#define ASUKA_TILEMAP_HPP

#include <defines.hpp>
#include <math.hpp>
#include "memory_arena.hpp"


enum tile_t {
    TILE_FREE = 0,
    TILE_WALL,
    TILE_INVALID,
    TILE_DOOR_UP,
    TILE_DOOR_DOWN,
    TILE_WIN,
};


struct tile_chunk {
    tile_t *tiles;
};

struct tile_chunk_position {
    // Allow chunks grow from the center of a map in any direction
    int32 chunk_x;
    int32 chunk_y;
    int32 chunk_z;

    // Inside a chunk there could not be tiles on negative coordinates
    uint32 chunk_relative_x;
    uint32 chunk_relative_y;
};

struct Tilemap {
    float32 tile_side_in_meters;

    uint32 tile_count_x;
    uint32 tile_count_y;

    uint32 chunk_count_x;
    uint32 chunk_count_y;
    uint32 chunk_count_z;

    tile_chunk* chunks;

    uint32 chunk_shift;
    uint32 chunk_mask;
};

struct TilemapPosition {
    /*

    Higher bits represent coordinates of a chunk;
    Lower bits represent coordinates of a tile inside a given chunk

    Chunk coordinates can be negative:

                     |                   |
      chunk_x = -1   |   chunk_x = 0     |   chunk_x = 1
      chunk_y = 0    |   chunk_y = 0     |   chunk_y = 0
    ---------------(0,0)-----------------+-----
      chunk_x = -1   |   chunk_x = 0     |   chunk_x = 1
      chunk_y = -1   |   chunk_y = -1    |   chunk_y = -1

    */

    int32 absolute_tile_x;
    int32 absolute_tile_y;
    int32 absolute_tile_z;

    // In pixels inside a tile
    math::v2 relative_position_on_tile;
};


tile_chunk_position GetChunkPosition(Tilemap *map, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z);
tile_chunk_position GetChunkPosition(Tilemap *map, TilemapPosition pos);
tile_t GetTileValue(Tilemap* map, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z);
void SetTileValue(MemoryArena *arena, Tilemap *tilemap, int32 abs_x, int32 abs_y, int32 abs_z, tile_t tile_value);
bool32 IsTileValueEmpty(tile_t tile_value);
bool32 IsWorldPointEmpty(Tilemap *map, TilemapPosition pos);
TilemapPosition NormalizeTilemapPosition(Tilemap* map, TilemapPosition position);
math::vector2 PositionDifference(Tilemap *tilemap, TilemapPosition p1, TilemapPosition p2);
TilemapPosition MovePosition(Tilemap *tilemap, TilemapPosition pos, math::v2 offset);

#ifdef UNITY_BUILD
#include "tilemap.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_TILEMAP_HPP
