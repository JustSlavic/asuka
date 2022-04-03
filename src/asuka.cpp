#include "asuka.hpp"

#include <math.hpp>
#include <debug/casts.hpp>
#include <sim_region.hpp>
#include "entity.cpp"

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
    v2i tl = round_to_v2i(V2(left, top));
    v2i br = tl + round_to_v2i(V2(image->width, image->height));

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
    v2i tl = round_to_v2i(top_left);
    v2i br = round_to_v2i(bottom_right);

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
void draw_empty_rectangle_in_meters(Game_OffscreenBuffer *buffer, Rectangle2 rect, u32 width, color24 color, f32 pixels_per_meter)
{
    f32 rect_width = get_width(rect);
    f32 rect_height = get_height(rect);

    v2 screen_center_in_pixels = 0.5f * V2(buffer->Width, buffer->Height);

    v2 min_corner_in_pixels = screen_center_in_pixels - (get_center(rect) - rect.min) * pixels_per_meter;
    v2 max_corner_in_pixels = screen_center_in_pixels + (rect.max - get_center(rect)) * pixels_per_meter;

    DrawRectangle(buffer, min_corner_in_pixels, V2(max_corner_in_pixels.x, min_corner_in_pixels.y + width), color);
    DrawRectangle(buffer, V2(min_corner_in_pixels.x, min_corner_in_pixels.y + width), V2(min_corner_in_pixels.x + width, max_corner_in_pixels.y - width), color);
    DrawRectangle(buffer, V2(max_corner_in_pixels.x - width, min_corner_in_pixels.y + width), V2(max_corner_in_pixels.x, max_corner_in_pixels.y - width), color);
    DrawRectangle(buffer, V2(min_corner_in_pixels.x, max_corner_in_pixels.y - width), max_corner_in_pixels, color);
}


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
void draw_hitpoints(SimEntity *entity, VisiblePieceGroup *group) {
    f32 health_width   = 0.1f; // meters
    f32 health_spacing = 1.5f * health_width; // pixels

    f32 offset_x = -0.5f * health_spacing * (entity->health_max - 1);
    f32 offset_y = 1.0f;

    for (i32 health_index = 0; health_index < entity->health_max; health_index++) {
        HealthPoint hp = entity->health[health_index];

        if (hp.fill > 0) {
            push_rectangle(group, V3(offset_x, offset_y, 0), V2(health_width), rgba(0.0f, 1.0f, 0.0f, 1.0f));
        } else {
            push_rectangle(group, V3(offset_x, offset_y, 0), V2(health_width), rgba(1.0f, 0.0f, 0.0f, 1.0f));
        }

        offset_x += health_spacing;
    }
}


struct EntityResult
{
    StoredEntity *entity;
    u32 index;
};


INTERNAL_FUNCTION
EntityResult add_entity(GameState *game_state, WorldPosition position = null_position()) {
    ASSERT(game_state->entity_count < ARRAY_COUNT(game_state->entities));

    EntityResult result {};
    result.index = game_state->entity_count++;
    result.entity = get_stored_entity(game_state, result.index);

    ASSERT(result.entity);

    memory::set(result.entity, 0, sizeof(StoredEntity));
    result.entity->world_position = null_position();
    result.entity->sim.storage_index = result.index;

    change_entity_location(game_state->world, result.index, result.entity, &position, &game_state->world_arena);

    return result;
}


INTERNAL_FUNCTION
void init_hitpoints(StoredEntity *entity, u32 health_max) {
    ASSERT(health_max < ARRAY_COUNT(entity->sim.health));

    entity->sim.health_max = health_max;
    entity->sim.health_fill_max = ENTITY_HEALTH_STARTING_FILL_MAX;
    for (i32 health_index = 0; health_index < entity->sim.health_max; health_index++) {
        entity->sim.health[health_index].fill = entity->sim.health_fill_max;
    }
}


INTERNAL_FUNCTION
EntityResult add_sword(GameState *game_state)
{
    EntityResult result = add_entity(game_state);

    result.entity->sim.type = ENTITY_TYPE_SWORD;
    result.entity->sim.hitbox = V2(0.4, 0.2);
    set(&result.entity->sim, ENTITY_FLAG_NONSPATIAL);

    return result;
}


INTERNAL_FUNCTION
EntityResult add_player(GameState *game_state) {
    EntityResult result = add_entity(game_state, world_origin());

    result.entity->sim.type = ENTITY_TYPE_PLAYER;
    set(&result.entity->sim, ENTITY_FLAG_COLLIDABLE);

    // // @todo: fix coordinates for hitbox
    result.entity->sim.hitbox = V2(0.8, 0.2); // In top-down coordinates, but in meters.

    EntityResult sword_ = add_sword(game_state);
    result.entity->sim.sword.index = sword_.index;

    init_hitpoints(result.entity, 3);

    return result;
}


