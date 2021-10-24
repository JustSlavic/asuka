#ifndef ASUKA_TILEMAP_HPP
#define ASUKA_TILEMAP_HPP


struct tile_chunk {
    int32 *tiles;
};

struct tile_chunk_position {
    // Allow chunks grow from the center of a map in any direction
    int32 tilechunk_x;
    int32 tilechunk_y;

    // Inside a chunk there could not be tiles on negative coordinates
    uint32 chunk_relative_x;
    uint32 chunk_relative_y;
};

struct tile_map {
    float32 tile_side_in_meters;

    uint32 tile_count_x;
    uint32 tile_count_y;

    uint32 tilechunk_count_x;
    uint32 tilechunk_count_y;

    tile_chunk* tilechunks;

    uint32 chunk_shift;
    uint32 chunk_mask;
};

struct tile_map_position {
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

    // In pixels inside a tile
    vector2 relative_position_on_tile;
};


tile_chunk_position GetChunkPosition(tile_map *map, int32 abs_tile_x, int32 abs_tile_y);
int32 GetTileValue(tile_map* map, int32 abs_tile_x, int32 abs_tile_y);
bool32 IsWorldPointEmpty(tile_map *map, tile_map_position pos);
tile_map_position NormalizeTilemapPosition(tile_map* map, tile_map_position position);
void SetTileValue(tile_map *tilemap, int32 abs_x, int32 abs_y, int32 tile_value);

#ifdef UNITY_BUILD
#include "tilemap.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_TILEMAP_HPP
