#include "asuka.hpp"
#include <math.hpp>
#include <debug/casts.hpp>

#define ASUKA_DEBUG_FOLLOWING_CAMERA 1


GLOBAL_VARIABLE v3 gravity = V3(0, 0, -9.8); // [m/s^2]


INTERNAL_FUNCTION
void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, GameState* game_state) {
    sound_sample_t* SampleOut = SoundBuffer->Samples;

    for (i32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
        u64 left_idx = (game_state->test_current_sound_cursor++) % game_state->test_wav_file.samples_count;
        u64 right_idx = (game_state->test_current_sound_cursor++) % game_state->test_wav_file.samples_count;

        sound_sample_t LeftSample =  game_state->test_wav_file.samples[left_idx];
        sound_sample_t RightSample = game_state->test_wav_file.samples[right_idx];

        f32 volume = 0.05f;

        *SampleOut++ = (sound_sample_t)(LeftSample  * volume);
        *SampleOut++ = (sound_sample_t)(RightSample * volume);
    }
}


INTERNAL_FUNCTION
void DrawBitmap(
    Game_OffscreenBuffer* buffer,
    f32 left, f32 top,
    Bitmap *image,
    f32 c_alpha = 1.0f)
{
    // @note: Top-down coordinate system.
    v2i tl = round_to_vector2i(V2(left, top));
    v2i br = tl + round_to_vector2i(V2(image->width, image->height));

    v2i image_tl = V2I(0, 0);
    v2i image_br = V2I(image->width, image->height);

    if (tl.x < 0) {
        image_tl.x = -tl.x;
        tl.x = 0;
    }
    if (tl.y < 0) {
        image_tl.y = -tl.y;
        tl.y = 0;
    }
    if (br.x > buffer->Width) {
        br.x = buffer->Width;
    }
    if (br.y > buffer->Height) {
        br.y = buffer->Height;
    }

    v2i dimensions = br - tl;
    v2i image_dims = image_br - image_tl;

    v2i dims {
        (dimensions.x < image_dims.x) ? dimensions.x : image_dims.x,
        (dimensions.y < image_dims.y) ? dimensions.y : image_dims.y,
    };

    u8* Row = (u8*)buffer->Memory + tl.y*buffer->Pitch + tl.x*buffer->BytesPerPixel;
    u8* image_pixel_row = (u8 *) image->pixels + image_tl.y * image->width * image->bytes_per_pixel + image_tl.x * image->bytes_per_pixel;

    for (int y = 0; y < dims.y; y++) {
        u32* Pixel = (u32*) Row;
        u8 * image_pixel = image_pixel_row;

        for (int x = 0; x < dims.x; x++) {
            u8 blue = image_pixel[0];
            u8 green = image_pixel[1];
            u8 red = image_pixel[2];
            u8 alpha = image_pixel[3];

            if (image->bytes_per_pixel == 1) {
                u8 chroma = *image_pixel;

                f32 c = chroma / 255.f;
                f32 a = alpha / 255.f;

                f32 back_r = (*Pixel & 0xFF) / 255.f;
                f32 back_g = ((*Pixel & 0xFF00) >> 8) / 255.f;
                f32 back_b = ((*Pixel & 0xFF0000) >> 16) / 255.f;
                f32 back_a = ((*Pixel & 0xFF000000) >> 24) / 255.f;

                f32 new_r = (1.0f - a) * back_r + a * c;
                f32 new_g = (1.0f - a) * back_g + a * c;
                f32 new_b = (1.0f - a) * back_b + a * c;

                *Pixel = (((u32)(new_r * 255.f)) << 0) |
                         (((u32)(new_g * 255.f)) << 8) |
                         (((u32)(new_b * 255.f)) << 16);
            } else if (image->bytes_per_pixel == 3) {
                *Pixel = (red) | (green << 8) | (blue << 16);
            } else if (image->bytes_per_pixel == 4) {
                f32 r = red / 255.f;
                f32 g = green / 255.f;
                f32 b = blue / 255.f;
                f32 a = (alpha / 255.f) * c_alpha;

                f32 back_r = (*Pixel & 0xFF) / 255.f;
                f32 back_g = ((*Pixel & 0xFF00) >> 8) / 255.f;
                f32 back_b = ((*Pixel & 0xFF0000) >> 16) / 255.f;
                f32 back_a = ((*Pixel & 0xFF000000) >> 24) / 255.f;

                f32 new_r = (1.0f - a) * back_r + a * r;
                f32 new_g = (1.0f - a) * back_g + a * g;
                f32 new_b = (1.0f - a) * back_b + a * b;

                *Pixel = (((u32)(new_r * 255.f)) << 0) |
                         (((u32)(new_g * 255.f)) << 8) |
                         (((u32)(new_b * 255.f)) << 16);
            }

            Pixel++;
            image_pixel += image->bytes_per_pixel;
        }

        Row += buffer->Pitch;
        image_pixel_row += image->width * image->bytes_per_pixel;
    }
}


INTERNAL_FUNCTION
void DrawRectangle(
    Game_OffscreenBuffer* buffer,
    v2 top_left, v2 bottom_right,
    color24 color,
    b32 stroke = false)
{
    v2i tl = round_to_vector2i(top_left);
    v2i br = round_to_vector2i(bottom_right);

    if (tl.x < 0) tl.x = 0;
    if (tl.y < 0) tl.y = 0;
    if (br.x > buffer->Width)  br.x = buffer->Width;
    if (br.y > buffer->Height) br.y = buffer->Height;

    v2i dimensions = br - tl;

    u8* Row = (u8*)buffer->Memory + tl.y*buffer->Pitch + tl.x*buffer->BytesPerPixel;

    for (int y = 0; y < dimensions.y; y++) {
        u32* Pixel = (u32*) Row;

        for (int x = 0; x < dimensions.x; x++) {
            if (stroke && (x == 0 || y == 0)) {
                *Pixel = 0;
            } else {
                *Pixel = pack_to_uint32(color);
            }
            Pixel++;
        }

        Row += buffer->Pitch;
    }
}


