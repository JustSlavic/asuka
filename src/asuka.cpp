#include "asuka.hpp"
#include <math.hpp>
#include <debug/casts.hpp>


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


GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);

    float32 dt = Input->dt;

#define TILEMAP_TILES_X 256
#define TILEMAP_TILES_Y 256

    uint32 tiles[TILEMAP_TILES_Y][TILEMAP_TILES_X] = {
        { 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
        { 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, },
        { 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
    };

    tile_chunk tilechunk {};
    tilechunk.tiles = (int32*) tiles;

    world w;
    w.tilemap.tile_side_in_meters = 1.0f; // [meters]

    w.tilemap.tilechunk_count_x = 1;
    w.tilemap.tilechunk_count_y = 1;
    w.tilemap.tilechunks = &tilechunk;

    // Tilechunks 256x256
    w.tilemap.chunk_shift = 8;
    w.tilemap.chunk_mask  = (1 << w.tilemap.chunk_shift) - 1;

    w.tilemap.tile_count_x = 256;
    w.tilemap.tile_count_y = 256;

    game_state* GameState = (game_state*)Memory->PermanentStorage;

    if (!Memory->IsInitialized) {
        GameState->player_position.absolute_tile_x = 3;
        GameState->player_position.absolute_tile_y = 3;

        GameState->player_position.relative_position_on_tile = { 0.0f, 0.0f }; // in meters relative to the tile

        const char *wav_filename = "piano2.wav";
        GameState->test_wav_file = load_wav_file(wav_filename);
        GameState->test_current_sound_cursor = 0;

        Memory->IsInitialized = true;
    }

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

    float32 pixels_per_meter = 30.f; // [pixels/m]

    v2  character_dimensions = { 0.75f, 1.0f }; // [m; m]
    f32 character_speed = 2.5f; // [m/s]
    f32 character_mass = 80.0f; // [kg]
    f32 gravity_acceleration = 9.8f; // [m/s^2]
    f32 friction_coefficient = 1.0f;

    {
        Game_ControllerInput* Input0 = GetController(Input, 0);
        Input0 = &Input->KeyboardController;

        // [units]
        v2 input_direction = v2{ Input0->StickLXEnded, Input0->StickLYEnded }.normalized();

        // [m/s^2]
        float32 acceleration_coefficient = 30.0f;
        if (Input0->A.EndedDown) {
            acceleration_coefficient = 120.0f;
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

        tile_map_position normalized_player_position = NormalizeTilemapPosition(&w.tilemap, temp_player_position);
        tile_map_position normalized_left_corner     = NormalizeTilemapPosition(&w.tilemap, player_left_corner);
        tile_map_position normalized_right_corner    = NormalizeTilemapPosition(&w.tilemap, player_right_corner);

        bool32 tile_is_valid = true;
        tile_is_valid &= IsWorldPointEmpty(&w.tilemap, temp_player_position);
        tile_is_valid &= IsWorldPointEmpty(&w.tilemap, normalized_left_corner);
        tile_is_valid &= IsWorldPointEmpty(&w.tilemap, normalized_right_corner);

        if (tile_is_valid) {
            GameState->player_position = normalized_player_position;
        }
    }

    // Game_OutputSound(SoundBuffer, GameState);

    // Rendering pink background to really see if there are some pixels I didn't drew
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    tile_chunk_position player_chunk_pos = GetChunkPosition(&w.tilemap, GameState->player_position.absolute_tile_x, GameState->player_position.absolute_tile_y);

    int32 player_absolute_tile_x = GameState->player_position.absolute_tile_x;
    int32 player_absolute_tile_y = GameState->player_position.absolute_tile_y;

    int32 center_x = 8 + 8;
    int32 center_y = 4 + 5;

    int32 row_border = 5 + 3;
    int32 col_border = 8 + 7;

    int32 tile_side_in_pixels = (int32) (w.tilemap.tile_side_in_meters * pixels_per_meter);

    for (int32 relative_row = -row_border; relative_row < row_border; relative_row++) {
        for (int32 relative_column = -col_border; relative_column < col_border; relative_column++) {

            int32 row = player_absolute_tile_y + relative_row;
            int32 column = player_absolute_tile_x + relative_column;

            int32 TileId = GetTileValue(&w.tilemap, column, row);

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
            if (TileId == -1) {
                TileColor = color24{ 1.0f };
            }
            if (TileId == 1) {
                TileColor = color24{ 0.2f, 0.3f, 0.2f };
            }

            if (row == (int32)player_chunk_pos.chunk_relative_y && column == (int32)player_chunk_pos.chunk_relative_x) {
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
        vector2{ (float32)center_x, (float32)center_y } * w.tilemap.tile_side_in_meters +
        vector2{ 0.5f, 0.5f } * w.tilemap.tile_side_in_meters;
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
