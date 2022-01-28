#ifndef ASUKA_TILEMAP_HPP
#define ASUKA_TILEMAP_HPP

#include <defines.hpp>
#include <math.hpp>
#include "memory_arena.hpp"


enum Tile {
    TILE_INVALID = 0,
    TILE_FREE,
};


struct TileChunk {
    // Coordinates of a TileChunk inside Tilemap. Used in hash table.
    int32 chunk_x;
    int32 chunk_y;

    // @note: This value determines if TileChunk is initialized.
    Tile *tiles;

    TileChunk *next_bucket_in_hashtable;
};

struct TileChunkPosition {
    // @note: Signed integers allow chunks grow from the center of a map in any direction.
    int32 chunk_x;
    int32 chunk_y;

    // @note: Unsigned integers because there could not be tiles with negative coordinates inside a TileChunk.
    uint32 chunk_relative_x;
    uint32 chunk_relative_y;
};

struct Tilemap {
    float32 tile_side_in_meters;

    // How many tiles inside given chunk.
    uint32 tile_count_x;
    uint32 tile_count_y;

    // @note: Size of the array should be power of two for now.
    TileChunk chunks_hash_table[4096];

    uint32 chunk_shift;
    uint32 chunk_mask;
};

struct TilemapPosition {
    /*

    Higher bits represent coordinates of a chunk;
    Lower bits represent coordinates of a tile inside a given chunk

    Chunk coordinates can be negative:

      chunk_x = -1   |   chunk_x =  0    |   chunk_x =  1
      chunk_y =  0   |   chunk_y =  0    |   chunk_y =  0
    ---------------(0,0)-----------------+-----------------
      chunk_x = -1   |   chunk_x =  0    |   chunk_x =  1
      chunk_y = -1   |   chunk_y = -1    |   chunk_y = -1

    */

    int32 absolute_tile_x;
    int32 absolute_tile_y;
};


TileChunkPosition GetChunkPosition(Tilemap *tilemap, int32 abs_tile_x, int32 abs_tile_y);
TileChunkPosition GetChunkPosition(Tilemap *tilemap, TilemapPosition pos);
Tile GetTileValue(Tilemap *tilemap, int32 abs_tile_x, int32 abs_tile_y);
void SetTileValue(MemoryArena *arena, Tilemap *tilemap, int32 abs_x, int32 abs_y, Tile tile_value);
bool32 IsTileValueEmpty(Tile tile_value);
bool32 IsWorldPointEmpty(Tilemap *tilemap, TilemapPosition pos);
TilemapPosition map_into_tile_space(Tilemap* tilemap, TilemapPosition base_position, math::v2 offset);
math::vector2 PositionDifference(Tilemap *tilemap, TilemapPosition p1, TilemapPosition p2);
TilemapPosition MovePosition(Tilemap *tilemap, TilemapPosition pos, math::v2 offset);

#ifdef UNITY_BUILD
#include "tilemap.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_TILEMAP_HPP