#if ASUKA_PLAYBACK_LOOP
INTERNAL_FUNCTION
void DrawBorder(Game_OffscreenBuffer* Buffer, u32 Width, color24 Color) {
    DrawRectangle(Buffer, V2(0, 0), V2(Buffer->Width, Width), Color);
    DrawRectangle(Buffer, V2(0, Width), V2(Width, Buffer->Height - Width), Color);
    DrawRectangle(Buffer, V2(Buffer->Width - Width, Width), V2(Buffer->Width, Buffer->Height - Width), Color);
    DrawRectangle(Buffer, V2(0, Buffer->Height - Width), V2(Buffer->Width, Buffer->Height), Color);
}
#endif

INTERNAL_FUNCTION
void push_piece(VisiblePieceGroup *group, v3 offset_in_meters, v2 dim_in_meters, Bitmap *bitmap, color32 color)
{
    // @note offset and dimensions are in world space (in meters, bottom-up coordinate space)
    ASSERT(group->count < ARRAY_COUNT(group->assets));

    VisiblePiece *asset = group->assets + (group->count++);
    memory::set(asset, 0, sizeof(VisiblePiece));

    asset->offset = V2(offset_in_meters.x, -(offset_in_meters.y + offset_in_meters.z)) * group->pixels_per_meter;
    asset->dimensions = dim_in_meters * group->pixels_per_meter;
    asset->bitmap = bitmap;
    asset->color = color;
}

INTERNAL_FUNCTION
void push_rectangle(VisiblePieceGroup *group, v3 offset_in_meters, v2 dim_in_meters, color32 color) {
    push_piece(group, offset_in_meters, dim_in_meters, NULL, color);
}

INTERNAL_FUNCTION
void push_asset(VisiblePieceGroup *group, Bitmap *bitmap, v3 offset_in_meters, f32 alpha = 1.0f) {
    push_piece(group, offset_in_meters, v2::zero(), bitmap, rgba(0, 0, 0, alpha));
}

INTERNAL_FUNCTION
void draw_hitpoints(LowEntity *low, VisiblePieceGroup *group) {
    f32 health_width   = 0.1f; // meters
    f32 health_spacing = 1.5f * health_width; // pixels

    f32 offset_x = -0.5f * health_spacing * (low->health_max - 1);
    f32 offset_y = 1.0f;

    for (i32 health_index = 0; health_index < low->health_max; health_index++) {
        HealthPoint hp = low->health[health_index];

        if (hp.fill > 0) {
            push_rectangle(group, V3(offset_x, offset_y, 0), V2(health_width), rgba(0.0f, 1.0f, 0.0f, 1.0f));
        } else {
            push_rectangle(group, V3(offset_x, offset_y, 0), V2(health_width), rgba(1.0f, 0.0f, 0.0f, 1.0f));
        }

        offset_x += health_spacing;
    }
}

INTERNAL_FUNCTION
LowEntity *get_low_entity(GameState *game_state, LowEntityIndex index) {
    ASSERT(index < ARRAY_COUNT(game_state->low_entities));

    LowEntity *result = NULL;
    if ((index > 0) && (index < game_state->low_entity_count)) {
        result = game_state->low_entities + index;
    }

    return result;
}


INTERNAL_FUNCTION
HighEntity *get_high_entity(GameState *game_state, HighEntityIndex index) {
    ASSERT(index < ARRAY_COUNT(game_state->high_entities));

    HighEntity *result = NULL;
    if ((index > 0) && (index < game_state->high_entity_count)) {
        result = game_state->high_entities + index;
    }

    return result;
}


INLINE
v2 get_camera_space_position(GameState *game_state, LowEntity *entity_low) {
    v2 result = position_difference(game_state->world, entity_low->world_position, game_state->camera_position);
    return result;
}


INTERNAL_FUNCTION
HighEntity *make_entity_high_frequency(GameState *game_state, LowEntity *entity_low, LowEntityIndex low_index, v2 camera_space_position) {
    ASSERT(entity_low->high_index == 0);
    ASSERT_MSG(game_state->high_entity_count < ARRAY_COUNT(game_state->high_entities), "Out of room for high frequency entities.");

    if (entity_low->type == ENTITY_TYPE_SWORD)
    {
        int debug_here = 0;
    }

    HighEntityIndex high_index = HighEntityIndex{ game_state->high_entity_count++ };
    HighEntity *entity_high = get_high_entity(game_state, high_index);

    entity_high->position.xy = camera_space_position;
    entity_high->velocity = v3::zero();
    entity_high->chunk_z = entity_low->world_position.chunk_z;
    entity_high->face_direction = FACE_DIRECTION_DOWN;
    entity_high->low_index = low_index;

    entity_low->high_index = high_index;

    return entity_high;
}


INTERNAL_FUNCTION
HighEntity *make_entity_high_frequency(GameState *game_state, LowEntityIndex low_index)
{
    LowEntity *entity_low = get_low_entity(game_state, low_index);

    HighEntity *entity_high = NULL;
    if (entity_low->high_index == 0) {
        v2 camera_space_position = get_camera_space_position(game_state, entity_low);
        entity_high = make_entity_high_frequency(game_state, entity_low, low_index, camera_space_position);
    } else {
        entity_high = get_high_entity(game_state, entity_low->high_index);
    }

    return entity_high;
}


INTERNAL_FUNCTION
void make_entity_low_frequency(GameState *game_state, LowEntityIndex low_index) {
    LowEntity *entity_low = get_low_entity(game_state, low_index);
    HighEntityIndex index_to_remove = entity_low->high_index;

    if (index_to_remove != 0) {
        HighEntity *entity_to_remove = get_high_entity(game_state, index_to_remove);
        HighEntityIndex last_high_index = HighEntityIndex{ game_state->high_entity_count - 1 };

        if (index_to_remove != last_high_index) {
            HighEntity *last_high_entity = get_high_entity(game_state, last_high_index);

            // Put last element in place of element that we remove from high entity table.
            *entity_to_remove = *last_high_entity;

            // Patch all low entities that were pointing at the last element so that they point into index_to_remove instead.
            for (LowEntityIndex index_to_patch { 1 }; index_to_patch < game_state->low_entity_count; index_to_patch++) {
                LowEntity *to_patch = get_low_entity(game_state, index_to_patch);
                if (to_patch->high_index == last_high_index) {
                    to_patch->high_index = index_to_remove;
                }
            }
        }

        entity_low->high_index = HighEntityIndex{ 0 };
        game_state->high_entity_count -= 1;
    }
}


