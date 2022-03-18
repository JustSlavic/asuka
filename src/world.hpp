#pragma once

#include <defines.hpp>
#include <math.hpp>
#include <entity.hpp>
#include "memory_arena.hpp"


/*

    Now there are two coordinate systems:

    1. Camera space: center at camera position, high frequency entity's position are in floating point coordinates.
    2. World space: center at world zero: world made of chunks, low frequency entity's position includes coordinates of a chunk and relative position within given chunk.

       Chunks of the world are stored in the hash table. Each chunk have EntityBlock which include low entity indecies.

*/


enum Tile {
    TILE_FREE = 0,
    TILE_WALL,
    TILE_INVALID,
    TILE_DOOR_UP,
    TILE_DOOR_DOWN,
    TILE_WIN,
};


struct EntityBlock {
    u32 entity_count;
    LowEntityIndex entities[16];
    EntityBlock *next_block;
};


struct Chunk {
    // Coordinates of a Chunk inside World. Used in hash table.
    i32 chunk_x;
    i32 chunk_y;
    i32 chunk_z;

    // @note: This value also determines if chunk is initialized.
    EntityBlock *entities;

    Chunk *next_in_hashtable;
};


struct World {
    f32 tile_side_in_meters;
    f32 chunk_side_in_meters;

    // @note: Size of the array should be power of two for now.
    Chunk chunks_hash_table[32];

    EntityBlock *next_free_block;
};


WorldPosition null_position();
v2 position_difference(World *world, WorldPosition p1, WorldPosition p2);
WorldPosition change_entity_location(World *world, LowEntityIndex index, WorldPosition old_position, WorldPosition new_position);
b32 is_canonical(World *world, WorldPosition p);
WorldPosition canonicalize_position(World *world, WorldPosition p);
WorldPosition map_into_world_space(World *world, WorldPosition camera_position);


#if UNITY_BUILD
#include "world.cpp"
#endif // UNITY_BUILD
