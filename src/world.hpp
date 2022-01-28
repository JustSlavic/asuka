#pragma once

#include <defines.hpp>
#include <math.hpp>
#include <index.hpp>
#include "memory_arena.hpp"


enum Tile {
    TILE_FREE = 0,
    TILE_WALL,
    TILE_INVALID,
    TILE_DOOR_UP,
    TILE_DOOR_DOWN,
    TILE_WIN,
};


struct LowFrequencyEntity;
using LowEntityIndex = Index<LowFrequencyEntity>;


struct EntityBlock {
    uint32 low_entity_count;
    LowEntityIndex entities[16];
    EntityBlock *next_block;
};


struct Chunk {
    // Coordinates of a Chunk inside World. Used in hash table.
    int32 chunk_x;
    int32 chunk_y;
    int32 chunk_z;

    // @note: This value also determines if chunk is initialized.
    EntityBlock *entities;

    Chunk *next_in_hashtable;
};

struct ChunkPosition {
    // @note: Signed integers allow chunks be placed in any direction from the center of the world.
    int32 chunk_x;
    int32 chunk_y;
    int32 chunk_z;

    math::v2 relative_position_in_chunk;
};

struct World {
    float32 tile_side_in_meters;

    uint32 tile_count_x; // Per chunk
    uint32 tile_count_y; // Per chunk

    // @note: Size of the array should be power of two for now.
    // @todo:
    Chunk chunks_hash_table[4096];

    uint32 chunk_shift;
    uint32 chunk_mask;
};

struct AbsoluteWorldPosition {
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
    int32 absolute_tile_z;

    // In pixels inside a tile
    math::v2 relative_position_on_tile;
};


ChunkPosition GetChunkPosition(World *world, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z);
ChunkPosition GetChunkPosition(World *world, AbsoluteWorldPosition pos);
AbsoluteWorldPosition map_into_tile_space(World* world, AbsoluteWorldPosition base_position, math::v2 offset);
math::vector2 PositionDifference(World *world, AbsoluteWorldPosition p1, AbsoluteWorldPosition p2);
AbsoluteWorldPosition MovePosition(World *world, AbsoluteWorldPosition pos, math::v2 offset);

ChunkPosition change_entity_location(World *world, LowEntityIndex index, ChunkPosition old_position, ChunkPosition new_position);


#ifdef UNITY_BUILD
#include "world.cpp"
#endif // UNITY_BUILD