INTERNAL_FUNCTION
LowEntityIndex add_low_entity(GameState *game_state, WorldPosition position) {
    ASSERT(game_state->low_entity_count < ARRAY_COUNT(game_state->low_entities));
    LowEntityIndex low_index { game_state->low_entity_count++ };

    LowEntity *low_entity = get_low_entity(game_state, low_index);
    ASSERT(low_entity);
    memory::set(low_entity, 0, sizeof(LowEntity));
    low_entity->world_position = null_position();

    change_entity_location(game_state->world, low_index, low_entity, &position, &game_state->world_arena);

    return low_index;
}


INTERNAL_FUNCTION
Entity get_entity(GameState *game_state, LowEntityIndex index) {
    LowEntity *low = get_low_entity(game_state, index);

    Entity entity {};

    if (low) {
        entity.low_index = index;
        entity.low = low;

        HighEntity *high = get_high_entity(game_state, low->high_index);
        if (high) {
            entity.high_index = low->high_index;
            entity.high = high;
        }
    }

    return entity;
}


INTERNAL_FUNCTION
Entity get_entity(GameState *game_state, HighEntityIndex index) {
    HighEntity *high = get_high_entity(game_state, index);

    Entity entity {};

    if (high) {
        entity.high_index = index;
        entity.high = high;

        LowEntity *low = get_low_entity(game_state, high->low_index);
        ASSERT(low);

        entity.low_index = high->low_index;
        entity.low = low;
    }

    return entity;
}


b32 is_valid(Entity entity) {
    b32 result = (entity.low_index != 0);
    return result;
}


b32 is_high(Entity entity)
{
    b32 result = (entity.high_index != 0) && (entity.high != NULL);
    return result;
}


INTERNAL_FUNCTION
void init_hitpoints(LowEntity *low, u32 health_max) {
    ASSERT(health_max < ARRAY_COUNT(low->health));

    low->health_max = health_max;
    low->health_fill_max = ENTITY_HEALTH_STARTING_FILL_MAX;
    for (i32 health_index = 0; health_index < low->health_max; health_index++) {
        low->health[health_index].fill = low->health_fill_max;
    }
}


INTERNAL_FUNCTION
Entity add_sword(GameState *game_state)
{
    LowEntityIndex index = add_low_entity(game_state, {});
    Entity entity = get_entity(game_state, index);
    entity.low->world_position = null_position();
    entity.low->type = ENTITY_TYPE_SWORD;
    entity.low->collidable = false;
    entity.low->hitbox = V2(0.4, 0.2);

    return entity;
}



INTERNAL_FUNCTION
Entity add_player(GameState *game_state) {
    WorldPosition position {};
    LowEntityIndex index = add_low_entity(game_state, position);

    Entity entity = get_entity(game_state, index);
    entity.low->type = ENTITY_TYPE_PLAYER;
    entity.low->world_position = position;
    // @todo: fix coordinates for hitbox
    entity.low->collidable = true;
    entity.low->hitbox = V2(0.8, 0.2); // In top-down coordinates, but in meters.

    Entity sword = add_sword(game_state);
    entity.low->sword_index = sword.low_index;

    init_hitpoints(entity.low, 3);
    make_entity_high_frequency(game_state, index);

    return entity;
}


INTERNAL_FUNCTION
Entity add_familiar(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v2 p) {
    WorldPosition position {};
    position.chunk_x = chunk_x;
    position.chunk_y = chunk_y;
    position.chunk_z = chunk_z;
    position.relative_position_in_chunk = p;

    LowEntityIndex index = add_low_entity(game_state, position);

    Entity entity = get_entity(game_state, index);
    entity.low->type = ENTITY_TYPE_FAMILIAR;
    entity.low->world_position = position;
    // @todo: fix coordinates for hitbox
    entity.low->collidable = false;
    entity.low->hitbox = V2(0.8, 0.2);
    entity.low->collidable = true;

    HighEntity *high = make_entity_high_frequency(game_state, index);
    high->position.z = 2.0f;

    return entity;
}


INTERNAL_FUNCTION
Entity add_monster(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v2 p) {
    WorldPosition position {};
    position.chunk_x = chunk_x;
    position.chunk_y = chunk_y;
    position.chunk_z = chunk_z;
    position.relative_position_in_chunk = p;

    LowEntityIndex index = add_low_entity(game_state, position);
    Entity entity = get_entity(game_state, index);

    entity.low->type = ENTITY_TYPE_MONSTER;
    entity.low->world_position = position;
    entity.low->collidable = true;
    entity.low->hitbox = V2(2.2, 2.2);

    init_hitpoints(entity.low, 7);

    return entity;
}


INTERNAL_FUNCTION
Entity add_wall(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v2 p) {
    WorldPosition position {};
    position.chunk_x = chunk_x;
    position.chunk_y = chunk_y;
    position.chunk_z = chunk_z;
    position.relative_position_in_chunk = p;

    LowEntityIndex index = add_low_entity(game_state, position);
    Entity entity = get_entity(game_state, index);

    entity.low->type = ENTITY_TYPE_WALL;
    entity.low->world_position = position;
    entity.low->collidable = true;
    entity.low->hitbox = V2(1, 0.4);

    return entity;
}


