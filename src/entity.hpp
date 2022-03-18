#pragma once

#include <math.hpp>
#include <index.hpp>


enum FaceDirection {
    FACE_DIRECTION_DOWN = 0,
    FACE_DIRECTION_LEFT = 1,
    FACE_DIRECTION_RIGHT = 2,
    FACE_DIRECTION_UP = 3,
};


enum EntityType {
    ENTITY_TYPE_NULL,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_WALL,
    ENTITY_TYPE_FAMILIAR,
    ENTITY_TYPE_MONSTER,
    ENTITY_TYPE_SWORD,
};


struct HighEntity;
using HighEntityIndex = Index<HighEntity>;

struct LowEntity;
using LowEntityIndex = Index<LowEntity>;


struct HighEntity {
    v3 position; // Relative to the camera
    v3 velocity;
    i32 chunk_z; // for moving up and down "stairs"

    f32 tBob = 0.0f;

    FaceDirection face_direction;

    LowEntityIndex low_index;
};


struct HealthPoint {
    u32 fill; // In percent (max 100).
    b32 shielded;
    b32 poisoned;
};


enum {
    ENTITY_HEALTH_STARTING_FILL_MAX = 3,
};

struct WorldPosition {
    // @note: Signed integers allow chunks be placed in any direction from the center of the world.
    i32 chunk_x;
    i32 chunk_y;
    i32 chunk_z;

    v2 relative_position_in_chunk;
};


struct LowEntity {
    EntityType type;
    WorldPosition world_position;
    // @note: for "stairs"
    i32 d_abs_tile_z;

    v2 hitbox;
    b32 collidable;

    HighEntityIndex high_index;

    i32 health_max;
    u32 health_fill_max;
    HealthPoint health[32];

    LowEntityIndex sword_index;
    f32 sword_distance_remaining;
};

struct Entity {
    HighEntityIndex high_index;
    HighEntity *high;

    LowEntityIndex low_index;
    LowEntity  *low;
};

