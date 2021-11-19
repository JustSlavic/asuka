#include "asuka.hpp"
#include <math.hpp>
#include <debug/casts.hpp>


INTERNAL_FUNCTION
void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, game_state* GameState) {
    sound_sample_t* SampleOut = SoundBuffer->Samples;

    for (int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
        uint64 left_idx = (GameState->test_current_sound_cursor++) % GameState->test_wav_file.samples_count;
        uint64 right_idx = (GameState->test_current_sound_cursor++) % GameState->test_wav_file.samples_count;

        sound_sample_t LeftSample =  GameState->test_wav_file.samples[left_idx];
        sound_sample_t RightSample = GameState->test_wav_file.samples[right_idx];

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
    int32 integer_scale = 0,
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

            if (image->bytes_per_pixel == 3) {
                *Pixel = (red) | (green << 8) | (blue << 16);
            } else {
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


#if ASUKA_PLAYBACK_LOOP
INTERNAL_FUNCTION
void RenderBorder(Game_OffscreenBuffer* Buffer, uint32 Width, math::color24 Color) {
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Width}, {(float32)Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {(float32)Buffer->Width - Width, (float32)Width}, {(float32)Buffer->Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Buffer->Height - Width}, {(float32)Buffer->Width, (float32)Buffer->Height}, Color);
}
#endif


math::v2 ClosestPointInRectangle(math::v2 min_corner, math::v2 max_corner, math::v2 position) {
    math::v2 result {};
    return result;
}


#include <time.h>
#include <stdlib.h>
GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    using math::v2;
    using math::color24;

    ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);

    float32 dt = Input->dt;

    game_state* GameState = (game_state*)Memory->PermanentStorage;

    i32 room_width_in_tiles = 16;
    i32 room_height_in_tiles = 9;

    // ===================== INITIALIZATION ===================== //

    if (!Memory->IsInitialized) {
        srand((unsigned int)time(NULL));
        GameState->player.position.absolute_tile_x = 3;
        GameState->player.position.absolute_tile_y = 3;
        GameState->player.position.absolute_tile_z = 0;

        GameState->player.position.relative_position_on_tile = { 0.0f, 0.0f }; // in meters relative to the tile

        GameState->player.face_direction = PLAYER_FACE_DOWN;
        GameState->player.hitbox = v2{ 0.75f, 0.35f };

        const char *wav_filename = "piano2.wav";
        GameState->test_wav_file = load_wav_file(wav_filename);
        GameState->test_current_sound_cursor = 0;

        const char *floor_texture_filename = "tile_16x16.png";
        GameState->floor_texture = load_png_file_myself(floor_texture_filename);

        const char *grass_texture_filename = "grass_texture.png";
        GameState->grass_texture = load_png_file(grass_texture_filename);

        const char *wall_texture_filename = "tile_60x60.bmp";
        GameState->wall_texture = load_bmp_file(wall_texture_filename);

        const char *player_face_texture_filename = "character_1.png";
        const char *player_left_texture_filename = "character_2.png";
        const char *player_right_texture_filename = "character_3.png";
        const char *player_back_texture_filename = "character_4.png";

        GameState->player_textures[0] = load_png_file(player_face_texture_filename);
        GameState->player_textures[1] = load_png_file(player_left_texture_filename);
        GameState->player_textures[2] = load_png_file(player_right_texture_filename);
        GameState->player_textures[3] = load_png_file(player_back_texture_filename);

        memory_arena *arena = &GameState->world_arena;

        initialize_arena(
            arena,
            (uint8 *) Memory->PermanentStorage + sizeof(game_state),
            Memory->PermanentStorageSize - sizeof(game_state));

        // ===================== WORLD GENERATION ===================== //

        GameState->world = push_struct(arena, game_world);
        game_world *world = GameState->world;
        tile_map *tilemap = &world->tilemap;

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

        Memory->IsInitialized = true;
    }

    game_world *world = GameState->world;
    tile_map *tilemap = &world->tilemap;

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
#endif // ASUKA_PLAYBACK_LOOP

    float32 pixels_per_meter = 60.f; // [pixels/m]

    v2  character_dimensions = { 0.75f, 1.0f }; // [m; m]
    f32 character_speed = 2.5f; // [m/s]
    f32 character_mass = 80.0f; // [kg]
    f32 gravity_acceleration = 9.8f; // [m/s^2]
    f32 friction_coefficient = 1.0f;

    {
        Game_ControllerInput* ControllerInput = GetGamepadInput(Input, 0);
        ControllerInput = &Input->KeyboardInput;

        if (GetPressCount(ControllerInput->Y)) {
            Memory->IsInitialized = false;
        }

            tile_map_position *pos = &GameState->player.position;
        if (GetPressCount(ControllerInput->X)) {
            tile_t tile_value = GetTileValue(tilemap, pos->absolute_tile_x, pos->absolute_tile_y, pos->absolute_tile_z);

            if (tile_value == TILE_DOOR_UP) {
                tile_map_position move_up_position = *pos;
                move_up_position.absolute_tile_z += 1;

                bool up_tile_empty = IsWorldPointEmpty(tilemap, move_up_position);
                if (up_tile_empty) {
                    pos->absolute_tile_z += 1;
                }
            }

            if (tile_value == TILE_DOOR_DOWN) {
                tile_map_position move_down_position = *pos;
                move_down_position.absolute_tile_z -= 1;

                bool down_tile_empty = IsWorldPointEmpty(tilemap, move_down_position);
                if (down_tile_empty) {
                    pos->absolute_tile_z -= 1;
                }
            }
        }

        // ================= MOVEMENT SIMULATION ====================== //

        game_entity* entity = &GameState->player;

        // [units]
        v2 input_direction = ControllerInput->LeftStickEnded.normalized();
        f32 input_strength = math::clamp(ControllerInput->LeftStickEnded.norm(), 0, 1);

        // [m/s^2]
        float32 acceleration_coefficient = 30.0f;
        if (ControllerInput->A.EndedDown) {
            acceleration_coefficient = 200.0f;
        }

        v2 acceleration = acceleration_coefficient * input_strength * input_direction;

        // [m/s^2] = [m/s] * [units] * [m/s^2]
        // @todo: why units do not add up?
        v2 friction_acceleration = (entity->velocity) * friction_coefficient * gravity_acceleration;

        v2 velocity = entity->velocity + (acceleration - friction_acceleration) * dt;
        v2 position = entity->position.relative_position_on_tile;

        v2 new_position = position + velocity * dt;

        // ================= COLLISION DETECTION ====================== //

        tile_map_position origin {
            entity->position.absolute_tile_x,
            entity->position.absolute_tile_y,
            entity->position.absolute_tile_z,
            0.0f, 0.0f
        };

        f32 min_x = min(position.x, new_position.x) - 0.5f * entity->hitbox.x;
        f32 min_y = min(position.y, new_position.y) - 0.5f * entity->hitbox.y;

        f32 max_x = max(position.x, new_position.x) + 0.5f * entity->hitbox.x;
        f32 max_y = max(position.y, new_position.y) + 0.5f * entity->hitbox.y;

        tile_map_position min_test_tile_position = origin;
        min_test_tile_position.relative_position_on_tile = v2{ min_x, min_y };
        min_test_tile_position = NormalizeTilemapPosition(tilemap, min_test_tile_position);

        tile_map_position max_test_tile_position = origin;
        max_test_tile_position.relative_position_on_tile = v2{ max_x, max_y };
        max_test_tile_position = NormalizeTilemapPosition(tilemap, max_test_tile_position);

        int32 min_tile_x = min_test_tile_position.absolute_tile_x;
        int32 min_tile_y = min_test_tile_position.absolute_tile_y;

        int32 max_tile_x = max_test_tile_position.absolute_tile_x;
        int32 max_tile_y = max_test_tile_position.absolute_tile_y;

        int32 abs_tile_z = entity->position.absolute_tile_z;

        v2 current_position = position;
        v2 current_velocity = velocity;
        v2 current_destination = new_position;
        f32 time_spent_moving = 0.0f;

        f32 original_move_distance = (new_position - position).length();

#define ASUKA_MAX_MOVE_TRIES 5
        for (int32 move_try = 0; move_try < ASUKA_MAX_MOVE_TRIES; move_try++) {
            v2 closest_destination = current_destination;
            v2 velocity_at_closest_destination = current_velocity;

            for (int32 abs_tile_y = min_tile_y; abs_tile_y <= max_tile_y; abs_tile_y++) {
                for (int32 abs_tile_x = min_tile_x; abs_tile_x <= max_tile_x; abs_tile_x++) {
                    tile_t tile_value = GetTileValue(tilemap, abs_tile_x, abs_tile_y, abs_tile_z);
                    if (IsTileValueEmpty(tile_value)) { continue; }

                    float32 tile_diameter_width = tilemap->tile_side_in_meters + entity->hitbox.x;
                    float32 tile_diameter_height = tilemap->tile_side_in_meters + entity->hitbox.y;

                    tile_map_position test_top_right_corner {
                        abs_tile_x, abs_tile_y, abs_tile_z,
                        0.5f * tile_diameter_width, 0.5f * tile_diameter_height
                    };

                    tile_map_position test_bottom_right_corner {
                        abs_tile_x, abs_tile_y, abs_tile_z,
                        0.5f * tile_diameter_width, -0.5f * tile_diameter_height
                    };

                    tile_map_position test_bottom_left_corner {
                        abs_tile_x, abs_tile_y, abs_tile_z,
                        -0.5f * tile_diameter_width, -0.5f * tile_diameter_height
                    };

                    tile_map_position test_top_left_corner {
                        abs_tile_x, abs_tile_y, abs_tile_z,
                        -0.5f * tile_diameter_width, 0.5f * tile_diameter_height
                    };

                    v2 vertices[4] = {
                        PositionDifference(tilemap, test_top_right_corner, origin),
                        PositionDifference(tilemap, test_bottom_right_corner, origin),
                        PositionDifference(tilemap, test_bottom_left_corner, origin),
                        PositionDifference(tilemap, test_top_left_corner, origin),
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
                                if ((res.intersection - current_position).length_2() < (closest_destination - current_position).length_2()) {
                                    closest_destination = res.intersection;
                                    velocity_at_closest_destination = math::project(current_velocity, wall); // wall * math::dot(current_velocity, wall) / wall.length_2();
                                }
                            }
                        }
                    }
                }
            }

            f32 move_distance = (closest_destination - current_position).length();
            if (move_distance > EPSILON) {
                f32 dt_prime = move_distance / current_velocity.length();
                time_spent_moving += dt_prime;
            }

            current_position = closest_destination;
            current_velocity = velocity_at_closest_destination;
            current_destination = current_position + current_velocity * (dt - time_spent_moving);
        }

        tile_map_position new_tilemap_position = entity->position;
        new_tilemap_position.relative_position_on_tile = current_position;

        entity->position = NormalizeTilemapPosition(tilemap, new_tilemap_position);
        entity->velocity = current_velocity;

        if (input_direction.length_2() > 0) {
            if (math::absolute(input_direction.x) > math::absolute(input_direction.y)) {
                if (input_direction.x > 0) {
                    entity->face_direction = PLAYER_FACE_RIGHT;
                } else {
                    entity->face_direction = PLAYER_FACE_LEFT;
                }
            } else {
                if (input_direction.y > 0) {
                    entity->face_direction = PLAYER_FACE_UP;
                } else {
                    entity->face_direction = PLAYER_FACE_DOWN;
                }
            }
        }
    }

    // Game_OutputSound(SoundBuffer, GameState);

    // ===================== RENDERING ===================== //

    // Rendering pink background to really see if there are some pixels I didn't drew
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    // RenderBitmap(Buffer, { 0, 0 }, { (float32)Buffer->Width, (f32)Buffer->Height }, &GameState->grass_texture);

    tile_chunk_position player_chunk_pos = GetChunkPosition(tilemap, GameState->player.position);

    int32 render_tiles_half_count_y = (int32)((f32)Buffer->Height / ((f32)tilemap->tile_side_in_meters * (f32)pixels_per_meter));
    int32 render_tiles_count_x = (int32)((f32)Buffer->Width / ((f32)tilemap->tile_side_in_meters * (f32)pixels_per_meter));

    int32 tile_side_in_pixels = (int32) (GameState->world->tilemap.tile_side_in_meters * pixels_per_meter);

    auto *player_p = &GameState->player.position;
    auto *camera_p = &GameState->camera_position;

