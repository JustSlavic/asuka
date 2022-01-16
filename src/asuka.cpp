#include "asuka.hpp"
#include <math.hpp>
#include <debug/casts.hpp>




INTERNAL_FUNCTION
void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, Game_State* game_state) {
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
void RenderBitmap(
    Game_OffscreenBuffer* buffer,
    math::v2 top_left, math::v2 bottom_right,
    bitmap *image,
    bool32 stroke = false)
{
    using math::vector2i;
    using math::vector2;

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
void RenderRectangle(
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


// void render_text_into_bitmap(FT_Bitmap *ft_bitmap, FT_Int x, FT_Int y, bitmap *target_bitmap) {
//     FT_Int i, j, p, q;
//     FT_Int x_max = x + ft_bitmap->width;
//     FT_Int y_max = y + ft_bitmap->rows;

//     /* for simplicity, we assume that `bitmap->pixel_mode' */
//     /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

//     for (i = x, p = 0; i < x_max; i++, p++)
//     {
//         for (j = y, q = 0; j < y_max; j++, q++)
//         {
//             if (i < 0 || j < 0 || i >= (int)target_bitmap->width || j >= (int)target_bitmap->height)
//                 continue;

//             ((uint8*)target_bitmap->pixels)[j * target_bitmap->width + i] |= ft_bitmap->buffer[q * ft_bitmap->width + p];
//         }
//     }
// }


#if ASUKA_PLAYBACK_LOOP
INTERNAL_FUNCTION
void RenderBorder(Game_OffscreenBuffer* Buffer, uint32 Width, math::color24 Color) {
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Width}, {(float32)Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {(float32)Buffer->Width - Width, (float32)Width}, {(float32)Buffer->Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Buffer->Height - Width}, {(float32)Buffer->Width, (float32)Buffer->Height}, Color);
}
#endif


INTERNAL_FUNCTION
void change_entity_residence(Game_State *game_state, EntityResidence residence, uint32 index) {
    if (residence == ENTITY_RESIDENCE_HIGH) {
        EntityResidence current_residence = game_state->residence_table[index];

        if (current_residence != ENTITY_RESIDENCE_HIGH) {
            HighFrequencyEntity *entity_high = game_state->high_frequency_entity_table + index;
            LowFrequencyEntity *entity_low = game_state->low_frequency_entity_table + index;

            math::v2 diff = PositionDifference(&game_state->world->tilemap, entity_low->tilemap_position, game_state->camera_position);
            entity_high->position = diff;
            entity_high->velocity = math::v2::zero();
            entity_high->absolute_tile_z = entity_low->tilemap_position.absolute_tile_z;
            entity_high->face_direction = FACE_DIRECTION_DOWN;
        }
    }

    game_state->residence_table[index] = residence;
}


INTERNAL_FUNCTION
Entity get_entity(Game_State *game_state, EntityResidence desired_residence, uint32 index) {
    Entity entity {};

    if ((index > 0) && (index < game_state->entity_count)) {
        if (game_state->residence_table[index] < desired_residence) {
            change_entity_residence(game_state, desired_residence, index);
            ASSERT(game_state->residence_table[index] >= desired_residence);
        }

        entity.residence = game_state->residence_table[index];
        entity.high = game_state->high_frequency_entity_table + index;
        entity.low  = game_state->low_frequency_entity_table + index;
    }

    return entity;
}


INTERNAL_FUNCTION
void initialize_player(Game_State *game_state, uint32 entity_index) {
    Entity entity = get_entity(game_state, ENTITY_RESIDENCE_LOW, entity_index);

    entity.low->tilemap_position.absolute_tile_x = 3;
    entity.low->tilemap_position.absolute_tile_y = 3;
    entity.low->tilemap_position.absolute_tile_z = 0;
    entity.low->tilemap_position.relative_position_on_tile = math::v2{};

    // @todo: fix coordinates for hitbox
    entity.low->hitbox = math::v2 { 0.8f, 0.4f }; // In top-down coordinates, but in meters.

    change_entity_residence(game_state, ENTITY_RESIDENCE_HIGH, entity_index);

    if (get_entity(game_state, ENTITY_RESIDENCE_LOW, game_state->index_of_entity_for_camera_to_follow).residence == ENTITY_RESIDENCE_NOT_EXIST) {
        game_state->index_of_entity_for_camera_to_follow = entity_index;
    }
}


INTERNAL_FUNCTION
uint32 add_entity(Game_State *game_state) {
    uint32 entity_index = game_state->entity_count++;

    ASSERT(game_state->entity_count < ARRAY_COUNT(game_state->residence_table));
    ASSERT(game_state->entity_count < ARRAY_COUNT(game_state->high_frequency_entity_table));
    ASSERT(game_state->entity_count < ARRAY_COUNT(game_state->low_frequency_entity_table));

    game_state->residence_table[entity_index] = ENTITY_RESIDENCE_LOW;
    game_state->high_frequency_entity_table[entity_index] = {};
    game_state->low_frequency_entity_table[entity_index] = {};

    return entity_index;
}


INTERNAL_FUNCTION
void move_entity(Game_State *game_state, uint32 entity_index, math::v2 acceleration, float32 dt) {
    using math::v2;

    Entity entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, entity_index);

    v2 velocity = entity.high->velocity + acceleration * dt;
    v2 position = entity.high->position;

    v2 new_position = position + velocity * dt;

    // ================= COLLISION DETECTION ====================== //

    v2 current_position = position;
    v2 current_velocity = velocity;
    v2 current_destination = new_position;
    f32 time_spent_moving = 0.0f;

    f32 original_move_distance = (new_position - position).length();

    const int ASUKA_MAX_MOVE_TRIES = 5;
    for (int32 move_try = 0; move_try < ASUKA_MAX_MOVE_TRIES; move_try++) {
        v2 closest_destination = current_destination;
        v2 velocity_at_closest_destination = current_velocity;

        uint32 hit_entity_index = 0;

        for (uint32 test_entity_index = 1; test_entity_index < game_state->entity_count; test_entity_index++) {
            if (test_entity_index == entity_index) continue;

            Entity test_entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, test_entity_index);

            if (test_entity.low->tilemap_position.absolute_tile_z != entity.low->tilemap_position.absolute_tile_z) continue;

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
                        // @todo: what if we collide with several entities during move tries
                        hit_entity_index = test_entity_index;
                        // @note: update only closest point
                        if ((res.intersection - current_position).length_2() < (closest_destination - current_position).length_2()) {
                            closest_destination = res.intersection;
                            velocity_at_closest_destination = math::project(current_velocity, wall); // wall * math::dot(current_velocity, wall) / wall.length_2();
                        }
                    }
                }
            }
        }

        // @note: this have to be calculated before we change current position.
        f32 move_distance = (closest_destination - current_position).length();

        current_position = closest_destination;
        if (hit_entity_index) {
            if (move_distance > EPSILON) {
                f32 dt_prime = move_distance / current_velocity.length();
                time_spent_moving += dt_prime;
            }

            current_velocity = velocity_at_closest_destination;
            current_destination = current_position + current_velocity * (dt - time_spent_moving);

            Entity hit_entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, hit_entity_index);
            entity.high->absolute_tile_z += hit_entity.low->d_abs_tile_z;
        } else {
            break;
        }
    }

    entity.high->position = current_position;
    entity.high->velocity = current_velocity;

    // @note: always write back a valid tile position
    auto tmp = map_into_tile_space(&game_state->world->tilemap, game_state->camera_position, entity.high->position);
    entity.low->tilemap_position.absolute_tile_x = tmp.absolute_tile_x;
    entity.low->tilemap_position.absolute_tile_y = tmp.absolute_tile_y;
    entity.low->tilemap_position.relative_position_on_tile = tmp.relative_position_on_tile;

    // osOutputDebugString("Entity: position (%d, %d, %d)\n", entity.low->tilemap_position.absolute_tile_x, entity.low->tilemap_position.absolute_tile_y, entity.low->tilemap_position.absolute_tile_z);
}


