#pragma once

namespace Game {


struct HealthPoint {
    u32 fill; // In percent (max 100).
    b32 shielded;
    b32 poisoned;
};

enum { ENTITY_HEALTH_STARTING_FILL_MAX = 3 };


enum EntityType {
    ENTITY_TYPE_NULL,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_WALL,
    ENTITY_TYPE_FAMILIAR,
    ENTITY_TYPE_MONSTER,
    ENTITY_TYPE_SWORD,
};


enum FaceDirection {
    FACE_DIRECTION_DOWN  = 0,
    FACE_DIRECTION_LEFT  = 1,
    FACE_DIRECTION_RIGHT = 2,
    FACE_DIRECTION_UP    = 3,
};


// @note: this union stores storage index when entity is stored in the storage,
// and pointer to the entity when entity loaded in simulation region.
struct SimEntity;
union EntityReference
{
    u32 index;
    SimEntity *ptr;
};

enum SimEntityFlags
{
    ENTITY_FLAG_COLLIDABLE = (1 << 0),
    ENTITY_FLAG_NONSPATIAL = (1 << 1),
};

struct SimEntity
{
    EntityType type;
    u32 storage_index;

    v3 position;
    v3 velocity;

    f32 tBob;
    FaceDirection face_direction;

    // @note: for "stairs"
    // i32 d_abs_tile_z;
    // i32 chunk_z; // for moving up and down "stairs"

    v3 hitbox;
    u32 flags;
    f32 distance_limit;
    f32 time_limit;

    i32 health_max;
    u32 health_fill_max;
    HealthPoint health[32];

    EntityReference sword;
};


struct SimEntityHashEntry
{
    u32 index;
    SimEntity *ptr;
};


struct SimRegion
{
    World *world;
    WorldPosition origin;
    Rectangle3 bounds;

    u32 entity_capacity;
    u32 entity_count;
    SimEntity *entities;

    // @note: hash table contains references for all entities inside sim region,
    // so you can get pointer to sim entity having storage index of that entity.
    SimEntityHashEntry hash_table[4096];
};


struct GameState;


// Find sim entity in the SimRegion
SimEntity *get_sim_entity(SimRegion *sim_region, u32 sim_index);

// Find sim entity in the SimRegion, and if it's not found, add entity to it and return resulted pointer
SimEntity *get_entity_by_storage_index(GameState *game_state, SimRegion *sim_region, u32 storage_index);

SimRegion *begin_simulation(GameState *game_state, Asuka::memory::arena_allocator *sim_arena, WorldPosition sim_origin, Rectangle3 sim_bounds);
void end_simulation(GameState *game_state, SimRegion *sim_region);


inline void set(SimEntity *entity, u32 flag)
{
    entity->flags |= flag;
}

inline void unset(SimEntity *entity, u32 flag)
{
    entity->flags &= (~flag);
}

inline b32 is(SimEntity *entity, u32 flag)
{
    b32 result = entity->flags & flag;
    return result;
}

inline void make_entity_nonspatial(SimEntity *entity)
{
    set(entity, ENTITY_FLAG_NONSPATIAL);
}

inline void make_entity_spatial(SimEntity *entity, v3 p, v3 v)
{
    unset(entity, ENTITY_FLAG_NONSPATIAL);
    entity->position = p;
    entity->velocity = v;
}


} // namespace Game