INTERNAL_FUNCTION
void move_entity(GameState *game_state, HighEntityIndex entity_index, v3 acceleration, f32 dt) {
    Entity entity = get_entity(game_state, entity_index);
    if (entity.high == NULL) {
        return;
    }

    v3 velocity = entity.high->velocity + acceleration * dt;
    v3 position = entity.high->position;

    v3 new_position = position + velocity * dt;

    if (new_position.z < 0) {
        velocity.z = 0;
        new_position.z = 0;
    }

    // ================= COLLISION DETECTION ====================== //

    v2 current_position = position.xy;
    v2 current_velocity = velocity.xy;
    v2 current_destination = new_position.xy;
    f32 time_spent_moving = 0.0f;

    f32 original_move_distance = length(new_position - position);

    const int ASUKA_MAX_MOVE_TRIES = 5;
    for (i32 move_try = 0; move_try < ASUKA_MAX_MOVE_TRIES; move_try++) {
        v2 closest_destination = current_destination;
        v2 velocity_at_closest_destination = current_velocity;

        HighEntityIndex hit_entity_index { 0 };

        for (HighEntityIndex test_index { 1 }; test_index < game_state->high_entity_count; test_index++) {
            if (test_index == entity_index) continue;

            Entity test_entity = get_entity(game_state, test_index);

            if ((test_entity.low->world_position.chunk_z != entity.low->world_position.chunk_z) ||
                (test_entity.low->collidable == false) || (entity.low->collidable == false))
            {
                continue;
            }

            f32 minkowski_test_width  = 0.5f * (test_entity.low->hitbox.x + entity.low->hitbox.x);
            f32 minkowski_test_height = 0.5f * (test_entity.low->hitbox.y + entity.low->hitbox.y);

            v2 vertices[4] = {
                v2{ test_entity.high->position.x - minkowski_test_width, test_entity.high->position.y + minkowski_test_height },
                v2{ test_entity.high->position.x + minkowski_test_width, test_entity.high->position.y + minkowski_test_height },
                v2{ test_entity.high->position.x + minkowski_test_width, test_entity.high->position.y - minkowski_test_height },
                v2{ test_entity.high->position.x - minkowski_test_width, test_entity.high->position.y - minkowski_test_height },
            };

            for (i32 vertex_idx = 0; vertex_idx < ARRAY_COUNT(vertices); vertex_idx++) {
                v2 w0 = vertices[vertex_idx];
                v2 w1 = vertices[(vertex_idx + 1) % ARRAY_COUNT(vertices)];

                v2 wall = w1 - w0;
                v2 normal = normalized(V2(-wall.y, wall.x));

                if (dot(current_velocity, normal) < 0) {
                    auto res = segment_segment_intersection(w0, w1, current_position, current_destination);


                    if (res.found == INTERSECTION_COLLINEAR) {
                        if (length_squared(current_destination - current_position) < length_squared(closest_destination - current_position)) {
                            closest_destination = current_destination;
                            velocity_at_closest_destination = project(current_velocity, wall);
                        }
                    }

                    if (res.found == INTERSECTION_FOUND) {
                        // @todo: What if we collide with several entities during move tries?
                        hit_entity_index = test_index;

                        // @note: Update only closest point.
                        if (length_squared(res.intersection - current_position) < length_squared(closest_destination - current_position)) {
                            // @todo: Make sliding better.
                            // @hack: Step out 3*eps from the wall to allow sliding along corners.
                            closest_destination = res.intersection + 4 * EPSILON * normal;
                            velocity_at_closest_destination = project(current_velocity, wall); // wall * math::dot(current_velocity, wall) / wall.length_2();
                        }
                    }
                }
            }
        }

        // @note: this have to be calculated before we change current position.
        f32 move_distance = length(closest_destination - current_position);

        current_position = closest_destination;
        if (hit_entity_index > 0) {
            if (move_distance > EPSILON) {
                f32 dt_prime = move_distance / length(current_velocity);
                time_spent_moving += dt_prime;
            }

            current_velocity = velocity_at_closest_destination;
            current_destination = current_position + current_velocity * (dt - time_spent_moving);

            Entity hit_entity = get_entity(game_state, hit_entity_index);
            entity.high->chunk_z += hit_entity.low->d_abs_tile_z;
        } else {
            break;
        }
    }

    entity.high->position.xy = current_position;
    entity.high->velocity.xy = current_velocity;

    // @temporary
    // @todo: make it go through collision detection by making collision detection 3d ?
    entity.high->position.z = new_position.z;
    entity.high->velocity.z = velocity.z;

    // @note: always write back a valid tile position
    auto new_world_position = map_into_world_space(game_state->world, game_state->camera_position, entity.high->position.xy);
    change_entity_location(game_state->world, entity.high->low_index, entity.low, &new_world_position, &game_state->world_arena);

    entity.low->world_position = new_world_position;
}


void update_familiar(GameState *game_state, Entity entity, f32 dt) {
    Entity closest_player {};
    f32 closest_distance_squared = math::square(5.0f); // @note: maximum following distance

    for (HighEntityIndex index { 1 }; index < game_state->high_entity_count; index++) {
        Entity test_entity = get_entity(game_state, index);

        if (test_entity.low->type == ENTITY_TYPE_PLAYER) {
            f32 distance_squared = length_squared(test_entity.high->position - entity.high->position);
            if (distance_squared < closest_distance_squared) {
                closest_distance_squared = distance_squared;
                closest_player = test_entity;
            }
        }
    }

    if (closest_player.high && closest_distance_squared > 2.0f) {
        f32 speed = 5;
        v3 nn_direction = (closest_player.high->position - entity.high->position);
        v3 friction = -2.0f * entity.high->velocity;
        v3 acceleration = speed * nn_direction / math::sqrt(closest_distance_squared) + friction + gravity;

        move_entity(game_state, entity.high_index, acceleration, dt);
    }
}


void update_monster(GameState *game_state, Entity entity, f32 dt) {

}

void update_sword(GameState *game_state, Entity entity, f32 dt)
{
    ASSERT(is_high(entity));

    v3 old_position = entity.high->position;
    move_entity(game_state, entity.high_index, v3::zero(), dt);

    f32 distance_traveled = length(entity.high->position - old_position);
    entity.low->sword_distance_remaining -= distance_traveled;
    if (entity.low->sword_distance_remaining < 0)
    {
        change_entity_location(game_state->world, entity.low_index, entity.low, NULL, &game_state->world_arena);
    }
}


