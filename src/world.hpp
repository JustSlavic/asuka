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
    uint32 entity_count;
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

struct WorldPosition {
    // @note: Signed integers allow chunks be placed in any direction from the center of the world.
    int32 chunk_x;
    int32 chunk_y;
    int32 chunk_z;

    math::v2 relative_position_in_chunk;
};

struct World {
    float32 tile_side_in_meters;
    float32 chunk_size_in_meters;

    uint32 tile_count_x; // Per chunk
    uint32 tile_count_y; // Per chunk

    // @note: Size of the array should be power of two for now.
    Chunk chunks_hash_table[4096];

    EntityBlock *next_free_block;
};


math::vector2 position_difference(World *world, WorldPosition p1, WorldPosition p2);
WorldPosition change_entity_location(World *world, LowEntityIndex index, WorldPosition old_position, WorldPosition new_position);


#ifdef UNITY_BUILD
#include "world.cpp"
#endif // UNITY_BUILD
