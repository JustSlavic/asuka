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
void RenderBMP(
    Game_OffscreenBuffer* buffer,
    vector2 top_left, vector2 bottom_right,
    bmp_file_contents *bmp,
    bool32 stroke = false)
{
    vector2i tl = round_to_vector2i(top_left);
    vector2i br = round_to_vector2i(bottom_right);

    if (tl.x < 0) tl.x = 0;
    if (tl.y < 0) tl.y = 0;
    if (br.x > buffer->Width)  br.x = buffer->Width;
    if (br.y > buffer->Height) br.y = buffer->Height;

    vector2i dimensions = br - tl;

    uint8* Row = (uint8*)buffer->Memory + tl.y*buffer->Pitch + tl.x*buffer->BytesPerPixel;
    uint8* bmp_pixel_row = (uint8 *) bmp->pixels;

    for (int y = 0; y < dimensions.y; y++) {
        uint32* Pixel = (uint32*) Row;
        uint8 * bmp_pixel = bmp_pixel_row;

        for (int x = 0; x < dimensions.x; x++) {
            if (stroke && (x == 0 || y == 0)) {
                *Pixel = 0;
            } else {
                uint8 red = *bmp_pixel++;
                uint8 green = *bmp_pixel++;
                uint8 blue = *bmp_pixel++;

                *Pixel = (red) | (green << 8) | (blue << 16);
            }
            Pixel++;
        }

        Row += buffer->Pitch;
        bmp_pixel_row += bmp->width * bmp->bytes_per_pixel;
    }
}


INTERNAL_FUNCTION
void RenderRectangle(
    Game_OffscreenBuffer* buffer,
    vector2 top_left, vector2 bottom_right,
    color24 color,
    bool32 stroke = false)
{
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


#ifdef ASUKA_DEBUG
INTERNAL_FUNCTION
void RenderBorder(Game_OffscreenBuffer* Buffer, uint32 Width, color24 Color) {
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Width}, {(float32)Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {(float32)Buffer->Width - Width, (float32)Width}, {(float32)Buffer->Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Buffer->Height - Width}, {(float32)Buffer->Width, (float32)Buffer->Height}, Color);
}
#endif


