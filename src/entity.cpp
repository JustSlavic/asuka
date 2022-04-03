#include "entity.hpp"


void move_entity(GameState *game_state, SimRegion *sim_region, SimEntity *entity, v3 acceleration, f32 dt);

void update_familiar(GameState *game_state, SimRegion *sim_region, SimEntity *entity, f32 dt) {
    SimEntity *closest_entity = NULL;
    u32 closest_entity_index = 0;
    f32 closest_distance_squared = math::square(7.0f); // @note: maximum following distance

    for (u32 index = 0; index < sim_region->entity_count; index++) {
        SimEntity *test_entity = get_sim_entity(sim_region, index);
        StoredEntity *low  = get_stored_entity(game_state, test_entity->storage_index);

        if (test_entity->type == ENTITY_TYPE_PLAYER) {
            f32 distance_squared = length_squared(test_entity->position - entity->position);
            if (distance_squared < closest_distance_squared) {
                closest_distance_squared = distance_squared;
                closest_entity_index = index;
                closest_entity = test_entity;
            }
        }
    }

    if (closest_entity)
    {
        if (closest_distance_squared > 2.0f) {
            f32 speed = 5;
            v3 nn_direction = (closest_entity->position - entity->position);
            v3 friction = -2.0f * entity->velocity;
            v3 acceleration = speed * nn_direction / math::sqrt(closest_distance_squared) + friction; // + gravity;

            move_entity(game_state, sim_region, entity, acceleration, dt);
        }
    }
}


void update_monster(GameState *game_state, SimEntity *entity, f32 dt)
{
}


void update_sword(GameState *game_state, SimRegion *sim_region, SimEntity *entity, f32 dt)
{
    if (is(entity, ENTITY_FLAG_NONSPATIAL))
    {
    }
    else
    {
        v3 old_position = entity->position;

        // @note: swords fly linearly, with no acceleration
        move_entity(game_state, sim_region, entity, V3(0, 0, 0), dt);

        f32 distance_traveled = length(entity->position - old_position);

        entity->sword_distance_remaining -= distance_traveled;
        if (entity->sword_distance_remaining < 0)
        {
            make_entity_nonspatial(entity);
        }
    }
}