// Random
#include <time.h>
#include <stdlib.h>

GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    using math::v2;
    using math::color24;

    ASSERT(sizeof(Game_State) <= Memory->PermanentStorageSize);

    float32 dt = Input->dt;

    Game_State* game_state = (Game_State*)Memory->PermanentStorage;

    i32 room_width_in_tiles = 16;
    i32 room_height_in_tiles = 9;

    // ===================== INITIALIZATION ===================== //

    if (!Memory->IsInitialized) {
        srand((unsigned int)time(NULL));

        // @note: reserve entity slot for the null entity
        add_entity(game_state);

        const char *wav_filename = "piano2.wav";
        game_state->test_wav_file = load_wav_file(wav_filename);
        game_state->test_current_sound_cursor = 0;

        const char *floor_texture_filename = "tile_16x16.png";
        game_state->floor_texture = load_png_file_myself(floor_texture_filename);

        const char *grass_texture_filename = "grass_texture.png";
        game_state->grass_texture = load_png_file(grass_texture_filename);

        const char *wall_texture_filename = "tile_60x60.bmp";
        game_state->wall_texture = load_bmp_file(wall_texture_filename);

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
            (uint8 *) Memory->PermanentStorage + sizeof(Game_State),
            Memory->PermanentStorageSize - sizeof(Game_State));

        game_state->world = push_struct(arena, Game_World);
        Game_World *world = game_state->world;
        Tilemap *tilemap = &world->tilemap;

        game_state->camera_position.absolute_tile_x = 16 / 2;
        game_state->camera_position.absolute_tile_y = 9 / 2;

        game_state->text_bitmap.width = 640;
        game_state->text_bitmap.height = 480;
        game_state->text_bitmap.pixels = push_array(arena, uint8, 640*480);
        game_state->text_bitmap.size = 640*480;
        game_state->text_bitmap.bytes_per_pixel = 1;

        char text_to_render[] = "Hello Sailor!";
        game_state->text_to_render.buffer = text_to_render;
        game_state->text_to_render.size = sizeof(text_to_render) - 1;

        // FT_Error freetype_error;
        // freetype_error = FT_Init_FreeType(&game_state->freetype_library);
        // freetype_error = FT_New_Face(game_state->freetype_library, "mangat.ttf", 0, &game_state->freetype_face);
        // freetype_error = FT_Set_Char_Size(game_state->freetype_face, 50*64, 0, 100, 0);
        // game_state->freetype_slot = game_state->freetype_face->glyph;

        /* set up matrix */
        // float angle = 0;
        // game_state->freetype_matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
        // game_state->freetype_matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
        // game_state->freetype_matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
        // game_state->freetype_matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

        // ===================== WORLD GENERATION ===================== //

        tilemap->tile_side_in_meters = 1.0f; // [meters]
        tilemap->chunk_count_x = 40;
        tilemap->chunk_count_y = 40;
        tilemap->chunk_count_z = 4;

        // Tilechunks 256x256
        tilemap->chunk_shift = 3;
        tilemap->chunk_mask  = (1 << tilemap->chunk_shift) - 1;
        tilemap->tile_count_x = 1 << tilemap->chunk_shift;
        tilemap->tile_count_y = 1 << tilemap->chunk_shift;

        tile_chunk *chunks = push_array(arena, tile_chunk, tilemap->chunk_count_x * tilemap->chunk_count_y * tilemap->chunk_count_z);
        tilemap->chunks = chunks;

        i32 screens_count = 20;

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

        for (i32 screen_idx = 0; screen_idx < screens_count; screen_idx++) {
            gen_direction choice = GEN_NONE;

            while(true) {
                choice = (gen_direction)(rand() % GEN_MAX);

                if (choice == GEN_FLOOR_UP && (screen_z + 1) == (i32)tilemap->chunk_count_z) continue;
                if (choice == GEN_FLOOR_DOWN && screen_z == 0) continue;
                if (choice == GEN_FLOOR_UP && previous_choice == GEN_FLOOR_DOWN) continue;
                if (choice == GEN_FLOOR_DOWN && previous_choice == GEN_FLOOR_UP) continue;

                if (choice != GEN_NONE) break;
            };

            if (screen_idx + 1 == screens_count) {
                choice = GEN_NONE;
            }

            for (i32 tile_y = 0; tile_y < room_height_in_tiles; tile_y++) {
                for (i32 tile_x = 0; tile_x < room_width_in_tiles; tile_x++) {
                    u32 x = screen_x * room_width_in_tiles  + tile_x;
                    u32 y = screen_y * room_height_in_tiles + tile_y;
                    u32 z = screen_z;

                    tile_t tile_value = TILE_FREE;

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

                    if ((screen_idx == screens_count - 1) && (tile_x == room_width_in_tiles - 2) && (tile_y == room_height_in_tiles - 2)) {
                        tile_value = TILE_WIN;
                    }

                    SetTileValue(arena, tilemap, x, y, z, tile_value);
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

        // uint32 player_index = add_entity(game_state);
        // initialize_player(game_state, player_index);

        Memory->IsInitialized = true;
    }

    Game_World *world = game_state->world;
    Tilemap *tilemap = &world->tilemap;

    float32 pixels_per_meter = 60.f; // [pixels/m]

    v2  character_dimensions = { 0.75f, 1.0f }; // [m; m]
    f32 character_speed = 2.5f; // [m/s]
    f32 character_mass = 80.0f; // [kg]
    f32 gravity_acceleration = 9.8f; // [m/s^2]
    f32 friction_coefficient = 1.5f;

    for (uint32 ControllerIndex = 0; ControllerIndex < ARRAY_COUNT(Input->ControllerInputs); ControllerIndex++) {
        Game_ControllerInput* ControllerInput = GetControllerInput(Input, ControllerIndex);

        uint32 entity_index = game_state->player_index_for_controller[ControllerIndex];
        Entity entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, entity_index);

        if (entity.residence == ENTITY_RESIDENCE_NOT_EXIST) {
            if (GetPressCount(ControllerInput->Start)) {
                entity_index = add_entity(game_state);
                game_state->player_index_for_controller[ControllerIndex] = entity_index;

                initialize_player(game_state, entity_index);

                entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, entity_index);
            } else {
                continue;
            }
        }

        if (GetPressCount(ControllerInput->Y)) {
            game_state->index_of_entity_for_camera_to_follow += 1;
            if (game_state->index_of_entity_for_camera_to_follow > game_state->entity_count - 1) {
                game_state->index_of_entity_for_camera_to_follow = 1;
            }
            game_state->camera_position.absolute_tile_z = game_state->low_frequency_entity_table[game_state->index_of_entity_for_camera_to_follow].tilemap_position.absolute_tile_z;
            osOutputDebugString("Follow entity %d\n", game_state->index_of_entity_for_camera_to_follow);
        }

        if (GetPressCount(ControllerInput->X)) {
            TilemapPosition *pos = &entity.low->tilemap_position;
            tile_t tile_value = GetTileValue(tilemap, pos->absolute_tile_x, pos->absolute_tile_y, pos->absolute_tile_z);

            if (tile_value == TILE_DOOR_UP) {
                TilemapPosition move_up_position = *pos;
                move_up_position.absolute_tile_z += 1;

                bool up_tile_empty = IsWorldPointEmpty(tilemap, move_up_position);
                if (up_tile_empty) {
                    pos->absolute_tile_z += 1;
                    Entity followed_entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, game_state->index_of_entity_for_camera_to_follow);
                    game_state->camera_position.absolute_tile_z = followed_entity.low->tilemap_position.absolute_tile_z;
                    osOutputDebugString("Entity %d go upstairs\n", entity_index);
                }
            }

            if (tile_value == TILE_DOOR_DOWN) {
                TilemapPosition move_down_position = *pos;
                move_down_position.absolute_tile_z -= 1;

                bool down_tile_empty = IsWorldPointEmpty(tilemap, move_down_position);
                if (down_tile_empty) {
                    pos->absolute_tile_z -= 1;
                    Entity followed_entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, game_state->index_of_entity_for_camera_to_follow);
                    game_state->camera_position.absolute_tile_z = followed_entity.low->tilemap_position.absolute_tile_z;
                    osOutputDebugString("Entity %d go downstairs\n", entity_index);
                }
            }
        }

        // ================= MOVEMENT SIMULATION ====================== //

        v2 input_direction = ControllerInput->LeftStickEnded.normalized();
        f32 input_strength = math::clamp(ControllerInput->LeftStickEnded.norm(), 0, 1);

        float32 acceleration_coefficient = 100.0f; // [m/s^2]
        if (ControllerInput->A.EndedDown) {
            acceleration_coefficient = 300.0f;
        }

        v2 acceleration = acceleration_coefficient * input_strength * input_direction;

        // [m/s^2] = [m/s] * [units] * [m/s^2]
        // @todo: why units do not add up?
        v2 friction_acceleration = (entity.high->velocity) * friction_coefficient * gravity_acceleration;

        move_entity(game_state, entity_index, acceleration - friction_acceleration, dt);

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

        Entity followed_entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, game_state->index_of_entity_for_camera_to_follow);

        if (followed_entity.residence == ENTITY_RESIDENCE_HIGH) {
            f32 room_width_in_meters = room_width_in_tiles * tilemap->tile_side_in_meters;
            f32 room_height_in_meters = room_height_in_tiles * tilemap->tile_side_in_meters;

            if (math::absolute(followed_entity.high->position.x) > room_width_in_meters / 2 ||
                math::absolute(followed_entity.high->position.y) > room_height_in_meters / 2)
            {
                // osOutputDebugString("Entity out of room: (%5.2f, %5.2f)!\n", followed_entity.high->position.x, followed_entity.high->position.y);

                math::v2 diff {};

                if (followed_entity.high->position.x > room_width_in_meters / 2) {
                    diff.x = - room_width_in_meters;
                    game_state->camera_position.absolute_tile_x += room_width_in_tiles;
                }
                if (followed_entity.high->position.x < -room_width_in_meters / 2) {
                    diff.x = room_width_in_meters;
                    game_state->camera_position.absolute_tile_x -= room_width_in_tiles;
                }
                if (followed_entity.high->position.y > room_height_in_meters / 2) {
                    diff.y = - room_height_in_meters;
                    game_state->camera_position.absolute_tile_y += room_height_in_tiles;
                }
                if (followed_entity.high->position.y < -room_height_in_meters / 2) {
                    diff.y = room_height_in_meters;
                    game_state->camera_position.absolute_tile_y -= room_height_in_tiles;
                }

                for (uint32 index = 1; index < game_state->entity_count; index++) {
                    if (game_state->residence_table[index] == ENTITY_RESIDENCE_HIGH) {
                        Entity e = get_entity(game_state, ENTITY_RESIDENCE_HIGH, index);
                        e.high->position += diff;
                    }
                }

                // @todo: map new entities in and old entities out
                // @todo: map walls and stairs (doors) into the entity set
            }
        }
    }

    // Game_OutputSound(SoundBuffer, game_state);

    // ===================== RENDERING ===================== //

    // Render pink background to see pixels I didn't drew.
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    // Background grass
    // RenderBitmap(Buffer, { 0, 0 }, { (float32)Buffer->Width, (f32)Buffer->Height }, &game_state->grass_texture);

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

    Entity entity_to_follow = get_entity(game_state, ENTITY_RESIDENCE_HIGH, game_state->index_of_entity_for_camera_to_follow);
    TileChunkPosition chunk_pos_for_camera_to_follow {};
    if (entity_to_follow.residence == ENTITY_RESIDENCE_HIGH) {
        chunk_pos_for_camera_to_follow = GetChunkPosition(tilemap, entity_to_follow.low->tilemap_position);
    }

    //
    // Render tiles
    //
    for (int32 relative_row = -render_tiles_half_count_y; relative_row < render_tiles_half_count_y; relative_row++) {
        for (int32 relative_column = -render_tiles_half_count_x; relative_column < render_tiles_half_count_x; relative_column++) {

            int32 row = camera_p->absolute_tile_y + relative_row;
            int32 column = camera_p->absolute_tile_x + relative_column;
            int32 level = camera_p->absolute_tile_z;

            v2 bottom_left_in_bottom_up_screen_pixel_coords =
                v2{ (f32)relative_column, (f32)relative_row } * (f32)tile_side_in_pixels -
                (camera_p->relative_position_on_tile + v2{ 0.0f, 0.5f }) * pixels_per_meter +
                v2{ 0.5f * Buffer->Width, 0.5f * Buffer->Height };

            v2 upper_left_in_up_down_screen_pixel_coords {
                bottom_left_in_bottom_up_screen_pixel_coords.x,
                Buffer->Height - bottom_left_in_bottom_up_screen_pixel_coords.y - tile_side_in_pixels,
            };

            v2 bottom_right_in_up_down_screen_pixel_coords =
                upper_left_in_up_down_screen_pixel_coords + v2::from((f32)tile_side_in_pixels);


            tile_t tile_value = GetTileValue(tilemap, column, row, level);
            auto TileColor = color24{ 0.5f, 0.5f, 0.5f };

            switch (tile_value) {
                case TILE_INVALID: {
                    TileColor = color24{ 1.0f };
                    break;
                    // continue;
                }
                case TILE_FREE: {
                    TileColor = color24{ 0.5f, 0.5f, 0.5f };

                    if (row == (int32)(chunk_pos_for_camera_to_follow.chunk_relative_y + chunk_pos_for_camera_to_follow.chunk_y * tilemap->tile_count_y) &&
                        column == (int32)(chunk_pos_for_camera_to_follow.chunk_relative_x + chunk_pos_for_camera_to_follow.chunk_x * tilemap->tile_count_x)) {
                        TileColor = color24{ 0.8f, 0.4f, 0.0f };
                    }
                    // else
                    // if ((row % tilemap->tile_count_y == 0 || column % tilemap->tile_count_x == 0))
                    // {
                    //     TileColor = color24{ 0.0f, 0.4f, 0.8f };
                    //     break;
                    // }

                    break;
                }
                case TILE_WALL: {
                    // RenderBitmap(Buffer, upper_left_in_up_down_screen_pixel_coords, bottom_right_in_up_down_screen_pixel_coords, &game_state->wall_texture);
                    // continue;

                    TileColor = color24{ 0.2f, 0.3f, 0.2f };
                    break;
                }
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

            RenderRectangle(
                Buffer,
                upper_left_in_up_down_screen_pixel_coords,
                bottom_right_in_up_down_screen_pixel_coords,
                TileColor, true);
        }
    }

    // ===================== RENDERING ENTITIES ===================== //

    for (uint32 entity_index = 0; entity_index < game_state->entity_count; entity_index++) {
        if (game_state->residence_table[entity_index] == ENTITY_RESIDENCE_HIGH) {
            Entity entity = get_entity(game_state, ENTITY_RESIDENCE_HIGH, entity_index);

            // osOutputDebugString("Entity %d on floor %d\n", entity_index, entity.low->tilemap_position.absolute_tile_z);
            if (entity.low->tilemap_position.absolute_tile_z != game_state->camera_position.absolute_tile_z) {
                continue;
            }

            v2 entity_position_in_pixels =
                0.5f * v2{ (f32)Buffer->Width, (f32)Buffer->Height } +
                v2{ entity.high->position.x, -entity.high->position.y } * pixels_per_meter;

            // Hitbox rectangle
            RenderRectangle(
                Buffer,
                entity_position_in_pixels - 0.5f * entity.low->hitbox * pixels_per_meter,
                entity_position_in_pixels + 0.5f * entity.low->hitbox * pixels_per_meter,
                color24{ 1.f, 1.f, 0.f });

            // Exact character's position point
            RenderRectangle(
                Buffer,
                entity_position_in_pixels - v2{ 3.f, 3.f },
                entity_position_in_pixels + v2{ 3.f, 3.f },
                color24{ 0.f, 0.f, 0.f });

            v2 top_left = {
                entity_position_in_pixels.x - 0.5f * character_dimensions.x * pixels_per_meter,
                entity_position_in_pixels.y - 1.0f * character_dimensions.y * pixels_per_meter,
            };

            auto *texture = &game_state->player_textures[entity.high->face_direction];
            v2 bottom_right = top_left + v2{ (f32)texture->width, (f32)texture->height };

            // Character's bitmap
            RenderBitmap(
                Buffer,
                top_left,
                bottom_right,
                texture);
        }

#if 0
        Entity *entity = get_entity(game_state, entity_index);

        if (entity->exists) {
            auto *entity_p = &entity->position;

            v2 entity_position_in_meters = v2{
                (f32)entity_p->absolute_tile_x - camera_p->absolute_tile_x,
                (f32)entity_p->absolute_tile_y - camera_p->absolute_tile_y,
            } * tilemap->tile_side_in_meters
              + (entity_p->relative_position_on_tile - camera_p->relative_position_on_tile)
              + v2{ 0.5f, 0.0f } * tilemap->tile_side_in_meters;

            // in top-down screen pixel coordinates
            v2 entity_position_in_pixels = v2{
                0.5f * Buffer->Width  + entity_position_in_meters.x * pixels_per_meter,
                0.5f * Buffer->Height + entity_position_in_meters.y * pixels_per_meter,
            };

            RenderRectangle(
                Buffer,
                entity_position_in_pixels - 0.5f * entity->hitbox * pixels_per_meter,
                entity_position_in_pixels + 0.5f * entity->hitbox * pixels_per_meter,
                color24{ 1.f, 1.f, 0.f });

            RenderRectangle(
                Buffer,
                entity_position_in_pixels - v2{ 3.f, 3.f },
                entity_position_in_pixels + v2{ 3.f, 3.f },
                color24{ 0.f, 0.f, 0.f });

            v2 top_left = {
                entity_position_in_pixels.x - 0.5f * character_dimensions.x * pixels_per_meter,
                entity_position_in_pixels.y - 1.0f * character_dimensions.y * pixels_per_meter,
            };

            auto *texture = &game_state->player_textures[entity->face_direction];
            v2 bottom_right = top_left + v2{ (f32)texture->width, (f32)texture->height };

            RenderBitmap(
                Buffer,
                top_left,
                bottom_right,
                texture);
        }
#endif
    }

    // ===================== RENDERING TEXT ===================== //

    // {
    //     /* the pen position in 26.6 cartesian space coordinates; */
    //     /* start at (300,200) relative to the upper left corner  */
    //     game_state->freetype_pen.x = 50; //300*64;
    //     game_state->freetype_pen.y = 50; //(game_state->text_bitmap.height - 200)*64;

    //     static int32 text_rendered = 0;
    //     if (!text_rendered)
    //     for (int n = 0; n < game_state->text_to_render.size; n++)
    //     {
    //         /* set transformation */
    //         FT_Set_Transform(game_state->freetype_face, &game_state->freetype_matrix, &game_state->freetype_pen);

    //         /* load glyph image into the slot (erase previous one) */
    //         FT_Error error = FT_Load_Char(game_state->freetype_face, game_state->text_to_render.buffer[n], FT_LOAD_RENDER);
    //         if (error)
    //             continue;                 /* ignore errors */

    //         /* now, draw to our target surface (convert position) */
    //         render_text_into_bitmap(
    //             &game_state->freetype_slot->bitmap,
    //             game_state->freetype_slot->bitmap_left,
    //             game_state->text_bitmap.height - game_state->freetype_slot->bitmap_top,
    //             &game_state->text_bitmap);

    //         /* increment pen position */
    //         game_state->freetype_pen.x += game_state->freetype_slot->advance.x;
    //         game_state->freetype_pen.y += game_state->freetype_slot->advance.y;
    //     }

    //     text_rendered = 1;

    //     RenderBitmap(Buffer, v2{0, 0}, v2{(f32)game_state->text_bitmap.width, (f32)game_state->text_bitmap.height}, &game_state->text_bitmap);
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
        RenderBorder(Buffer, BorderWidth, BorderColor);
    }
#endif // ASUKA_PLAYBACK_LOOP
}