INTERNAL_FUNCTION
void set_camera_position(GameState *game_state, WorldPosition new_camera_position) {
    auto *world = game_state->world;
    v2 diff = position_difference(world, game_state->camera_position, new_camera_position);

    if (!is_canonical(world, new_camera_position))
    {
        new_camera_position = canonicalize_position(world, new_camera_position);
    }

    game_state->camera_position = new_camera_position;

    v2i   high_zone_in_chunks = V2I(4, 3);
    Rectangle2 high_zone_in_meters = Rectangle2::from_center_dim(v2::zero(), world->chunk_side_in_meters * upcast_to_vector2(high_zone_in_chunks));

    WorldPosition min_corner = map_into_world_space(world, new_camera_position, high_zone_in_meters.min);
    WorldPosition max_corner = map_into_world_space(world, new_camera_position, high_zone_in_meters.max);

    // @note, that the process is separated into two loops, because we have to make room for more high entities by removing old ones first.
    // If we would do it in a one big loop, we would have to make high_entities[] array twice as big.

    // First, remove entities that are out of high simulation range from high entity set.
    for (HighEntityIndex index { 1 }; index < game_state->high_entity_count;) {
        Entity entity = get_entity(game_state, index);

        // @note: change position to be
        v2 new_position = entity.high->position.xy + diff;

        if (!is_valid(entity.low->world_position) ||
            (!in_rectangle(high_zone_in_meters, new_position) || (entity.low->world_position.chunk_z != new_camera_position.chunk_z)))
        {
            osOutputDebugString("Remove entity %d (%d, %d) from high entity set.\n", entity.high->low_index.index, entity.low->world_position.chunk_x, entity.low->world_position.chunk_y);

            ASSERT(entity.low->high_index == index);
            make_entity_low_frequency(game_state, entity.high->low_index);
        } else {
            entity.high->position.xy = new_position;
            index++;
        }
    }

    for (i32 chunk_y = min_corner.chunk_y; chunk_y <= max_corner.chunk_y; chunk_y++) {
        for (i32 chunk_x = min_corner.chunk_x; chunk_x <= max_corner.chunk_x; chunk_x++) {
            Chunk *chunk = get_chunk(world, chunk_x, chunk_y, new_camera_position.chunk_z, &game_state->world_arena);
            if (chunk) {
                for (EntityBlock *block = chunk->entities; block != NULL; block = block->next_block) {
                    for (u32 i = 0; i < block->entity_count; i++) {
                        LowEntityIndex entity_index = block->entities[i];

                        LowEntity *entity_low = get_low_entity(game_state, entity_index);
                        if (entity_low->high_index == 0) {
                            auto camera_space_position = get_camera_space_position(game_state, entity_low);
                            if (in_rectangle(high_zone_in_meters, camera_space_position)) {
                                osOutputDebugString("Move entity %d into high entity set.\n", entity_index.index);
                                make_entity_high_frequency(game_state, entity_low, entity_index, camera_space_position);
                            }
                        }
                    }
                }
            }
        }
    }
}


// Random
#include <time.h>
#include <stdlib.h>

GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    ASSERT(sizeof(GameState) <= Memory->PermanentStorageSize);

    f32 dt = Input->dt;

    GameState* game_state = (GameState*)Memory->PermanentStorage;

    i32 room_width_in_tiles = 16;
    i32 room_height_in_tiles = 9;

    // ===================== INITIALIZATION ===================== //

    if (!Memory->IsInitialized) {
        srand((unsigned int)time(NULL));

        // @note: reserve entity slot for the null entity
        game_state->low_entity_count  = 1;
        game_state->high_entity_count = 1;

        game_state->test_wav_file = load_wav_file("piano2.wav");
        game_state->test_current_sound_cursor = 0;

        game_state->grass_texture       = load_png_file("grass_texture.png");
        game_state->tree_texture        = load_png_file("tree_60x100.png");
        game_state->heart_full_texture  = load_png_file("heart_full.png");
        game_state->heart_empty_texture = load_png_file("heart_empty.png");
        game_state->monster_head        = load_png_file("monster_head.png");
        game_state->monster_left_arm    = load_png_file("monster_left_arm.png");
        game_state->monster_right_arm   = load_png_file("monster_right_arm.png");
        game_state->familiar_texture    = load_png_file("familiar.png");
        game_state->shadow_texture      = load_png_file("shadow.png");
        game_state->fireball_texture    = load_png_file("fireball.png");
        game_state->sword_texture       = load_png_file("sword.png");

        game_state->player_textures[0]  = load_png_file("character_1.png");
        game_state->player_textures[1]  = load_png_file("character_2.png");
        game_state->player_textures[2]  = load_png_file("character_3.png");
        game_state->player_textures[3]  = load_png_file("character_4.png");

        memory::arena_allocator *arena  = &game_state->world_arena;

        initialize_arena(
            arena,
            (u8 *) Memory->PermanentStorage + sizeof(GameState),
            Memory->PermanentStorageSize - sizeof(GameState));

        f32 tile_side_in_meters = 1.0f;
        f32 chunk_side_in_meters = 5.0f;
        i32 chunk_side_in_tiles = math::truncate_to_int32(chunk_side_in_meters / tile_side_in_meters);

        World *world = push_struct(arena, World);
        initialize_world(world, tile_side_in_meters, chunk_side_in_meters);
        game_state->world = world;

        // ===================== WORLD GENERATION ===================== //

        i32 screen_count = 6;

        i32 screen_x = 0;
        i32 screen_y = 0;
        i32 screen_z = 0;

        i32 room_width_in_meters  = math::truncate_to_int32(room_width_in_tiles  * world->tile_side_in_meters);
        i32 room_height_in_meters = math::truncate_to_int32(room_height_in_tiles * world->tile_side_in_meters);

        enum gen_direction {
            GEN_NONE,
            GEN_UP,
            GEN_RIGHT,
            GEN_FLOOR_UP,
            GEN_FLOOR_DOWN,
            // ---------
            GEN_MAX,
        };

        gen_direction previous_choice = GEN_NONE;

        for (i32 screen_idx = 0; screen_idx < screen_count; screen_idx++) {
            gen_direction choice = GEN_NONE;

            while(true) {
                choice = (gen_direction)(rand() % GEN_MAX);

                if (choice == GEN_FLOOR_UP || choice == GEN_FLOOR_DOWN) continue;
                // if (choice == GEN_FLOOR_UP && previous_choice == GEN_FLOOR_DOWN) continue;
                // if (choice == GEN_FLOOR_DOWN && previous_choice == GEN_FLOOR_UP) continue;

                if (choice != GEN_NONE) break;
            };

            if (screen_idx + 1 == screen_count) {
                choice = GEN_NONE;
            }

            i32 max_tile_x = room_width_in_tiles / 2;
            i32 min_tile_x = -room_width_in_tiles / 2;
            i32 max_tile_y = room_height_in_tiles / 2;
            i32 min_tile_y = -room_height_in_tiles / 2;

            for (i32 tile_y = min_tile_y; tile_y <= max_tile_y; tile_y++) {
                for (i32 tile_x = min_tile_x; tile_x <= max_tile_x; tile_x++) {
                    i32 x = screen_x * room_width_in_tiles  + tile_x;
                    i32 y = screen_y * room_height_in_tiles + tile_y;
                    i32 z = screen_z;

                    Tile tile_value = TILE_FREE;

                    // bottom wall
                    if ((tile_y == min_tile_y) && (tile_x != 0)) {
                        tile_value = TILE_WALL;
                    }

                    // upper wall
                    if ((tile_y == max_tile_y) && (tile_x != 0)) {
                        tile_value = TILE_WALL;
                    }

                    // left wall
                    if ((tile_x == min_tile_x) && (tile_y != 0)) {
                        tile_value = TILE_WALL;
                    }

                    // right wall
                    if ((tile_x == max_tile_x) && (tile_y != 0)) {
                        tile_value = TILE_WALL;
                    }

                    if ((choice == GEN_FLOOR_UP || previous_choice == GEN_FLOOR_DOWN) && (tile_x == 6 && tile_y == 6)) {
                        tile_value = TILE_DOOR_UP;
                    }

                    if ((choice == GEN_FLOOR_DOWN || previous_choice == GEN_FLOOR_UP) && (tile_x == 7 && tile_y == 6)) {
                        tile_value = TILE_DOOR_DOWN;
                    }

                    if ((screen_idx == screen_count - 1) && (tile_x == room_width_in_tiles - 2) && (tile_y == room_height_in_tiles - 2)) {
                        tile_value = TILE_WIN;
                    }

                    // SetTileValue(world, x, y, z, tile_value, arena);
                    i32 chunk_x = x / chunk_side_in_tiles;
                    i32 chunk_y = y / chunk_side_in_tiles;
                    i32 chunk_z = z;

                    f32 relative_x = (x % chunk_side_in_tiles) * tile_side_in_meters; // - 0.5f * chunk_side_in_meters + 0.5f * tile_side_in_meters;
                    f32 relative_y = (y % chunk_side_in_tiles) * tile_side_in_meters; // - 0.5f * chunk_side_in_meters + 0.5f * tile_side_in_meters;

                    if (tile_value == TILE_WALL) {
                        add_wall(game_state, chunk_x, chunk_y, chunk_z, v2{ relative_x, relative_y });
                    }
                }
            }

            switch (choice) {
                case GEN_UP:
                    screen_y += 1;
                    break;
                case GEN_RIGHT:
                    screen_x += 1;
                    break;
                case GEN_FLOOR_UP:
                    screen_z += 1;
                    break;
                case GEN_FLOOR_DOWN:
                    screen_z -= 1;
                    break;
                default:
                    break;
            }

            previous_choice = choice;
        }


        WorldPosition camera_position {};
        // camera_position.relative_position_in_chunk = v2{ (f32)room_width_in_tiles, (f32)room_height_in_tiles } * world->tile_side_in_meters * 0.5f;

        // @note: Add a monster BEFORE calling set camera position so it could be brought to the high entity set before player appearing on the screen.
        add_monster(game_state, 0, 0, 0, V2(2, 1));
        for (int i = 0; i < 1; i++) {
            f32 x = -5.0f + i * 1.0f;
            add_familiar(game_state, 0, 0, 0, V2(x, 1));
        }

        set_camera_position(game_state, camera_position);

        Memory->IsInitialized = true;
    }

    World *world = game_state->world;

    f32 pixels_per_meter = 60.f; // [pixels/m]

    f32 character_speed = 2.5f; // [m/s]
    f32 character_mass = 80.0f; // [kg]

    f32 friction_coefficient = 1.5f;

    for (InputIndex ControllerIndex { 0 }; ControllerIndex < ARRAY_COUNT(Input->ControllerInputs); ControllerIndex++) {
        Game_ControllerInput* ControllerInput = GetControllerInput(Input, ControllerIndex);

        LowEntityIndex low_entity_index = game_state->player_index_for_controller[ControllerIndex.index];
        Entity player = get_entity(game_state, low_entity_index);

        if (player.low == NULL && player.high == NULL) {
            if (GetPressCount(ControllerInput->Start)) {
                player = add_player(game_state);

                if (game_state->player_index_for_controller[ControllerIndex.index] == 0) {
                    game_state->player_index_for_controller[ControllerIndex.index] = player.low_index;
                    game_state->index_of_entity_for_camera_to_follow = player.low_index;
                    game_state->index_of_controller_for_camera_to_follow = { ControllerIndex.index };
                }
            } else {
                continue;
            }
        }

        // if (GetPressCount(ControllerInput->Y)) {
        //     game_state->index_of_controller_for_camera_to_follow += 1;
        //     if (game_state->index_of_controller_for_camera_to_follow.index >= ARRAY_COUNT(Input->ControllerInputs)) {
        //         game_state->index_of_controller_for_camera_to_follow = { 0 };
        //     }
        //     game_state->index_of_entity_for_camera_to_follow = game_state->player_index_for_controller[game_state->index_of_controller_for_camera_to_follow.index];
        //     game_state->camera_position.chunk_z = game_state->low_entities[game_state->index_of_entity_for_camera_to_follow.index].world_position.chunk_z;
        //     osOutputDebugString("Follow entity %d\n", game_state->index_of_entity_for_camera_to_follow.index);
        // }

        // if (GetPressCount(ControllerInput->X)) {
        //     WorldPosition *pos = &player.low->world_position;
        // }

        // ================= MOVEMENT SIMULATION ====================== //

        if (player.high == NULL) {
            // @todo: Handle multiple simulation regions for cooperative multiplayer.
            // @todo: Network cooperative multiplayer?
            continue;
        }

        v2 input_direction = normalized(ControllerInput->LeftStickEnded);
        f32 input_strength = math::clamp(length(ControllerInput->LeftStickEnded), 0, 1);

        f32 acceleration_coefficient = 100.0f; // [m/s^2]
        // if (ControllerInput->B.EndedDown) {
        //     acceleration_coefficient = 300.0f;
        // }

        v3 acceleration {};
        v3 friction_acceleration {};

        // @note: accelerate the guy only if he's standing on the ground.
        // if (player.high->position.z < EPSILON)
        {
            acceleration.xy = acceleration_coefficient * input_strength * input_direction;

            // [m/s^2] = [m/s] * [units] * [m/s^2]
            // @todo: why units do not add up?
            // @note: N = nu * g []
            friction_acceleration.xy = (player.high->velocity.xy) * friction_coefficient * gravity.z;
        }

        // @note: gravity works always, no matter where we are.
        acceleration += gravity; // [m/s^2] of gravity

        if (GetPressCount(ControllerInput->Start) > 0)
        {
            acceleration.z += 100.0f;
        }

        HighEntityIndex high_entity_index = player.low->high_index;

        v3 sword_speed {};
        if (GetPressCount(ControllerInput->X))
        {
            sword_speed = V3(-1, 0, 0);
        }
        else if (GetPressCount(ControllerInput->Y))
        {
            sword_speed = V3(0, 1, 0);
        }
        else if (GetPressCount(ControllerInput->A))
        {
            sword_speed = V3(0, -1, 0);
        }
        else if (GetPressCount(ControllerInput->B))
        {
            sword_speed = V3(1, 0, 0);
        }

        move_entity(game_state, high_entity_index, acceleration + friction_acceleration, dt);

        if (sword_speed != v3::zero())
        {
            Entity sword = get_entity(game_state, player.low->sword_index);
            if (is_valid(sword))
            {
                WorldPosition new_position = player.low->world_position;

                change_entity_location(game_state->world, sword.low_index, sword.low, &new_position, &game_state->world_arena);

                HighEntity *sword_high = make_entity_high_frequency(game_state, sword.low_index);
                sword_high->position.z = 0.5f;
                sword_high->velocity = sword_speed * 4.0f;
                sword.low->sword_distance_remaining = 3.0f; // meters
            }
        }

        if (length_squared(input_direction) > 0) { // @todo: Why bothering getting length when can ask for equallity to zero
            if (math::absolute(input_direction.x) > math::absolute(input_direction.y)) {
                if (input_direction.x > 0) {
                    player.high->face_direction = FACE_DIRECTION_RIGHT;
                } else {
                    player.high->face_direction = FACE_DIRECTION_LEFT;
                }
            } else {
                if (input_direction.y > 0) {
                    player.high->face_direction = FACE_DIRECTION_UP;
                } else {
                    player.high->face_direction = FACE_DIRECTION_DOWN;
                }
            }
        }
    }

    Entity followed_entity = get_entity(game_state, game_state->index_of_entity_for_camera_to_follow);

    if (followed_entity.high != NULL) {
        v2 room_in_tiles  = V2(16, 9);
        Rectangle2 room_in_meters = Rectangle2::from_center_dim(v2::zero(), world->tile_side_in_meters * room_in_tiles);

        WorldPosition new_camera_position = game_state->camera_position;

#if ASUKA_DEBUG_FOLLOWING_CAMERA
        new_camera_position.relative_position_in_chunk += followed_entity.high->position.xy;
#else
        if (followed_entity.high->position.x > room_in_meters.max.x) {
            new_camera_position.relative_position_in_chunk.x += get_width(room_in_meters);
        }

        if (followed_entity.high->position.x < room_in_meters.min.x) {
            new_camera_position.relative_position_in_chunk.x -= get_width(room_in_meters);
        }

        if (followed_entity.high->position.y > room_in_meters.max.y) {
            new_camera_position.relative_position_in_chunk.y += get_height(room_in_meters);
        }

        if (followed_entity.high->position.y < room_in_meters.min.y) {
            new_camera_position.relative_position_in_chunk.y -= get_height(room_in_meters);
        }
#endif

        set_camera_position(game_state, new_camera_position);
    }

    // Game_OutputSound(SoundBuffer, game_state);

    // ===================== RENDERING ===================== //

    // Render pink background to see pixels I didn't drew.
    DrawRectangle(Buffer, V2(0, 0), V2(Buffer->Width, Buffer->Height), rgb(1.f, 0.f, 1.f));
    DrawRectangle(Buffer, V2(0, 0), V2(Buffer->Width, Buffer->Height), rgb(0.5, 0.5, 0.5));

    // Background grass
    // DrawBitmap(Buffer, { 0, 0 }, { (f32)Buffer->Width, (f32)Buffer->Height }, &game_state->grass_texture);

    // ===================== RENDERING ENTITIES ===================== //
    VisiblePieceGroup group {};
    group.pixels_per_meter = pixels_per_meter;

    for (HighEntityIndex entity_index { 0 }; entity_index < game_state->high_entity_count; entity_index++) {
        Entity entity = get_entity(game_state, entity_index);
        if (entity.high != NULL) {
            group.count = 0;

            if (entity.low->world_position.chunk_z != game_state->camera_position.chunk_z) {
                continue;
            }

            if (entity.low->type == ENTITY_TYPE_PLAYER) {
                auto *shadow_texture = &game_state->shadow_texture;
                push_asset(&group, shadow_texture, V3(-0.5f, 0.85f, 0), 1.0f / (1.0f + entity.high->position.z));

                auto *player_texture = &game_state->player_textures[entity.high->face_direction];
                push_asset(&group, player_texture, V3(-0.4f, 1.0f, entity.high->position.z));

                draw_hitpoints(entity.low, &group);
            }
            else if (entity.low->type == ENTITY_TYPE_FAMILIAR)
            {
                update_familiar(game_state, entity, dt);

                entity.high->tBob += dt;
                if (entity.high->tBob > 2 * math::pi) {
                    entity.high->tBob -= 2 * math::pi;
                }

                f32 a = 2.0f;
                f32 t = a * math::sin(3.0f * entity.high->tBob);
                f32 h = 2.0f / (2.0f + a + t);

                auto *shadow = &game_state->shadow_texture;
                push_asset(&group, shadow, V3(-0.5f, 0.85f, 0), h);

                auto *texture = &game_state->familiar_texture;
                push_asset(&group, texture, V3(-0.5f, 0.8f, 0.2f / h));
            }
            else if (entity.low->type == ENTITY_TYPE_MONSTER)
            {
                update_monster(game_state, entity, dt);

                auto *head = &game_state->monster_head;
                auto *left_arm  = &game_state->monster_left_arm;
                auto *right_arm = &game_state->monster_right_arm;

                push_asset(&group, head, V3(-2.5f, 2.5f, 0));
                push_asset(&group, left_arm, V3(-2.0f, 2.5f, 0));
                push_asset(&group, right_arm, V3(-3.0f, 2.5f, 0));

                draw_hitpoints(entity.low, &group);
            }
            else if (entity.low->type == ENTITY_TYPE_WALL)
            {
                auto *texture = &game_state->tree_texture;
                push_asset(&group, texture, V3(-0.5f, 1.6f, 0));
            }
            else if (entity.low->type == ENTITY_TYPE_SWORD)
            {
                update_sword(game_state, entity, dt);
                auto *texture = &game_state->sword_texture;
                auto *shadow_texture = &game_state->shadow_texture;

                // @todo: If I have to take into account position.z in here, therefore I should 
                push_asset(&group, shadow_texture, V3(-0.5, 0.85, 0));
                push_asset(&group, texture, V3(-0.4f, 0.2f, entity.high->position.z));
            }
            else
            {
                INVALID_CODE_PATH();
            }

            v2 entity_position_in_pixels =
                0.5f * V2(Buffer->Width, Buffer->Height) +
                V2(entity.high->position.x, -entity.high->position.y) * pixels_per_meter;

            f32 x = entity_position_in_pixels.x;
            f32 y = entity_position_in_pixels.y;

            // Hitbox rectangle
            // DrawRectangle(
            //     Buffer,
            //     entity_position_in_pixels - 0.5f * entity.low->hitbox * pixels_per_meter,
            //     entity_position_in_pixels + 0.5f * entity.low->hitbox * pixels_per_meter,
            //     color24{ 1.f, 1.f, 0.f });

            for (u32 asset_index = 0; asset_index < group.count; asset_index++) {
                auto *asset = &group.assets[asset_index];
                ASSERT(asset);

                v2 center = V2(x, y) + asset->offset;
                if (asset->bitmap) {
                    DrawBitmap(Buffer, center.x, center.y, asset->bitmap, asset->color.a);
                } else {
                    v2 half_dim = 0.5f * asset->dimensions;
                    DrawRectangle(Buffer, center - half_dim, center + half_dim, asset->color.rgb);
                }
            }
        }
    }

