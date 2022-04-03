#pragma once


inline void set(SimEntity *entity, u32 flag)
{
    entity->flags = (entity->flags | flag);
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

inline void make_entity_spatial(SimEntity *entity, v2 p, v2 v)
{
    unset(entity, ENTITY_FLAG_NONSPATIAL);
    entity->position.xy = p;
    entity->velocity.xy = v;
}