#include <time.h>
#include <stdlib.h>
GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);

    float32 dt = Input->dt;

    game_state* GameState = (game_state*)Memory->PermanentStorage;

    if (!Memory->IsInitialized) {
        srand((unsigned int)time(NULL));
        GameState->player_position.absolute_tile_x = 3;
        GameState->player_position.absolute_tile_y = 3;
        GameState->player_position.absolute_tile_z = 0;

        GameState->player_position.relative_position_on_tile = { 0.0f, 0.0f }; // in meters relative to the tile

        const char *wav_filename = "piano2.wav";
        GameState->test_wav_file = load_wav_file(wav_filename);
        GameState->test_current_sound_cursor = 0;

        const char *bmp_filename = "tree_60x60.bmp";
        GameState->test_bmp_file = load_bmp_file(bmp_filename);

        memory_arena *arena = &GameState->world_arena;

        initialize_arena(
            arena,
            (uint8 *) Memory->PermanentStorage + sizeof(game_state),
            Memory->PermanentStorageSize - sizeof(game_state));

        // Generate tilemap
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


        i32 room_width = 16;
        i32 room_height = 9;

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

            for (i32 tile_y = 0; tile_y < room_height; tile_y++) {
                for (i32 tile_x = 0; tile_x < room_width; tile_x++) {
                    u32 x = screen_x * room_width  + tile_x;
                    u32 y = screen_y * room_height + tile_y;
                    u32 z = screen_z;

                    tile_t tile_value = TILE_FREE;

                    // bottom wall
                    if (tile_y == 0) {
                        tile_value = TILE_WALL;

                        if ((previous_choice == GEN_UP) && (tile_x == room_width / 2 || tile_x == (room_width / 2 - 1))) {
                            tile_value = TILE_FREE;
                        }
                    }

                    // upper wall
                    if (tile_y == room_height - 1) {
                        tile_value = TILE_WALL;

                        if ((choice == GEN_UP) && (tile_x == room_width / 2 || tile_x == (room_width / 2 - 1))) {
                            tile_value = TILE_FREE;
                        }
                    }

                    // left wall
                    if (tile_x == 0) {
                        tile_value = TILE_WALL;

                        if ((previous_choice == GEN_RIGHT && (tile_y == room_height / 2))) {
                            tile_value = TILE_FREE;
                        }
                    }

                    // right wall
                    if (tile_x == room_width - 1) {
                        tile_value = TILE_WALL;

                        if ((choice == GEN_RIGHT && (tile_y == room_height / 2))) {
                            tile_value = TILE_FREE;
                        }
                    }

                    if ((choice == GEN_FLOOR_UP || previous_choice == GEN_FLOOR_DOWN) && (tile_x == 6 && tile_y == 6)) {
                        tile_value = TILE_DOOR_UP;
                    }

                    if ((choice == GEN_FLOOR_DOWN || previous_choice == GEN_FLOOR_UP) && (tile_x == 7 && tile_y == 6)) {
                        tile_value = TILE_DOOR_DOWN;
                    }

                    if ((screen_idx == screens_count - 1) && (tile_x == room_width - 2) && (tile_y == room_height - 2)) {
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

#ifdef ASUKA_PLAYBACK_LOOP
    color24 BorderColor {};
    uint32 BorderWidth = 5;
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
        Game_ControllerInput* Input0 = GetController(Input, 0);
        Input0 = &Input->KeyboardController;

        if (Input0->Y.EndedDown) {
            Memory->IsInitialized = false;
        }

        if (Input0->X.EndedDown) {
            tile_map_position *pos = &GameState->player_position;
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

        // [units]
        v2 input_direction = v2{ Input0->StickLXEnded, Input0->StickLYEnded }.normalized();

        // [m/s^2]
        float32 acceleration_coefficient = 30.0f;
        if (Input0->A.EndedDown) {
            acceleration_coefficient = 200.0f;
        }

        v2 acceleration = acceleration_coefficient * input_direction;

        // [m/s^2] = [m/s] * [units] * [m/s^2]
        // @todo: why units do not add up?
        v2 friction_acceleration = (GameState->player_velocity) * friction_coefficient * gravity_acceleration;

        // [m/s] = [m/s] + ([m/s^2] - [m/s^2]) * [s]
        GameState->player_velocity = GameState->player_velocity + (acceleration - friction_acceleration) * dt;

        // [m] = [m] + [m/s] * [s]
        v2 new_position =
            GameState->player_position.relative_position_on_tile +
            GameState->player_velocity * dt;

        tile_map_position temp_player_position = GameState->player_position;
        temp_player_position.relative_position_on_tile = new_position;

        tile_map_position player_left_corner = temp_player_position;
        player_left_corner.relative_position_on_tile.x -= character_dimensions.x * 0.5f;

        tile_map_position player_right_corner = temp_player_position;
        player_right_corner.relative_position_on_tile.x += character_dimensions.x * 0.5f;

        tile_map_position normalized_player_position = NormalizeTilemapPosition(&GameState->world->tilemap, temp_player_position);
        tile_map_position normalized_left_corner     = NormalizeTilemapPosition(&GameState->world->tilemap, player_left_corner);
        tile_map_position normalized_right_corner    = NormalizeTilemapPosition(&GameState->world->tilemap, player_right_corner);

        bool32 tile_is_valid = true;
        tile_is_valid &= IsWorldPointEmpty(&GameState->world->tilemap, temp_player_position);
        tile_is_valid &= IsWorldPointEmpty(&GameState->world->tilemap, normalized_left_corner);
        tile_is_valid &= IsWorldPointEmpty(&GameState->world->tilemap, normalized_right_corner);

        if (tile_is_valid) {
            GameState->player_position = normalized_player_position;
        }
    }

    // Game_OutputSound(SoundBuffer, GameState);

    // Rendering pink background to really see if there are some pixels I didn't drew
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    tile_chunk_position player_chunk_pos = GetChunkPosition(
        tilemap,
        GameState->player_position.absolute_tile_x,
        GameState->player_position.absolute_tile_y,
        GameState->player_position.absolute_tile_z);

    int32 player_absolute_tile_x = GameState->player_position.absolute_tile_x;
    int32 player_absolute_tile_y = GameState->player_position.absolute_tile_y;
    int32 player_absolute_tile_z = GameState->player_position.absolute_tile_z;

    int32 center_x = 8; // + 8 + 20;
    int32 center_y = 4; // + 5 + 20;

    int32 row_border = 6; // + 3 + 20;
    int32 col_border = 9; // + 7 + 20;

    int32 tile_side_in_pixels = (int32) (GameState->world->tilemap.tile_side_in_meters * pixels_per_meter);

    for (int32 relative_row = -row_border; relative_row < row_border; relative_row++) {
        for (int32 relative_column = -col_border; relative_column < col_border; relative_column++) {

            int32 row = player_absolute_tile_y + relative_row;
            int32 column = player_absolute_tile_x + relative_column;
            int32 level = player_absolute_tile_z;

            // int32 abs_x = (column / tilemap->tile_count_x << tilemap->chunk_shift) | (column % tilemap->tile_count_x);
            // int32 abs_y = (row / tilemap->tile_count_y << tilemap->chunk_shift) | (row % tilemap->tile_count_y);

            tile_t TileId = GetTileValue(tilemap, column, row, level);

            float32 TileBottomLeftX = (float32) (relative_column + center_x) * tile_side_in_pixels - GameState->player_position.relative_position_on_tile.x * pixels_per_meter;
            float32 TileBottomLeftY = (float32) Buffer->Height - (float32) (relative_row + center_y) * tile_side_in_pixels + GameState->player_position.relative_position_on_tile.y * pixels_per_meter;

            vector2 TileUpperLeft {
                TileBottomLeftX,
                TileBottomLeftY - tile_side_in_pixels,
            };

            vector2 TileBottomRight {
                TileUpperLeft.x + tile_side_in_pixels,
                TileUpperLeft.y + tile_side_in_pixels,
            };

            auto TileColor = color24{ 0.5f, 0.5f, 0.5f };

            if ((row % tilemap->tile_count_y == 0 || column % tilemap->tile_count_x == 0)) {
                TileColor = color24{ 0.0f, 0.4f, 0.8f };
            }

            if (TileId == TILE_INVALID) {
                TileColor = color24{ 1.0f };
            } else if (TileId == TILE_WALL) {
                // TileColor = color24{ 0.2f, 0.3f, 0.2f };
                RenderBMP(Buffer, TileUpperLeft, TileBottomRight, &GameState->test_bmp_file, true);
                continue;
            } else if (TileId == TILE_DOOR_UP) {
                TileColor = color24{ 0.8f, 0.8f, 0.8f };
            } else if (TileId == TILE_DOOR_DOWN) {
                TileColor = color24{ 0.3f, 0.3f, 0.3f };
            } else if (TileId == TILE_WIN) {
                STATIC_VARIABLE f32 t_color = 0.0f;
                f32 tmp_r = sinf(pi_f32 * t_color - 0.5f * pi_f32);
                f32 tmp_g = sinf(pi_f32 * t_color - 5.0f * pi_f32 / 6.0f);
                f32 tmp_b = sinf(pi_f32 * t_color - pi_f32 / 6.0f);

                f32 normalization_constant = 2.0f / 3.0f;
                normalization_constant = 1.0f;

                color24 tile_win_color = color24{
                    tmp_r * tmp_r * normalization_constant,
                    tmp_g * tmp_g * normalization_constant,
                    tmp_b * tmp_b * normalization_constant,
                };
                t_color += 0.01f;

                if (t_color > 1.0f) { t_color -= 1.0f; }

                TileColor = tile_win_color;
            }

            if (row == (int32)(player_chunk_pos.chunk_relative_y + player_chunk_pos.chunk_y * tilemap->tile_count_y) &&
                column == (int32)(player_chunk_pos.chunk_relative_x + player_chunk_pos.chunk_x * tilemap->tile_count_x)) {
                TileColor = color24{ 0.8f, 0.4f, 0.0f };
            }

            RenderRectangle(
                Buffer,
                TileUpperLeft,
                TileBottomRight,
                TileColor, true);
        }
    }

    vector2 absolute_player_position =
        vector2{ (float32)center_x, (float32)center_y } * tilemap->tile_side_in_meters +
        vector2{ 0.5f, 0.5f } * tilemap->tile_side_in_meters;
    absolute_player_position.y = Buffer->Height / pixels_per_meter - absolute_player_position.y;

    vector2 top_left = {
        absolute_player_position.x - 0.5f * character_dimensions.x,
        absolute_player_position.y - 1.0f * character_dimensions.y,
    };

    vector2 bottom_right = {
        absolute_player_position.x + 0.5f * character_dimensions.x,
        absolute_player_position.y
    };

    RenderRectangle(
        Buffer,
        top_left * pixels_per_meter,
        bottom_right * pixels_per_meter,
        color24{ 0.9f, 0.9f, 0.2f });

    RenderRectangle(
        Buffer,
        absolute_player_position * pixels_per_meter - vector2{ 3.f, 3.f },
        absolute_player_position * pixels_per_meter + vector2{ 3.f, 3.f },
        color24{ 0.f, 0.f, 0.f });

#ifdef ASUKA_PLAYBACK_LOOP
    if (BorderVisible) {
        RenderBorder(Buffer, BorderWidth, BorderColor);
    }
#endif // ASUKA_PLAYBACK_LOOP
}
