#include "asuka.hpp"
#include <math.hpp>
#include <debug/casts.hpp>

#define ASUKA_DEBUG_FOLLOWING_CAMERA 0


INTERNAL_FUNCTION
void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, GameState* game_state) {
    sound_sample_t* SampleOut = SoundBuffer->Samples;

    for (int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
        uint64 left_idx = (game_state->test_current_sound_cursor++) % game_state->test_wav_file.samples_count;
        uint64 right_idx = (game_state->test_current_sound_cursor++) % game_state->test_wav_file.samples_count;

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
    math::v2 top_left, math::v2 bottom_right,
    Bitmap *image,
    bool32 stroke = false)
{
    using math::vector2i;
    using math::vector2;

    // @note: Top-down coordinate system.
    vector2i tl = round_to_vector2i(top_left);
    vector2i br = round_to_vector2i(bottom_right);

    vector2i image_tl = vector2i{ 0, 0 };
    vector2i image_br = vector2i{ (i32)image->width, (i32)image->height };

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

    vector2i dimensions = br - tl;
    vector2i image_dims = image_br - image_tl;

    vector2i dims {
        (dimensions.x < image_dims.x) ? dimensions.x : image_dims.x,
        (dimensions.y < image_dims.y) ? dimensions.y : image_dims.y,
    };

    uint8* Row = (uint8*)buffer->Memory + tl.y*buffer->Pitch + tl.x*buffer->BytesPerPixel;
    uint8* image_pixel_row = (uint8 *) image->pixels + image_tl.y * image->width * image->bytes_per_pixel + image_tl.x * image->bytes_per_pixel;

    for (int y = 0; y < dims.y; y++) {
        uint32* Pixel = (uint32*) Row;
        uint8 * image_pixel = image_pixel_row;

        for (int x = 0; x < dims.x; x++) {
            uint8 blue = image_pixel[0];
            uint8 green = image_pixel[1];
            uint8 red = image_pixel[2];
            uint8 alpha = image_pixel[3];

            if (image->bytes_per_pixel == 1) {
                uint8 chroma = *image_pixel;

                f32 c = chroma / 255.f;
                f32 a = alpha / 255.f;

                f32 back_r = (*Pixel & 0xFF) / 255.f;
                f32 back_g = ((*Pixel & 0xFF00) >> 8) / 255.f;
                f32 back_b = ((*Pixel & 0xFF0000) >> 16) / 255.f;
                f32 back_a = ((*Pixel & 0xFF000000) >> 24) / 255.f;

                f32 new_r = (1.0f - a) * back_r + a * c;
                f32 new_g = (1.0f - a) * back_g + a * c;
                f32 new_b = (1.0f - a) * back_b + a * c;

                *Pixel = (((uint32)(new_r * 255.f)) << 0) |
                         (((uint32)(new_g * 255.f)) << 8) |
                         (((uint32)(new_b * 255.f)) << 16);
            } else if (image->bytes_per_pixel == 3) {
                *Pixel = (red) | (green << 8) | (blue << 16);
            } else if (image->bytes_per_pixel == 4) {
                f32 r = red / 255.f;
                f32 g = green / 255.f;
                f32 b = blue / 255.f;
                f32 a = alpha / 255.f;

                f32 back_r = (*Pixel & 0xFF) / 255.f;
                f32 back_g = ((*Pixel & 0xFF00) >> 8) / 255.f;
                f32 back_b = ((*Pixel & 0xFF0000) >> 16) / 255.f;
                f32 back_a = ((*Pixel & 0xFF000000) >> 24) / 255.f;

                f32 new_r = (1.0f - a) * back_r + a * r;
                f32 new_g = (1.0f - a) * back_g + a * g;
                f32 new_b = (1.0f - a) * back_b + a * b;

                *Pixel = (((uint32)(new_r * 255.f)) << 0) |
                         (((uint32)(new_g * 255.f)) << 8) |
                         (((uint32)(new_b * 255.f)) << 16);
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
    math::v2 top_left, math::v2 bottom_right,
    math::color24 color,
    bool32 stroke = false)
{
    using math::vector2i;
    using math::vector2;

    vector2i tl = round_to_vector2i(top_left);
    vector2i br = round_to_vector2i(bottom_right);

    if (tl.x < 0) tl.x = 0;
    if (tl.y < 0) tl.y = 0;
    if (br.x > buffer->Width)  br.x = buffer->Width;
    if (br.y > buffer->Height) br.y = buffer->Height;

    vector2i dimensions = br - tl;

    uint8* Row = (uint8*)buffer->Memory + tl.y*buffer->Pitch + tl.x*buffer->BytesPerPixel;

    for (int y = 0; y < dimensions.y; y++) {
        uint32* Pixel = (uint32*) Row;

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
void DrawBorder(Game_OffscreenBuffer* Buffer, uint32 Width, math::color24 Color) {
    DrawRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Width}, Color);
    DrawRectangle(Buffer, {0, (float32)Width}, {(float32)Width, (float32)Buffer->Height - Width}, Color);
    DrawRectangle(Buffer, {(float32)Buffer->Width - Width, (float32)Width}, {(float32)Buffer->Width, (float32)Buffer->Height - Width}, Color);
    DrawRectangle(Buffer, {0, (float32)Buffer->Height - Width}, {(float32)Buffer->Width, (float32)Buffer->Height}, Color);
}
#endif


INTERNAL_FUNCTION
LowFrequencyEntity *get_low_entity(GameState *game_state, LowEntityIndex index) {
    ASSERT(index < ARRAY_COUNT(game_state->low_entities));

    LowFrequencyEntity *result = NULL;

    if (index > 0) {
        result = game_state->low_entities + index;
    }

    return result;
}


INTERNAL_FUNCTION
HighFrequencyEntity *get_high_entity(GameState *game_state, HighEntityIndex index) {
    ASSERT(index < ARRAY_COUNT(game_state->high_entities));

    HighFrequencyEntity *result = NULL;

    if (index > 0) {
        result = game_state->high_entities + index;
    }

    return result;
}


INTERNAL_FUNCTION
void make_entity_high_frequency(GameState *game_state, LowEntityIndex low_index) {
    LowFrequencyEntity *entity_low = get_low_entity(game_state, low_index);

    if (entity_low->high_index == 0) {
        ASSERT_MSG(game_state->high_entity_count < ARRAY_COUNT(game_state->high_entities), "Out of room for high frequency entities.");

        HighEntityIndex high_index = HighEntityIndex{ game_state->high_entity_count++ };
        HighFrequencyEntity *entity_high = get_high_entity(game_state, high_index);

        // @bug: PositionDifference return zero when Z tilemap coordinate differs. This makes mapping from low frequency to high frequency wrong.
        math::v2 diff = PositionDifference(game_state->world, entity_low->world_position, game_state->camera_position);
        entity_high->position.xy = diff;
        entity_high->velocity = math::v3::zero();
        entity_high->absolute_tile_z = entity_low->world_position.absolute_tile_z;
        entity_high->face_direction = FACE_DIRECTION_DOWN;
        entity_high->low_index = low_index;

        entity_low->high_index = high_index;
    }
}


INTERNAL_FUNCTION
void make_entity_low_frequency(GameState *game_state, LowEntityIndex low_index) {
    LowFrequencyEntity *entity_low = get_low_entity(game_state, low_index);
    HighEntityIndex index_to_remove = entity_low->high_index;

    if (index_to_remove != 0) {
        HighFrequencyEntity *entity_to_remove = get_high_entity(game_state, index_to_remove);
        HighEntityIndex last_high_index = HighEntityIndex{ game_state->high_entity_count - 1 };

        if (index_to_remove != last_high_index) {
            HighFrequencyEntity *last_high_entity = get_high_entity(game_state, last_high_index);

            // Put last element in place of element that we remove from high entity table.
            *entity_to_remove = *last_high_entity;

            // Patch all low entities that were pointing at the last element so that they point into index_to_remove instead.
            for (LowEntityIndex index_to_patch { 1 }; index_to_patch < game_state->low_entity_count; index_to_patch++) {
                LowFrequencyEntity *to_patch = get_low_entity(game_state, index_to_patch);
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
LowEntityIndex add_low_entity(GameState *game_state) {
    LowEntityIndex low_index { game_state->low_entity_count++ };

    ASSERT(game_state->low_entity_count < ARRAY_COUNT(game_state->low_entities));

    LowFrequencyEntity *low_entity = get_low_entity(game_state, low_index);
    if (low_entity) {
        *low_entity = {};
    }

    return low_index;
}


INTERNAL_FUNCTION
Entity get_entity(GameState *game_state, LowEntityIndex index) {
    Entity entity {};

    ASSERT(index < ARRAY_COUNT(game_state->low_entities));

    entity.low = get_low_entity(game_state, index);
    if (entity.low) {
        entity.high = get_high_entity(game_state, entity.low->high_index);
    }

    return entity;
}


INTERNAL_FUNCTION
Entity get_entity(GameState *game_state, HighEntityIndex index) {
    Entity entity {};

    ASSERT(index < ARRAY_COUNT(game_state->high_entities));

    entity.high = get_high_entity(game_state, index);
    if (entity.high) {
        entity.low = get_low_entity(game_state, entity.high->low_index);
    }

    return entity;
}


INTERNAL_FUNCTION
Entity initialize_player(GameState *game_state, LowEntityIndex index) {
    Entity entity = get_entity(game_state, index);

    entity.low->type = ENTITY_TYPE_PLAYER;
    entity.low->world_position.absolute_tile_x = 3;
    entity.low->world_position.absolute_tile_y = 3;
    entity.low->world_position.absolute_tile_z = 0;
    entity.low->world_position.relative_position_on_tile = math::v2{};

    // @todo: fix coordinates for hitbox
    entity.low->collidable = true;
    entity.low->hitbox = math::v2 { 0.8f, 0.4f }; // In top-down coordinates, but in meters.

    make_entity_high_frequency(game_state, index);

    return entity;
}


INTERNAL_FUNCTION
LowEntityIndex add_player(GameState *game_state) {
    LowEntityIndex index = add_low_entity(game_state);
    Entity player = initialize_player(game_state, index);

    return index;
}


INTERNAL_FUNCTION
Entity add_wall(GameState *game_state, int32 abs_tile_x, int32 abs_tile_y, int32 abs_tile_z) {
    LowEntityIndex index = add_low_entity(game_state);
    Entity entity = get_entity(game_state, index);

    entity.low->type = ENTITY_TYPE_WALL;
    entity.low->world_position.absolute_tile_x = abs_tile_x;
    entity.low->world_position.absolute_tile_y = abs_tile_y;
    entity.low->world_position.absolute_tile_z = abs_tile_z;
    entity.low->world_position.relative_position_on_tile = math::v2{};

    entity.low->collidable = true;
    entity.low->hitbox = math::v2 { .50f, .50f };

    return entity;
}


INTERNAL_FUNCTION
void move_entity(GameState *game_state, HighEntityIndex entity_index, math::v3 acceleration, float32 dt) {
    using math::v2;
    using math::v3;

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

    f32 original_move_distance = (new_position - position).length();

    const int ASUKA_MAX_MOVE_TRIES = 5;
    for (int32 move_try = 0; move_try < ASUKA_MAX_MOVE_TRIES; move_try++) {
        v2 closest_destination = current_destination;
        v2 velocity_at_closest_destination = current_velocity;

        HighEntityIndex hit_entity_index { 0 };

        for (HighEntityIndex test_index { 1 }; test_index < game_state->high_entity_count; test_index++) {
            if (test_index == entity_index) continue;

            Entity test_entity = get_entity(game_state, test_index);

            if ((test_entity.low->world_position.absolute_tile_z != entity.low->world_position.absolute_tile_z) ||
                (test_entity.low->collidable == false) || (entity.low->collidable == false))
            {
                continue;
            }

            float32 minkowski_test_width  = 0.5f * (test_entity.low->hitbox.x + entity.low->hitbox.x);
            float32 minkowski_test_height = 0.5f * (test_entity.low->hitbox.y + entity.low->hitbox.y);

            v2 vertices[4] = {
                v2{ test_entity.high->position.x - minkowski_test_width, test_entity.high->position.y + minkowski_test_height },
                v2{ test_entity.high->position.x + minkowski_test_width, test_entity.high->position.y + minkowski_test_height },
                v2{ test_entity.high->position.x + minkowski_test_width, test_entity.high->position.y - minkowski_test_height },
                v2{ test_entity.high->position.x - minkowski_test_width, test_entity.high->position.y - minkowski_test_height },
            };

            for (int32 vertex_idx = 0; vertex_idx < ARRAY_COUNT(vertices); vertex_idx++) {
                v2 w0 = vertices[vertex_idx];
                v2 w1 = vertices[(vertex_idx + 1) % ARRAY_COUNT(vertices)];

                v2 wall = w1 - w0;
                v2 normal = v2{ -wall.y, wall.x }.normalized();

                if (math::dot(current_velocity, normal) < 0) {
                    auto res = segment_segment_intersection(w0, w1, current_position, current_destination);


                    if (res.found == math::INTERSECTION_COLLINEAR) {
                        if ((current_destination - current_position).length_2() < (closest_destination - current_position).length_2()) {
                            closest_destination = current_destination;
                            velocity_at_closest_destination = math::project(current_velocity, wall);
                        }
                    }

                    if (res.found == math::INTERSECTION_FOUND) {
                        // @todo: What if we collide with several entities during move tries?
                        hit_entity_index = test_index;

                        // @note: Update only closest point.
                        if ((res.intersection - current_position).length_2() < (closest_destination - current_position).length_2()) {
                            // @todo: Make sliding better.
                            // @hack: Step out 3*eps from the wall to allow sliding along corners.
                            closest_destination = res.intersection + 4 * EPSILON * normal;
                            velocity_at_closest_destination = math::project(current_velocity, wall); // wall * math::dot(current_velocity, wall) / wall.length_2();
                        }
                    }
                }
            }
        }

        // @note: this have to be calculated before we change current position.
        f32 move_distance = (closest_destination - current_position).length();

        current_position = closest_destination;
        if (hit_entity_index > 0) {
            if (move_distance > EPSILON) {
                f32 dt_prime = move_distance / current_velocity.length();
                time_spent_moving += dt_prime;
            }

            current_velocity = velocity_at_closest_destination;
            current_destination = current_position + current_velocity * (dt - time_spent_moving);

            Entity hit_entity = get_entity(game_state, hit_entity_index);
            entity.high->absolute_tile_z += hit_entity.low->d_abs_tile_z;
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
    auto tmp = map_into_tile_space(game_state->world, game_state->camera_position, entity.high->position.xy);
    entity.low->world_position.absolute_tile_x = tmp.absolute_tile_x;
    entity.low->world_position.absolute_tile_y = tmp.absolute_tile_y;
    entity.low->world_position.relative_position_on_tile = tmp.relative_position_on_tile;
}


INTERNAL_FUNCTION
void SetCameraPosition(GameState *game_state, AbsoluteWorldPosition new_camera_position) {
    auto *world = game_state->world;
    math::vector2 diff = PositionDifference(world, game_state->camera_position, new_camera_position);
    game_state->camera_position = new_camera_position;

    math::vector2i   high_zone_in_tiles = math::vector2i { 16 * 3, 9 * 3 };
    math::rectangle2 high_zone_in_float = math::rectangle2::from_center_dim(math::v2::zero(), world->tile_side_in_meters * math::upcast_to_vector2(high_zone_in_tiles));

    i32 min_tile_x = new_camera_position.absolute_tile_x - high_zone_in_tiles.x / 2 + 1;
    i32 max_tile_x = new_camera_position.absolute_tile_x + high_zone_in_tiles.x / 2 - 1;
    i32 min_tile_y = new_camera_position.absolute_tile_y - high_zone_in_tiles.y / 2 + 1;
    i32 max_tile_y = new_camera_position.absolute_tile_y + high_zone_in_tiles.y / 2 - 1;

    // @note, that the process is separated into two loops, because we have to make room for more high entities by removing old ones first.
    // If we would do it in a one big loop, we would have to make high_entities[] array twice as big.

    // First, remove entities that are out of high simulation range from high entity set.
    for (HighEntityIndex index { 1 }; index < game_state->high_entity_count;) {
        Entity entity = get_entity(game_state, index);

        math::v2 new_position = entity.high->position.xy + diff;
        if (!in_rectangle(high_zone_in_float, new_position) || (entity.low->world_position.absolute_tile_z != new_camera_position.absolute_tile_z)) {
            osOutputDebugString("Remove entity %d from high entity set.\n", entity.high->low_index.index);
            make_entity_low_frequency(game_state, entity.high->low_index);
        } else {
            entity.high->position.xy = new_position;
            index++;
        }
    }

    // Second, add entities that are in high simulation range into high entity set.
    for (LowEntityIndex index { 1 }; index < game_state->low_entity_count; index++) {
        Entity entity = get_entity(game_state, index);
        if (entity.high == 0) {
            if ((entity.low->world_position.absolute_tile_z == new_camera_position.absolute_tile_z) &&
                (entity.low->world_position.absolute_tile_x >= min_tile_x) &&
                (entity.low->world_position.absolute_tile_x <= max_tile_x) &&
                (entity.low->world_position.absolute_tile_y >= min_tile_y) &&
                (entity.low->world_position.absolute_tile_y <= max_tile_y))
            {
                osOutputDebugString("Move entity %d into high entity set.\n", index.index);
                make_entity_high_frequency(game_state, index);
            }
        }
    }
}


// Random
#include <time.h>
#include <stdlib.h>

GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    using math::v2;
    using math::v3;
    using math::color24;

    ASSERT(sizeof(GameState) <= Memory->PermanentStorageSize);

    float32 dt = Input->dt;

    GameState* game_state = (GameState*)Memory->PermanentStorage;

    i32 room_width_in_tiles = 16;
    i32 room_height_in_tiles = 9;

    // ===================== INITIALIZATION ===================== //

    if (!Memory->IsInitialized) {
        srand((unsigned int)time(NULL));

        // @note: reserve entity slot for the null entity
        add_low_entity(game_state);
        game_state->high_entity_count = 1;

        const char *wav_filename = "piano2.wav";
        game_state->test_wav_file = load_wav_file(wav_filename);
        game_state->test_current_sound_cursor = 0;

        const char *floor_texture_filename = "tile_16x16.png";
        game_state->floor_texture = load_png_file_myself(floor_texture_filename);

        const char *grass_texture_filename = "grass_texture.png";
        game_state->grass_texture = load_png_file(grass_texture_filename);

        const char *wall_texture_filename = "tile_60x60.bmp";
        game_state->wall_texture = load_bmp_file(wall_texture_filename);

        char *heart_full_texture_filename = "heart_full.png";
        game_state->heart_full_texture = load_png_file(heart_full_texture_filename);

        char *heart_empty_texture_filename = "heart_empty.png";
        game_state->heart_empty_texture = load_png_file(heart_empty_texture_filename);

        const char *player_face_texture_filename = "character_1.png";
        const char *player_left_texture_filename = "character_2.png";
        const char *player_right_texture_filename = "character_3.png";
        const char *player_back_texture_filename = "character_4.png";

        game_state->player_textures[0] = load_png_file(player_face_texture_filename);
        game_state->player_textures[1] = load_png_file(player_left_texture_filename);
        game_state->player_textures[2] = load_png_file(player_right_texture_filename);
        game_state->player_textures[3] = load_png_file(player_back_texture_filename);

        MemoryArena *arena = &game_state->world_arena;

        initialize_arena(
            arena,
            (uint8 *) Memory->PermanentStorage + sizeof(GameState),
            Memory->PermanentStorageSize - sizeof(GameState));

        game_state->world = push_struct(arena, World);
        World *world = game_state->world;

        // ===================== WORLD GENERATION ===================== //

        world->tile_side_in_meters = 1.0f; // [meters]

        // Tilechunks 256x256
        world->chunk_shift = 3;
        world->chunk_mask  = (1 << world->chunk_shift) - 1;
        world->tile_count_x = 1 << world->chunk_shift;
        world->tile_count_y = 1 << world->chunk_shift;

        i32 screen_count = 200;

        i32 screen_x = 0;
        i32 screen_y = 0;
        i32 screen_z = 0;

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

            for (i32 tile_y = 0; tile_y < room_height_in_tiles; tile_y++) {
                for (i32 tile_x = 0; tile_x < room_width_in_tiles; tile_x++) {
                    u32 x = screen_x * room_width_in_tiles  + tile_x;
                    u32 y = screen_y * room_height_in_tiles + tile_y;
                    u32 z = screen_z;

                    Tile tile_value = TILE_FREE;

                    // bottom wall
                    if (tile_y == 0) {
                        tile_value = TILE_WALL;

                        if ((previous_choice == GEN_UP) && (tile_x == room_width_in_tiles / 2 || tile_x == (room_width_in_tiles / 2 - 1))) {
                            tile_value = TILE_FREE;
                        }
                    }

                    // upper wall
                    if (tile_y == room_height_in_tiles - 1) {
                        tile_value = TILE_WALL;

                        if ((choice == GEN_UP) && (tile_x == room_width_in_tiles / 2 || tile_x == (room_width_in_tiles / 2 - 1))) {
                            tile_value = TILE_FREE;
                        }
                    }

                    // left wall
                    if (tile_x == 0) {
                        tile_value = TILE_WALL;

                        if ((previous_choice == GEN_RIGHT && (tile_y == room_height_in_tiles / 2))) {
                            tile_value = TILE_FREE;
                        }
                    }

                    // right wall
                    if (tile_x == room_width_in_tiles - 1) {
                        tile_value = TILE_WALL;

                        if ((choice == GEN_RIGHT && (tile_y == room_height_in_tiles / 2))) {
                            tile_value = TILE_FREE;
                        }
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
                    if (tile_value == TILE_WALL)  add_wall(game_state, x, y, z);
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

        AbsoluteWorldPosition camera_position;
        camera_position.absolute_tile_x = room_width_in_tiles / 2;
        camera_position.absolute_tile_y = room_height_in_tiles / 2;
        camera_position.absolute_tile_z = 0;
        camera_position.relative_position_on_tile = { -0.5f, 0.0f }; // @note: Offset because the center of the room lies on the edge between tiles.
        SetCameraPosition(game_state, camera_position);

        Memory->IsInitialized = true;
    }

    World *world = game_state->world;

    float32 pixels_per_meter = 60.f; // [pixels/m]

    v2  character_dimensions = { 0.75f, 1.0f }; // [m; m]
    f32 character_speed = 2.5f; // [m/s]
    f32 character_mass = 80.0f; // [kg]
    v3 gravity = { 0.0f, 0.0f, -9.8f }; // [m/s^2]

    f32 friction_coefficient = 1.5f;

    for (uint32 ControllerIndex = 0; ControllerIndex < ARRAY_COUNT(Input->ControllerInputs); ControllerIndex++) {
        Game_ControllerInput* ControllerInput = GetControllerInput(Input, ControllerIndex);

        LowEntityIndex low_entity_index = game_state->player_index_for_controller[ControllerIndex];
        Entity entity = get_entity(game_state, low_entity_index);

        if (entity.low == NULL && entity.high == NULL) {
            if (GetPressCount(ControllerInput->Start)) {
                LowEntityIndex player_index = add_player(game_state);
                entity = get_entity(game_state, player_index);

                if (game_state->player_index_for_controller[ControllerIndex] == 0) {
                    game_state->player_index_for_controller[ControllerIndex] = player_index;
                    game_state->index_of_entity_for_camera_to_follow = player_index;
                    game_state->index_of_controller_for_camera_to_follow = { ControllerIndex };
                }
            } else {
                continue;
            }
        }

        if (GetPressCount(ControllerInput->Y)) {
            game_state->index_of_controller_for_camera_to_follow += 1;
            if (game_state->index_of_controller_for_camera_to_follow.index >= ARRAY_COUNT(Input->ControllerInputs)) {
                game_state->index_of_controller_for_camera_to_follow = { 0 };
            }
            game_state->index_of_entity_for_camera_to_follow = game_state->player_index_for_controller[game_state->index_of_controller_for_camera_to_follow.index];
            game_state->camera_position.absolute_tile_z = game_state->low_entities[game_state->index_of_entity_for_camera_to_follow.index].world_position.absolute_tile_z;
            osOutputDebugString("Follow entity %d\n", game_state->index_of_entity_for_camera_to_follow.index);
        }

        if (GetPressCount(ControllerInput->X)) {
            AbsoluteWorldPosition *pos = &entity.low->world_position;
            // Tile tile_value = GetTileValue(world, pos->absolute_tile_x, pos->absolute_tile_y, pos->absolute_tile_z);

            // if (tile_value == TILE_DOOR_UP) {
            //     AbsoluteWorldPosition move_up_position = *pos;
            //     move_up_position.absolute_tile_z += 1;

            //     // bool up_tile_empty = IsWorldPointEmpty(world, move_up_position);
            //     if (up_tile_empty) {
            //         pos->absolute_tile_z += 1;
            //         Entity followed_entity = get_entity(game_state, game_state->index_of_entity_for_camera_to_follow);
            //         game_state->camera_position.absolute_tile_z = followed_entity.low->world_position.absolute_tile_z;
            //         osOutputDebugString("Entity %d go upstairs\n", low_entity_index.index);
            //     }
            // }

            // if (tile_value == TILE_DOOR_DOWN) {
            //     AbsoluteWorldPosition move_down_position = *pos;
            //     move_down_position.absolute_tile_z -= 1;

            //     // bool down_tile_empty = IsWorldPointEmpty(world, move_down_position);
            //     if (down_tile_empty) {
            //         pos->absolute_tile_z -= 1;
            //         Entity followed_entity = get_entity(game_state, game_state->index_of_entity_for_camera_to_follow);
            //         game_state->camera_position.absolute_tile_z = followed_entity.low->world_position.absolute_tile_z;
            //         osOutputDebugString("Entity %d go downstairs\n", low_entity_index.index);
            //     }
            // }
        }

        // ================= MOVEMENT SIMULATION ====================== //

        if (entity.high == NULL) {
            // @todo: Handle multiple simulation regions for cooperative multiplayer.
            // @todo: Network cooperative multiplayer?
            continue;
        }

        v2 input_direction = ControllerInput->LeftStickEnded.normalized();
        f32 input_strength = math::clamp(ControllerInput->LeftStickEnded.norm(), 0, 1);

        float32 acceleration_coefficient = 100.0f; // [m/s^2]
        if (ControllerInput->B.EndedDown) {
            acceleration_coefficient = 300.0f;
        }

        math::v3 acceleration {};
        math::v3 friction_acceleration {};

        // @note: accelerate the guy only if he's standing on the ground.
        if (entity.high->position.z < EPSILON) {
            acceleration.xy = acceleration_coefficient * input_strength * input_direction;

            // [m/s^2] = [m/s] * [units] * [m/s^2]
            // @todo: why units do not add up?
            friction_acceleration.xy = (entity.high->velocity.xy) * friction_coefficient * gravity.z;
        }

        // @note: gravity works always, no matter where we are.
        acceleration += gravity; // [m/s^2] of gravity

        if (GetHoldsCount(ControllerInput->A) > 0 && entity.high->position.z < EPSILON) {
            acceleration.z += 100.0f;
        }

        HighEntityIndex high_entity_index = entity.low->high_index;
        // ASSERT(high_entity_index > 0);

        move_entity(game_state, high_entity_index, acceleration + friction_acceleration, dt);

        if (input_direction.length_2() > 0) {
            if (math::absolute(input_direction.x) > math::absolute(input_direction.y)) {
                if (input_direction.x > 0) {
                    entity.high->face_direction = FACE_DIRECTION_RIGHT;
                } else {
                    entity.high->face_direction = FACE_DIRECTION_LEFT;
                }
            } else {
                if (input_direction.y > 0) {
                    entity.high->face_direction = FACE_DIRECTION_UP;
                } else {
                    entity.high->face_direction = FACE_DIRECTION_DOWN;
                }
            }
        }
    }

    Entity followed_entity = get_entity(game_state, game_state->index_of_entity_for_camera_to_follow);

    if (followed_entity.high != NULL) {
        f32 room_width_in_meters = room_width_in_tiles * world->tile_side_in_meters;
        f32 room_height_in_meters = room_height_in_tiles * world->tile_side_in_meters;

        AbsoluteWorldPosition new_camera_position = game_state->camera_position;

#if ASUKA_DEBUG_FOLLOWING_CAMERA
        // Debug scrolling
        if (followed_entity.high->position.x > 1) {
            // diff.x = - room_width_in_meters;
            new_camera_position.absolute_tile_x += 1;
        }

        if (followed_entity.high->position.x < -1) {
            // diff.x = room_width_in_meters;
            new_camera_position.absolute_tile_x -= 1;
        }

        if (followed_entity.high->position.y > 1) {
            // diff.y = - room_height_in_meters;
            new_camera_position.absolute_tile_y += 1;
        }

        if (followed_entity.high->position.y < -1) {
            // diff.y = room_height_in_meters;
            new_camera_position.absolute_tile_y -= 1;
        }

        new_camera_position = followed_entity.low->world_position;
        // osOutputDebugString("(%f, %f)\n", new_camera_position.relative_position_on_tile.x, new_camera_position.relative_position_on_tile.y);
        // new_camera_position.relative_position_on_tile = followed_entity.low->world_position.relative_position_on_tile;
#else
        if (followed_entity.high->position.x > room_width_in_meters / 2) {
            // diff.x = - room_width_in_meters;
            new_camera_position.absolute_tile_x += room_width_in_tiles;
        }

        if (followed_entity.high->position.x < -room_width_in_meters / 2) {
            // diff.x = room_width_in_meters;
            new_camera_position.absolute_tile_x -= room_width_in_tiles;
        }

        if (followed_entity.high->position.y > room_height_in_meters / 2) {
            // diff.y = - room_height_in_meters;
            new_camera_position.absolute_tile_y += room_height_in_tiles;
        }

        if (followed_entity.high->position.y < -room_height_in_meters / 2) {
            // diff.y = room_height_in_meters;
            new_camera_position.absolute_tile_y -= room_height_in_tiles;
        }
#endif

        SetCameraPosition(game_state, new_camera_position);
    }

    // Game_OutputSound(SoundBuffer, game_state);

    // ===================== RENDERING ===================== //

    // Render pink background to see pixels I didn't drew.
    DrawRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});
    DrawRectangle(Buffer, v2{0, 0}, v2{(f32)Buffer->Width, (f32)Buffer->Height}, color24{ 0.5f, 0.5f, 0.5f });

    // Background grass
    // DrawBitmap(Buffer, { 0, 0 }, { (float32)Buffer->Width, (f32)Buffer->Height }, &game_state->grass_texture);

    AbsoluteWorldPosition *camera_p = &game_state->camera_position;

#if 1
    int32 tile_side_in_pixels = (i32)((f32)world->tile_side_in_meters * (f32)pixels_per_meter);
    int32 render_tiles_half_count_y = Buffer->Height / tile_side_in_pixels;
    int32 render_tiles_half_count_x = Buffer->Width / tile_side_in_pixels;
#else
    int32 tile_side_in_pixels = (i32)((f32)world->tile_side_in_meters * (f32)pixels_per_meter) / 3;
    int32 render_tiles_half_count_y = Buffer->Height / tile_side_in_pixels * 2;
    int32 render_tiles_half_count_x = Buffer->Width / tile_side_in_pixels * 2;
#endif

    Entity entity_to_follow = get_entity(game_state, game_state->index_of_entity_for_camera_to_follow);
    ChunkPosition chunk_pos_for_camera_to_follow {};
    if (entity_to_follow.high != NULL) {
        chunk_pos_for_camera_to_follow = GetChunkPosition(world, entity_to_follow.low->world_position);
    }

    // ===================== RENDERING ENTITIES ===================== //

    for (HighEntityIndex entity_index { 0 }; entity_index < game_state->high_entity_count; entity_index++) {
        Entity entity = get_entity(game_state, entity_index);
        if (entity.high != NULL) {
            if (entity.low->world_position.absolute_tile_z != game_state->camera_position.absolute_tile_z) {
                continue;
            }

            v2 entity_position_in_pixels =
                0.5f * v2{ (f32)Buffer->Width, (f32)Buffer->Height } +
                v2{ entity.high->position.x, -entity.high->position.y } * pixels_per_meter;

            // Hitbox rectangle
            DrawRectangle(
                Buffer,
                entity_position_in_pixels - 0.5f * entity.low->hitbox * pixels_per_meter,
                entity_position_in_pixels + 0.5f * entity.low->hitbox * pixels_per_meter,
                color24{ 1.f, 1.f, 0.f });

            // Exact character's position point
            DrawRectangle(
                Buffer,
                entity_position_in_pixels - v2{ 3.f, 3.f },
                entity_position_in_pixels + v2{ 3.f, 3.f },
                color24{ 0.f, 0.f, 0.f });

            if (entity.low->type == ENTITY_TYPE_PLAYER) {
                v2 top_left = {
                    entity_position_in_pixels.x - 0.5f * character_dimensions.x * pixels_per_meter,
                    entity_position_in_pixels.y - 1.0f * character_dimensions.y * pixels_per_meter - entity.high->position.z * pixels_per_meter,
                };

                auto *texture = &game_state->player_textures[entity.high->face_direction];
                v2 bottom_right = top_left + v2{ (f32)texture->width, (f32)texture->height };

                // Character's bitmap
                DrawBitmap(
                    Buffer,
                    top_left,
                    bottom_right,
                    texture);
                game_state->player_max_health = 3;
                game_state->player_health = 2;
                for (uint32 health_index = 0; health_index < game_state->player_max_health; health_index++){
                    v2 heart_top_left = entity_position_in_pixels + v2{ (health_index - 1.5f) * game_state->heart_full_texture.width, - (f32)game_state->player_textures[0].height - 10.0f};
                    if (health_index + 1 > game_state->player_health) {
                        DrawBitmap(
                            Buffer,
                            heart_top_left,
                            heart_top_left + v2{(f32)game_state->heart_full_texture.width, (f32)game_state->heart_full_texture.height},
                            &game_state->heart_empty_texture);
                    } else {
                        DrawBitmap(
                            Buffer,
                            heart_top_left,
                            heart_top_left + v2{(f32)game_state->heart_full_texture.width, (f32)game_state->heart_full_texture.height},
                            &game_state->heart_full_texture);
                    }
                }
            } else {
                auto TileColor = color24{ 0.2f, 0.3f, 0.2f };

                v2 top_left = entity_position_in_pixels - 0.5f * entity.low->hitbox * pixels_per_meter;
                v2 bottom_right = top_left + entity.low->hitbox * pixels_per_meter;
                DrawRectangle(
                    Buffer,
                    top_left,
                    bottom_right,
                    TileColor, true);
            }
        }
    }

    // ===================== RENDERING SIGNALING BORDERS ===================== //

#if ASUKA_PLAYBACK_LOOP
    color24 BorderColor {};
    uint32 BorderWidth = 10;
    bool32 BorderVisible {};

    switch (Input->PlaybackLoopState) {
        case PLAYBACK_LOOP_IDLE: {
            BorderVisible = false;
            break;
        }
        case PLAYBACK_LOOP_RECORDING: {
            BorderVisible = true;
            BorderColor = color24{ 1.f, 244.f / 255.f, 43.f / 255.f };
            break;
        }
        case PLAYBACK_LOOP_PLAYBACK: {
            BorderVisible = true;
            BorderColor = color24{ 29.f / 255.f, 166.f / 255.f, 8.f / 255.f };
            break;
        }
    }

    if (BorderVisible) {
        DrawBorder(Buffer, BorderWidth, BorderColor);
    }
#endif // ASUKA_PLAYBACK_LOOP
}