INTERNAL_FUNCTION
EntityResult add_familiar(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v2 p) {
    WorldPosition position = world_position(game_state->world, chunk_x, chunk_y, chunk_z, p);

    EntityResult result = add_entity(game_state, position);
    result.entity->sim.type = ENTITY_TYPE_FAMILIAR;
    // // @todo: fix coordinates for hitbox
    result.entity->sim.hitbox = V2(0.8, 0.2);
    set(&result.entity->sim, ENTITY_FLAG_COLLIDABLE);

    return result;
}


INTERNAL_FUNCTION
EntityResult add_monster(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v2 p) {
    WorldPosition position = world_position(game_state->world, chunk_x, chunk_y, chunk_z, p);

    EntityResult result = add_entity(game_state, position);
    result.entity->sim.type = ENTITY_TYPE_MONSTER;
    result.entity->world_position = position;
    result.entity->sim.hitbox = V2(2.2, 2.2);
    set(&result.entity->sim, ENTITY_FLAG_COLLIDABLE);

    init_hitpoints(result.entity, 7);

    return result;
}


INTERNAL_FUNCTION
EntityResult add_wall(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v2 p)
{
    WorldPosition position = world_position(game_state->world, chunk_x, chunk_y, chunk_z, p);

    EntityResult result = add_entity(game_state, position);
    result.entity->sim.type = ENTITY_TYPE_WALL;
    result.entity->world_position = position;
    set(&result.entity->sim, ENTITY_FLAG_COLLIDABLE);
    result.entity->sim.hitbox = V2(1, 0.4);

    return result;
}