#define DEBUG_CAMERA_FOLLOW_PLAYER (1 && ASUKA_DEBUG)

#if DEBUG_CAMERA_FOLLOW_PLAYER
    GameState->camera_position = *player_p;
#else
    int32 player_room_x = player_p->absolute_tile_x / room_width_in_tiles;
    int32 player_room_y = player_p->absolute_tile_y / room_height_in_tiles;

    GameState->camera_position = {};
    GameState->camera_position.absolute_tile_x = player_room_x * room_width_in_tiles + room_width_in_tiles / 2;
    GameState->camera_position.absolute_tile_y = player_room_y * room_height_in_tiles + room_height_in_tiles / 2;
    GameState->camera_position.absolute_tile_z = player_p->absolute_tile_z;
#endif

    for (int32 relative_row = -render_tiles_half_count_y; relative_row < render_tiles_half_count_y; relative_row++) {
        for (int32 relative_column = -render_tiles_count_x; relative_column < render_tiles_count_x; relative_column++) {

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

                    if (row == (int32)(player_chunk_pos.chunk_relative_y + player_chunk_pos.chunk_y * tilemap->tile_count_y) &&
                        column == (int32)(player_chunk_pos.chunk_relative_x + player_chunk_pos.chunk_x * tilemap->tile_count_x)) {
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
                    RenderBitmap(Buffer, upper_left_in_up_down_screen_pixel_coords, bottom_right_in_up_down_screen_pixel_coords, &GameState->wall_texture);
                    continue;

                    // TileColor = color24{ 0.2f, 0.3f, 0.2f };
                    // break;
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

    // ===================== RENDERING CHARACTERS ===================== //

    v2 player_position_in_bottom_up_screen_pixel_coordinates = {
        (f32) Buffer->Width / 2.0f +
        (player_p->absolute_tile_x - camera_p->absolute_tile_x) * tilemap->tile_side_in_meters * pixels_per_meter +
        (player_p->relative_position_on_tile.x - camera_p->relative_position_on_tile.x) * pixels_per_meter +
        0.5f * tilemap->tile_side_in_meters * pixels_per_meter,

        (f32) Buffer->Height / 2.0f +
        (player_p->absolute_tile_y - camera_p->absolute_tile_y) * tilemap->tile_side_in_meters * pixels_per_meter +
        (player_p->relative_position_on_tile.y - camera_p->relative_position_on_tile.y) * pixels_per_meter,
    };

    v2 player_position_in_top_bottom_screen_pixel_coordinates = {
        player_position_in_bottom_up_screen_pixel_coordinates.x,
        (f32)Buffer->Height - player_position_in_bottom_up_screen_pixel_coordinates.y,
    };

    v2 top_left = {
        player_position_in_top_bottom_screen_pixel_coordinates.x - 0.5f * character_dimensions.x * pixels_per_meter,
        player_position_in_top_bottom_screen_pixel_coordinates.y - 1.0f * character_dimensions.y * pixels_per_meter,
    };

    v2 bottom_right = {
        player_position_in_top_bottom_screen_pixel_coordinates.x + 0.5f * character_dimensions.x * pixels_per_meter,
        player_position_in_top_bottom_screen_pixel_coordinates.y
    };

    RenderRectangle(
        Buffer,
        player_position_in_top_bottom_screen_pixel_coordinates - 0.5f * GameState->player.hitbox * pixels_per_meter,
        player_position_in_top_bottom_screen_pixel_coordinates + 0.5f * GameState->player.hitbox * pixels_per_meter,
        color24{ 1.f, 1.f, 0.f });

    RenderRectangle(
        Buffer,
        player_position_in_top_bottom_screen_pixel_coordinates - v2{ 3.f, 3.f },
        player_position_in_top_bottom_screen_pixel_coordinates + v2{ 3.f, 3.f },
        color24{ 0.f, 0.f, 0.f });

    RenderBitmap(
        Buffer,
        top_left,
        v2{ (f32)Buffer->Width, (f32)Buffer->Height },
        &GameState->player_textures[GameState->player.face_direction]);

#if ASUKA_PLAYBACK_LOOP
    if (BorderVisible) {
        RenderBorder(Buffer, BorderWidth, BorderColor);
    }
#endif // ASUKA_PLAYBACK_LOOP
}
