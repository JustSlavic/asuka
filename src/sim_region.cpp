#include "sim_region.hpp"


namespace Game {

SimEntity *get_sim_entity(SimRegion *sim_region, u32 index)
{
    ASSERT(index < sim_region->entity_count);

    SimEntity *result = sim_region->entities + index;
    return result;
}


INTERNAL
v3 map_to_sim_space_coordinates(SimRegion *sim_region, StoredEntity *entity) {
    // @todo: Do we want to set resulted position to signaling NaN in debug mode,
    // so that is anyone tries to use this value, it would throw exception right away.
    v3 result = position_difference(sim_region->world, entity->world_position, sim_region->origin);
    return result;
}


INTERNAL
SimEntity *add_entity_to_sim_region(SimRegion *sim_region)
{
    ASSERT(sim_region->entity_count < sim_region->entity_capacity);

    SimEntity *entity = sim_region->entities + sim_region->entity_count++;
    memory::set(entity, 0, sizeof(SimEntity));

    return entity;
}


INTERNAL
SimEntityHashEntry *get_hash_entry(SimRegion *sim_region, u32 storage_index)
{
    SimEntityHashEntry *result = NULL;
    for (u32 entry_offset = 0; entry_offset < ARRAY_COUNT(sim_region->hash_table); entry_offset++)
    {
        u32 hash_index = (storage_index + entry_offset) % ARRAY_COUNT(sim_region->hash_table);
        SimEntityHashEntry *entry = sim_region->hash_table + hash_index;
        if ((entry->index == 0) || (entry->index == storage_index))
        {
            result = entry;
            break;
        }
    }

    return result;
}

INTERNAL
SimEntity *add_entity_to_sim_region(GameState *game_state, SimRegion *sim_region, StoredEntity *stored, u32 storage_index, v2 *sim_position);

INTERNAL
void load_entity_reference(GameState *game_state, SimRegion *sim_region, EntityReference *ref)
{
    if (ref->index)
    {
        SimEntityHashEntry *entry = get_hash_entry(sim_region, ref->index);
        if (entry->ptr == 0)
        {
            StoredEntity *stored = get_stored_entity(game_state, ref->index);
            add_entity_to_sim_region(game_state, sim_region, stored, ref->index, NULL);
        }

        ref->ptr = entry->ptr;
    }
}


INTERNAL
SimEntity *add_entity_to_sim_region(GameState *game_state, SimRegion *sim_region, StoredEntity *stored, u32 storage_index, v2 *sim_position)
{
    ASSERT(storage_index > 0);

    SimEntity *entity = NULL;

    SimEntityHashEntry *entry = get_hash_entry(sim_region, storage_index);
    if (entry->index == 0)
    {
        entity = add_entity_to_sim_region(sim_region);
        if (entity)
        {
            // @note: Add entity into hash table immediately
            if (entry->index == 0)
            {
                // It might be that entity is already loaded because of somebody is have a reference to it.
                entry->index = storage_index;
                entry->ptr = entity;

                // @note: Decomression happens here.
                *entity = stored->sim;

                if (sim_position) {
                    entity->position.xy = *sim_position;
                } else {
                    entity->position = map_to_sim_space_coordinates(sim_region, stored);
                }

                load_entity_reference(game_state, sim_region, &entity->sword);
            }
        }
    }

    return entity;
}


StoredEntity *get_stored_entity(GameState *game_state, u32 index);


SimEntity *get_entity_by_storage_index(GameState *game_state, SimRegion *sim_region, u32 storage_index)
{
    SimEntity *result = NULL;

    if (storage_index)
    {
        SimEntityHashEntry *entry = get_hash_entry(sim_region, storage_index);
        if (entry->index == storage_index)
        {
            result = entry->ptr;
        }
    }

    return result;
}


SimRegion *begin_simulation(GameState *game_state, memory::arena_allocator *sim_arena, WorldPosition sim_origin, Rectangle3 sim_bounds)
{
    SimRegion *sim_region = ALLOCATE_STRUCT(sim_arena, SimRegion);
    sim_region->world  = game_state->world;
    sim_region->origin = sim_origin;
    sim_region->bounds = sim_bounds;

    // @todo: need to be more specific aboute entity counts
    sim_region->entity_capacity = 1024;
    sim_region->entity_count = 0;
    sim_region->entities = ALLOCATE_BUFFER(sim_arena, SimEntity, sim_region->entity_capacity);

    // Map stored entities into sim_space
    WorldPosition min_corner = map_into_world_space(game_state->world, sim_origin, sim_bounds.min);
    WorldPosition max_corner = map_into_world_space(game_state->world, sim_origin, sim_bounds.max);

    for (i32 chunk_z = min_corner.chunk.z; chunk_z <= max_corner.chunk.z; chunk_z++)
    {
        for (i32 chunk_y = min_corner.chunk.y; chunk_y <= max_corner.chunk.y; chunk_y++)
        {
            for (i32 chunk_x = min_corner.chunk.x; chunk_x <= max_corner.chunk.x; chunk_x++)
            {
                Chunk *chunk = get_chunk(game_state->world, chunk_x, chunk_y, sim_origin.chunk.z, &game_state->world_arena);

                if (chunk)
                {
                    for (EntityBlock *block = chunk->entities; block != NULL; block = block->next_block)
                    {
                        for (u32 i = 0; i < block->entity_count; i++)
                        {
                            u32 storage_index = block->entities[i];
                            StoredEntity *entity = get_stored_entity(game_state, storage_index);

                            if (!is(&entity->sim, ENTITY_FLAG_NONSPATIAL))
                            {
                                v3 sim_space_coordinates = map_to_sim_space_coordinates(sim_region, entity);
                                if (in_rectangle(sim_bounds, sim_space_coordinates))
                                {
                                    add_entity_to_sim_region(game_state, sim_region, entity, storage_index, &sim_space_coordinates.xy);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return sim_region;
}


INTERNAL
void unload_entity_reference(GameState *game_state, SimRegion *sim_region, EntityReference *ref)
{
    if (ref->ptr)
    {
        ref->index = ref->ptr->storage_index;
    }
}


INTERNAL
void store_entity_in_storage(GameState *game_state, SimRegion *sim_region, SimEntity *entity)
{
    // @todo: Compress entity into StoredEntity
    u32 storage_index = entity->storage_index;

    StoredEntity *stored = get_stored_entity(game_state, storage_index);
    unload_entity_reference(game_state, sim_region, &entity->sword);

    stored->sim = *entity;

    WorldPosition p;
    if (is(entity, ENTITY_FLAG_NONSPATIAL))
    {
        p = null_position();
    }
    else
    {
        p = map_into_world_space(game_state->world, sim_region->origin, entity->position);
    }

    change_entity_location(game_state->world, storage_index, stored, &p, &game_state->world_arena);
}


void end_simulation(GameState *game_state, SimRegion *sim_region)
{
    // Store sim entities into entity array in the world
    for (u32 sim_entity_index = 0; sim_entity_index < sim_region->entity_count; sim_entity_index++)
    {
        SimEntity *entity = sim_region->entities + sim_entity_index;
        store_entity_in_storage(game_state, sim_region, entity);
    }
}


} // namespace Game