#if 0
    {
        v2i   high_zone_in_chunks = v2i { 2, 1 };
        Rectangle2 high_zone_in_meters = Rectangle2::from_center_dim(v2::zero(), world->chunk_side_in_meters * math::upcast_to_vector2(high_zone_in_chunks));

        v2 high_zone_center_in_meters = get_center(high_zone_in_meters);

        v2 screen_center_in_pixels = 0.5f * V2(Buffer->Width, Buffer->Height);
        v2 min_corner_in_pixels = screen_center_in_pixels - (high_zone_center_in_meters - high_zone_in_meters.min) * pixels_per_meter;
        v2 max_corner_in_pixels = screen_center_in_pixels + (high_zone_in_meters.max - high_zone_center_in_meters) * pixels_per_meter;

        DrawRectangle(Buffer, min_corner_in_pixels, V2(min_corner_in_pixels.x + 2, max_corner_in_pixels.y), rgb(1, 1, 0));
        DrawRectangle(Buffer, min_corner_in_pixels, V2(max_corner_in_pixels.x, min_corner_in_pixels.y + 2), rgb(1, 1, 0));
        DrawRectangle(Buffer, V2(min_corner_in_pixels.x, max_corner_in_pixels.y - 2), max_corner_in_pixels, rgb(1, 1, 0));
        DrawRectangle(Buffer, V2(max_corner_in_pixels.x - 2, min_corner_in_pixels.y), max_corner_in_pixels, rgb(1, 1, 0));
    }
#endif

    // ===================== RENDERING SIGNALING BORDERS ===================== //

#if ASUKA_PLAYBACK_LOOP
    color24 BorderColor {};
    u32 BorderWidth = 10;
    b32 BorderVisible {};

    switch (Input->PlaybackLoopState) {
        case PLAYBACK_LOOP_IDLE: {
            BorderVisible = false;
            break;
        }
        case PLAYBACK_LOOP_RECORDING: {
            BorderVisible = true;
            BorderColor = rgb(1.0f, 244.0f / 255.0f, 43.0f / 255.0f);
            break;
        }
        case PLAYBACK_LOOP_PLAYBACK: {
            BorderVisible = true;
            BorderColor = rgb(29.0f / 255.0f, 166.0f / 255.0f, 8.0f / 255.0f);
            break;
        }
    }

    if (BorderVisible) {
        DrawBorder(Buffer, BorderWidth, BorderColor);
    }
#endif // ASUKA_PLAYBACK_LOOP
}