void move_entity(GameState *game_state, SimRegion *sim_region, SimEntity *entity, v3 acceleration, f32 dt)
{
    ASSERT(!is(entity, ENTITY_FLAG_NONSPATIAL));
    // p = p0 + v0 * t + a * t^2 / 2

    v3 position = entity->position;
    v3 velocity = entity->velocity + acceleration * dt;

    v3 new_position = position + velocity * dt;

    // v3 position = entity->position;
    // v3 velocity = entity->velocity;

    // v3 new_velocity = velocity + acceleration * dt;
    // v3 new_position = position + new_velocity * dt;
    // v3 new_velocity = velocity + acceleration * dt;
    // v3 new_position = position + velocity * dt + 0.5f * acceleration * math::square(dt);

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

        b32 hit_happened = false;
        u32 hit_entity_index = 0;

        if (is(entity, ENTITY_FLAG_COLLIDABLE) && !is(entity, ENTITY_FLAG_NONSPATIAL))
        {
            for (u32 test_index = 0; test_index < sim_region->entity_count; test_index++) {
                SimEntity *test_entity = get_sim_entity(sim_region, test_index);

                if (test_entity->storage_index == entity->storage_index)
                {
                    continue;
                }

                if (is(test_entity, ENTITY_FLAG_COLLIDABLE) && !is(test_entity, ENTITY_FLAG_NONSPATIAL))
                {
                    f32 minkowski_test_width  = 0.5f * (test_entity->hitbox.x + entity->hitbox.x);
                    f32 minkowski_test_height = 0.5f * (test_entity->hitbox.y + entity->hitbox.y);

                    v2 vertices[4] = {
                        v2{ test_entity->position.x - minkowski_test_width, test_entity->position.y + minkowski_test_height },
                        v2{ test_entity->position.x + minkowski_test_width, test_entity->position.y + minkowski_test_height },
                        v2{ test_entity->position.x + minkowski_test_width, test_entity->position.y - minkowski_test_height },
                        v2{ test_entity->position.x - minkowski_test_width, test_entity->position.y - minkowski_test_height },
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
                                hit_happened = true;

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
            }
        }

        // @note: this have to be calculated before we change current position.
        f32 move_distance = length(closest_destination - current_position);

        current_position = closest_destination;
        if (hit_happened) {
            if (move_distance > EPSILON) {
                f32 dt_prime = move_distance / length(current_velocity);
                time_spent_moving += dt_prime;
            }

            current_velocity = velocity_at_closest_destination;
            current_destination = current_position + current_velocity * (dt - time_spent_moving);

            SimEntity *hit_entity = get_sim_entity(sim_region, hit_entity_index);
            // @todo: Do something with "hit_entity" and "entity", like, register hit or something.
        } else {
            break;
        }
    }

    entity->position.xy = current_position;
    entity->velocity.xy = current_velocity;

    // @temporary
    // @todo: make it go through collision detection by making collision detection 3d ?
    entity->position.z = new_position.z;
    entity->velocity.z = velocity.z;
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

    if (!Memory->IsInitialized)
    {
        srand((unsigned int)time(NULL));

        // @note: reserve entity slot for the null entity
        game_state->entity_count  = 1;

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

        initialize_arena(arena, (u8 *) Memory->PermanentStorage + sizeof(GameState), Memory->PermanentStorageSize - sizeof(GameState));

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

        // @note: Add a monster BEFORE calling set camera position so it could be brought to the high entity set before player appearing on the screen.
        add_monster(game_state, 0, 0, 0, V2(2, 1));
        for (int i = 0; i < 1; i++) {
            f32 x = -5.0f + i * 1.0f;
            add_familiar(game_state, 0, 0, 0, V2(x, 1));
        }

        Memory->IsInitialized = true;
    }


    World *world = game_state->world;

    Rectangle2 sim_bounds = Rectangle2::from_center_dim(V2(0, 0), V2(20, 12)); // in meters

    initialize_arena(&game_state->sim_arena, (u8 *) Memory->TransientStorage, Memory->TransientStorageSize);
    SimRegion *sim_region = begin_simulation(game_state, &game_state->sim_arena, game_state->camera_position, sim_bounds);

    f32 pixels_per_meter = 60.f; // [pixels/m]

    f32 character_speed = 2.5f; // [m/s]
    f32 character_mass = 80.0f; // [kg]

    f32 friction_coefficient = 1.5f;

    for (InputIndex ControllerIndex { 0 }; ControllerIndex < ARRAY_COUNT(Input->ControllerInputs); ControllerIndex++)
    {
        Game_ControllerInput* ControllerInput = GetControllerInput(Input, ControllerIndex);

        u32 entity_index = game_state->player_index_for_controller[ControllerIndex.index];
        SimEntity *player = get_entity_by_storage_index(game_state, sim_region, entity_index);

        if (player == NULL)
        {
            if (GetPressCount(ControllerInput->Start))
            {
                EntityResult added_player = add_player(game_state);
                player = add_entity_to_sim_region(game_state, sim_region, added_player.entity, added_player.index, 0);
                //player = find_or_add_stored_entity_to_sim_region(game_state, sim_region, added_player.index);

                if (game_state->player_index_for_controller[ControllerIndex.index] == 0)
                {
                    game_state->player_index_for_controller[ControllerIndex.index] = added_player.index;
                    game_state->index_of_entity_for_camera_to_follow = added_player.index;
                    game_state->index_of_controller_for_camera_to_follow = ControllerIndex.index;
                }
            }

            continue;
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

        if (player)
        {
            // @todo: Handle multiple simulation regions for cooperative multiplayer.
            // @todo: Network cooperative multiplayer?

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
                friction_acceleration.xy = (player->velocity.xy) * friction_coefficient * gravity.z;
            }

            // @note: gravity works always, no matter where we are.
            acceleration += gravity; // [m/s^2] of gravity

            if (GetPressCount(ControllerInput->Start) > 0)
            {
                acceleration.z += 100.0f;
            }

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

            // @todo: Update player ?
            move_entity(game_state, sim_region, player, acceleration + friction_acceleration, dt);

            if (sword_speed != v3::zero())
            {
                SimEntity *sword = player->sword.ptr;
                if (sword)
                {
                    make_entity_spatial(sword, player->position.xy, sword_speed.xy * 4.0f);
                    sword->position.z = 0.5f;
                    sword->sword_distance_remaining = 3.0f; // meters
                }
            }

            if (length_squared(input_direction) > 0) { // @todo: Why bothering getting length when can ask for equallity to zero
                if (math::absolute(input_direction.x) > math::absolute(input_direction.y))
                {
                    if (input_direction.x > 0)
                    {
                        player->face_direction = FACE_DIRECTION_RIGHT;
                    }
                    else
                    {
                        player->face_direction = FACE_DIRECTION_LEFT;
                    }
                }
                else
                {
                    if (input_direction.y > 0)
                    {
                        player->face_direction = FACE_DIRECTION_UP;
                    }
                    else
                    {
                        player->face_direction = FACE_DIRECTION_DOWN;
                    }
                }
            }
        }
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

    for (u32 sim_entity_index = 0; sim_entity_index < sim_region->entity_count; sim_entity_index++) {
        SimEntity *entity = get_sim_entity(sim_region, sim_entity_index);

        group.count = 0;

        if (entity->type == ENTITY_TYPE_PLAYER) {
            auto *shadow_texture = &game_state->shadow_texture;
            push_asset(&group, shadow_texture, V3(-0.5f, 0.85f, 0), 1.0f / (1.0f + entity->position.z));

            auto *player_texture = &game_state->player_textures[entity->face_direction];
            push_asset(&group, player_texture, V3(-0.4f, 1.0f, entity->position.z));

            draw_hitpoints(entity, &group);
        }
        else if (entity->type == ENTITY_TYPE_FAMILIAR)
        {
            update_familiar(game_state, sim_region, entity, dt);

            entity->tBob += dt;
            if (entity->tBob > 2 * math::pi) {
                entity->tBob -= 2 * math::pi;
            }

            f32 a = 2.0f;
            f32 t = a * math::sin(3.0f * entity->tBob);
            f32 h = 2.0f / (2.0f + a + t);

            auto *shadow = &game_state->shadow_texture;
            push_asset(&group, shadow, V3(-0.5f, 0.85f, 0), h);

            auto *texture = &game_state->familiar_texture;
            push_asset(&group, texture, V3(-0.5f, 0.8f, 0.2f / h));
        }
        else if (entity->type == ENTITY_TYPE_MONSTER)
        {
            update_monster(game_state, entity, dt);

            auto *head = &game_state->monster_head;
            auto *left_arm  = &game_state->monster_left_arm;
            auto *right_arm = &game_state->monster_right_arm;

            push_asset(&group, head, V3(-2.5f, 2.5f, 0));
            push_asset(&group, left_arm, V3(-2.0f, 2.5f, 0));
            push_asset(&group, right_arm, V3(-3.0f, 2.5f, 0));

            draw_hitpoints(entity, &group);
        }
        else if (entity->type == ENTITY_TYPE_WALL)
        {
            auto *texture = &game_state->tree_texture;
            push_asset(&group, texture, V3(-0.5f, 1.6f, 0));
        }
        else if (entity->type == ENTITY_TYPE_SWORD)
        {
            update_sword(game_state, sim_region, entity, dt);
            auto *texture = &game_state->sword_texture;
            auto *shadow_texture = &game_state->shadow_texture;

            // @todo: If I have to take into account position.z in here, therefore I should
            push_asset(&group, shadow_texture, V3(-0.5, 0.85, 0), 1.0f / (1.0f + entity->position.z));
            push_asset(&group, texture, V3(-0.4f, 0.2f, entity->position.z));
        }
        else
        {
            INVALID_CODE_PATH();
        }

        v2 entity_position_in_pixels =
            0.5f * V2(Buffer->Width, Buffer->Height) +
            V2(entity->position.x, -entity->position.y) * pixels_per_meter;

        f32 x = entity_position_in_pixels.x;
        f32 y = entity_position_in_pixels.y;

        // Hitbox rectangle
        // DrawRectangle(
        //     Buffer,
        //     entity_position_in_pixels - 0.5f * entity->hitbox * pixels_per_meter,
        //     entity_position_in_pixels + 0.5f * entity->hitbox * pixels_per_meter,
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

    // Change camera location and end the simulation
    StoredEntity *followed_entity = get_stored_entity(game_state, game_state->index_of_entity_for_camera_to_follow);
    if (followed_entity) {
        v2 room_in_tiles  = V2(16, 9);
        Rectangle2 room_in_meters = Rectangle2::from_center_dim(v2::zero(), world->tile_side_in_meters * room_in_tiles);

        WorldPosition new_camera_position = game_state->camera_position;

#if ASUKA_DEBUG_FOLLOWING_CAMERA
        new_camera_position = followed_entity->world_position;
#else
        if (followed_entity.high->position.x > room_in_meters.max.x) {
            new_camera_position.offset.x += get_width(room_in_meters);
        }

        if (followed_entity.high->position.x < room_in_meters.min.x) {
            new_camera_position.offset.x -= get_width(room_in_meters);
        }

        if (followed_entity.high->position.y > room_in_meters.max.y) {
            new_camera_position.offset.y += get_height(room_in_meters);
        }

        if (followed_entity.high->position.y < room_in_meters.min.y) {
            new_camera_position.offset.y -= get_height(room_in_meters);
        }
#endif

        game_state->camera_position = new_camera_position;
    }

    end_simulation(game_state, sim_region);

#if 1
    {
        // draw_empty_rectangle_in_meters(Buffer, sim_bounds, 2, rgb(1, 1, 0), pixels_per_meter);

        Rectangle2 debug_rect_to_draw = Rectangle2::from_center_dim(V2(0, 0), V2(1, 1));
        draw_empty_rectangle_in_meters(Buffer, debug_rect_to_draw, 2, rgb(0, 1, 0), pixels_per_meter);
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
