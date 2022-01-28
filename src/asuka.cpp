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
        math::v2 diff = PositionDifference(game_state->tilemap, entity_low->tilemap_position, game_state->camera_position);
        entity_high->position.xy = diff;
        entity_high->velocity = math::v3::zero();
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
    entity.low->tilemap_position.absolute_tile_x = 3;
    entity.low->tilemap_position.absolute_tile_y = 3;

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
    entity.low->tilemap_position.absolute_tile_x = abs_tile_x;
    entity.low->tilemap_position.absolute_tile_y = abs_tile_y;

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
    auto tmp = map_into_tile_space(game_state->tilemap, game_state->camera_position, entity.high->position.xy);
    entity.low->tilemap_position.absolute_tile_x = tmp.absolute_tile_x;
    entity.low->tilemap_position.absolute_tile_y = tmp.absolute_tile_y;
}


INTERNAL_FUNCTION
void SetCameraPosition(GameState *game_state, TilemapPosition new_camera_position) {
    Tilemap *tilemap = game_state->tilemap;
    math::vector2 diff = PositionDifference(tilemap, game_state->camera_position, new_camera_position);
    game_state->camera_position = new_camera_position;

    math::vector2i   high_zone_in_tiles = math::vector2i { 16, 9 };
    math::rectangle2 high_zone_in_float = math::rectangle2::from_center_dim(math::v2::zero(), tilemap->tile_side_in_meters * math::upcast_to_vector2(high_zone_in_tiles));

    i32 min_tile_x = new_camera_position.absolute_tile_x - high_zone_in_tiles.x / 2;
    i32 max_tile_x = new_camera_position.absolute_tile_x + high_zone_in_tiles.x / 2;
    i32 min_tile_y = new_camera_position.absolute_tile_y - high_zone_in_tiles.y / 2;
    i32 max_tile_y = new_camera_position.absolute_tile_y + high_zone_in_tiles.y / 2;

    // @note, that the process is separated into two loops, because we have to make room for more high entities by removing old ones first.
    // If we would do it in a one big loop, we would have to make high_entities[] array twice as big.

    // First, remove entities that are out of high simulation range from high entity set.
    for (HighEntityIndex index { 1 }; index < game_state->high_entity_count;) {
        Entity entity = get_entity(game_state, index);

        math::v2 new_position = entity.high->position.xy + diff;
        if (!in_rectangle(high_zone_in_float, new_position)) {
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
            if ((entity.low->tilemap_position.absolute_tile_x >= min_tile_x) &&
                (entity.low->tilemap_position.absolute_tile_x <= max_tile_x) &&
                (entity.low->tilemap_position.absolute_tile_y >= min_tile_y) &&
                (entity.low->tilemap_position.absolute_tile_y <= max_tile_y))
            {
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

    GameState* game_state = (GameState*)Memory->PermanentStorage;

    // ===================== INITIALIZATION ===================== //

    if (!Memory->IsInitialized) {
        srand((unsigned int)time(NULL));

        // @note: reserve entity slot for the null entity
        add_low_entity(game_state);
        game_state->high_entity_count = 1;

        const char *wav_filename = "piano2.wav";
        game_state->test_wav_file = load_wav_file(wav_filename);
        game_state->test_current_sound_cursor = 0;

        MemoryArena *arena = &game_state->world_arena;

        initialize_arena(
            arena,
            (uint8 *) Memory->PermanentStorage + sizeof(GameState),
            Memory->PermanentStorageSize - sizeof(GameState));

        game_state->tilemap = push_struct(arena, Tilemap);
        Tilemap *tilemap = game_state->tilemap;

        // ===================== WORLD GENERATION ===================== //

        tilemap->tile_side_in_meters = 1.0f; // [meters]
        tilemap->chunk_count_x = 40;
        tilemap->chunk_count_y = 40;

        // Tilechunks 256x256
        tilemap->chunk_shift = 3;
        tilemap->chunk_mask  = (1 << tilemap->chunk_shift) - 1;
        tilemap->tile_count_x = 1 << tilemap->chunk_shift;
        tilemap->tile_count_y = 1 << tilemap->chunk_shift;

        i32 screen_count = 2; // 20;

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

                if (choice != GEN_NONE) break;
            };

            if (screen_idx + 1 == screen_count) {
                choice = GEN_NONE;
            }

            for (u32 tile_y = 0; tile_y < tilemap->tile_count_y; tile_y++) {
                for (u32 tile_x = 0; tile_x < tilemap->tile_count_x; tile_x++) {
                    Tile tile_value = TILE_FREE;
                    SetTileValue(arena, tilemap, tile_x, tile_y, tile_value);
                }
            }

            previous_choice = choice;
        }

        TilemapPosition camera_position;
        camera_position.absolute_tile_x = 0;
        camera_position.absolute_tile_y = 0;
        SetCameraPosition(game_state, camera_position);

        Memory->IsInitialized = true;
    }

    Tilemap *tilemap = game_state->tilemap;

    float32 pixels_per_meter = 60.f; // [pixels/m]

    for (uint32 ControllerIndex = 0; ControllerIndex < ARRAY_COUNT(Input->ControllerInputs); ControllerIndex++) {
        Game_ControllerInput* ControllerInput = GetControllerInput(Input, ControllerIndex);
    }

    // ===================== RENDERING ===================== //

    // Render pink background to see pixels I didn't drew.
    DrawRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    // Background grass
    // DrawBitmap(Buffer, { 0, 0 }, { (float32)Buffer->Width, (f32)Buffer->Height }, &game_state->grass_texture);

    TilemapPosition *camera_p = &game_state->camera_position;

#if 1
    int32 tile_side_in_pixels = (i32)((f32)tilemap->tile_side_in_meters * (f32)pixels_per_meter);
    int32 render_tiles_half_count_y = Buffer->Height / tile_side_in_pixels;
    int32 render_tiles_half_count_x = Buffer->Width / tile_side_in_pixels;
#else
    int32 tile_side_in_pixels = (i32)((f32)tilemap->tile_side_in_meters * (f32)pixels_per_meter) / 3;
    int32 render_tiles_half_count_y = Buffer->Height / tile_side_in_pixels * 2;
    int32 render_tiles_half_count_x = Buffer->Width / tile_side_in_pixels * 2;
#endif

    //
    // Render tiles
    //
    for (int32 relative_row = -render_tiles_half_count_y; relative_row < render_tiles_half_count_y; relative_row++) {
        for (int32 relative_column = -render_tiles_half_count_x; relative_column < render_tiles_half_count_x; relative_column++) {

            int32 row = camera_p->absolute_tile_y + relative_row;
            int32 column = camera_p->absolute_tile_x + relative_column;

            v2 bottom_left_in_bottom_up_screen_pixel_coords =
                v2{ (f32)relative_column, (f32)relative_row } * (f32)tile_side_in_pixels -
                0.5f * pixels_per_meter * v2::ey() + // @bug: What this term even does??
                v2{ 0.5f * Buffer->Width, 0.5f * Buffer->Height };

            v2 upper_left_in_up_down_screen_pixel_coords {
                bottom_left_in_bottom_up_screen_pixel_coords.x,
                Buffer->Height - bottom_left_in_bottom_up_screen_pixel_coords.y - tile_side_in_pixels,
            };

            v2 bottom_right_in_up_down_screen_pixel_coords =
                upper_left_in_up_down_screen_pixel_coords + v2::make((f32)tile_side_in_pixels);


            Tile tile_value = GetTileValue(tilemap, column, row);
            auto TileColor = color24{ 0.5f, 0.5f, 0.5f };

            switch (tile_value) {
                case TILE_INVALID: {
                    TileColor = color24{ 1.0f };
                    break;
                    // continue;
                }
                case TILE_FREE: {
                    TileColor = color24{ 0.5f, 0.5f, 0.5f };

                    if ((row % tilemap->tile_count_y == 0 || column % tilemap->tile_count_x == 0))
                    {
                        TileColor = color24{ 0.0f, 0.4f, 0.8f };
                        break;
                    }

                    break;
                }
                // Walls are entities now

                // case TILE_WALL: {
                //     // DrawBitmap(Buffer, upper_left_in_up_down_screen_pixel_coords, bottom_right_in_up_down_screen_pixel_coords, &game_state->wall_texture);
                //     // continue;

                //     TileColor = color24{ 0.2f, 0.3f, 0.2f };
                //     break;
                // }
                case TILE_DOOR_UP: {
                    TileColor = color24{ 0.8f, 0.8f, 0.8f };
                    break;
                }
                case TILE_DOOR_DOWN: {
                    TileColor = color24{ 0.3f, 0.3f, 0.3f };
                    break;
                }
                case TILE_WIN: {
                    STATIC_VARIABLE f32 t_color = 0.0f;
                    f32 tmp_r = sinf(math::pi * t_color - 0.5f * math::pi);
                    f32 tmp_g = sinf(math::pi * t_color - 5.0f * math::pi / 6.0f);
                    f32 tmp_b = sinf(math::pi * t_color - math::pi / 6.0f);

                    f32 normalization_constant = 1.0f;

                    color24 tile_win_color = color24{
                        tmp_r * tmp_r * normalization_constant,
                        tmp_g * tmp_g * normalization_constant,
                        tmp_b * tmp_b * normalization_constant,
                    };
                    t_color += 0.01f;

                    if (t_color > 1.0f) { t_color -= 1.0f; }

                    TileColor = tile_win_color;
                    break;
                }
            }

            DrawRectangle(
                Buffer,
                upper_left_in_up_down_screen_pixel_coords,
                bottom_right_in_up_down_screen_pixel_coords,
                TileColor, true);
        }
    }

    // ===================== RENDERING ENTITIES ===================== //

    // for (HighEntityIndex entity_index { 0 }; entity_index < game_state->high_entity_count; entity_index++) {
    //     Entity entity = get_entity(game_state, entity_index);
    //     if (entity.high != NULL) {
    //         if (entity.low->tilemap_position.absolute_tile_z != game_state->camera_position.absolute_tile_z) {
    //             continue;
    //         }

    //         // @todo: It should not access internal member 'index'.
    //         if (entity_index.index == game_state->index_of_entity_for_camera_to_follow.index)
    //             osOutputDebugString("(%5.2f, %5.2f)\n", entity.high->position.x, entity.high->position.y);

    //         v2 entity_position_in_pixels =
    //             0.5f * v2{ (f32)Buffer->Width, (f32)Buffer->Height } +
    //             v2{ entity.high->position.x, -entity.high->position.y } * pixels_per_meter;

    //         auto TileColor = color24{ 0.2f, 0.3f, 0.2f };

    //         v2 top_left = entity_position_in_pixels - 0.5f * entity.low->hitbox * pixels_per_meter;
    //         v2 bottom_right = top_left + entity.low->hitbox * pixels_per_meter;
    //         DrawRectangle(
    //             Buffer,
    //             top_left,
    //             bottom_right,
    //             TileColor, true);
    //     }
    // }

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
