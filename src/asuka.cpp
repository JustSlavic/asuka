#include "asuka.hpp"
#include <math.hpp>
#include <debug/casts.hpp>


void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, Game_State* GameState) {
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


inline tile_chunk* GetTileChunk(World* world, int32 tilechunk_x, int32 tilechunk_y) {
    // Allow negative coordinates of chunks so that chunks can grow
    // from the center of the map in any direction.

    // @todo: what asserts should I use here?
    // ASSERT(world->tilechunk_count_x - tilechunk_x > 0);
    // ASSERT(world->tilechunk_count_y - tilechunk_y > 0);

    tile_chunk* result = NULL;

    if (tilechunk_x >= 0 && tilechunk_x < (int32)world->tilechunk_count_x &&
        tilechunk_y >= 0 && tilechunk_y < (int32)world->tilechunk_count_y)
    {
        result = &world->tilechunks[tilechunk_y * world->tilechunk_count_x + tilechunk_x];
    }

    return result;
}


inline uint32 GetTileValue_Unchecked(World* world, tile_chunk* tilechunk, uint32 tile_x, uint32 tile_y) {
    ASSERT(tile_x < world->tile_count_x);
    ASSERT(tile_y < world->tile_count_y);

    uint32 result = tilechunk->tiles[tile_y*world->tile_count_x + tile_x];
    return result;
}


INTERNAL_FUNCTION
void NormalizeCoordinate(World *world, i32 *tile, f32 *relative_coord) {
    f32 coord = *relative_coord + 0.5f;
    i32 offset = floor_to_int32(coord / world->tile_side_in_meters);

    *tile += offset;
    *relative_coord -= offset * world->tile_side_in_meters;

    ASSERT(*relative_coord <=  (0.5f * world->tile_side_in_meters));
    ASSERT(*relative_coord >= -(0.5f * world->tile_side_in_meters));
}


INTERNAL_FUNCTION
WorldPosition NormalizeWorldPosition(World* world, WorldPosition position) {
    WorldPosition result = position;

    float32 tile_top   = -0.5f * world->tile_side_in_meters;
    float32 tile_bottom = 0.5f * world->tile_side_in_meters;

    float32 tile_left = -0.5f * world->tile_side_in_meters;
    float32 tile_right = 0.5f * world->tile_side_in_meters;

    NormalizeCoordinate(world, &result.absolute_tile_x, &result.relative_position_on_tile.x);
    NormalizeCoordinate(world, &result.absolute_tile_y, &result.relative_position_on_tile.y);

    // ASSERT((0 <= result.tile.x) && (result.tile.x < world->tile_count_x));
    // ASSERT((0 <= result.tile.y) && (result.tile.y < world->tile_count_y));

    // ASSERT((tile_left <= result.relative_position_on_tile.x) && (result.relative_position_on_tile.x < tile_right));
    // ASSERT((tile_top  <= result.relative_position_on_tile.y) && (result.relative_position_on_tile.y < tile_bottom));

    return result;
}


INTERNAL_FUNCTION
tile_chunk_position GetChunkPosition(World *world, int32 abs_tile_x, int32 abs_tile_y) {
    tile_chunk_position result {};

    result.tilechunk_x = abs_tile_x >> world->chunk_shift;
    result.tilechunk_y = abs_tile_y >> world->chunk_shift;

    result.chunk_relative_x = abs_tile_x & world->chunk_mask;
    result.chunk_relative_y = abs_tile_y & world->chunk_mask;

    return result;
}


INTERNAL_FUNCTION
int32 GetTileValue(World* world, int32 abs_tile_x, int32 abs_tile_y) {
    int32 result = -1;

    tile_chunk_position chunk_pos = GetChunkPosition(world, abs_tile_x, abs_tile_y);
    tile_chunk *chunk = GetTileChunk(world, chunk_pos.tilechunk_x, chunk_pos.tilechunk_y);

    if (chunk) {
        result = GetTileValue_Unchecked(world, chunk, chunk_pos.chunk_relative_x, chunk_pos.chunk_relative_y);
    }

    return result;
}


INTERNAL_FUNCTION
bool32 IsWorldPointEmpty(World *world, WorldPosition norm_position) {
    bool32 is_empty = false;

    int32 tile_value = GetTileValue(world, norm_position.absolute_tile_x, norm_position.absolute_tile_y);

    is_empty = (tile_value == 0);
    return is_empty;
}


GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    ASSERT(sizeof(Game_State) <= Memory->PermanentStorageSize);

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

    World world;
    world.tile_side_in_meters = 1; // [meters]

    world.tilechunk_count_x = 1;
    world.tilechunk_count_y = 1;
    world.tilechunks = &tilechunk;

    // Tilechunks 256x256
    world.chunk_shift = 8;
    world.chunk_mask  = (1 << world.chunk_shift) - 1;

    world.tile_count_x = 256;
    world.tile_count_y = 256;

    Game_State* GameState = (Game_State*)Memory->PermanentStorage;

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

    float32 pixels_per_meter = 60.f; // [pixels/m]

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

        WorldPosition temp_player_position = GameState->player_position;
        temp_player_position.relative_position_on_tile = new_position;

        WorldPosition player_left_corner = temp_player_position;
        player_left_corner.relative_position_on_tile.x -= character_dimensions.x * 0.5f;

        WorldPosition player_right_corner = temp_player_position;
        player_right_corner.relative_position_on_tile.x += character_dimensions.x * 0.5f;

        WorldPosition normalized_player_position = NormalizeWorldPosition(&world, temp_player_position);
        WorldPosition normalized_left_corner     = NormalizeWorldPosition(&world, player_left_corner);
        WorldPosition normalized_right_corner    = NormalizeWorldPosition(&world, player_right_corner);

        bool32 tile_is_valid = true;
        tile_is_valid &= IsWorldPointEmpty(&world, temp_player_position);
        tile_is_valid &= IsWorldPointEmpty(&world, normalized_left_corner);
        tile_is_valid &= IsWorldPointEmpty(&world, normalized_right_corner);

        if (tile_is_valid) {
            GameState->player_position = normalized_player_position;
        }
    }

    // Game_OutputSound(SoundBuffer, GameState);

    // Rendering pink background to really see if there are some pixels I didn't drew
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    tile_chunk_position player_chunk_pos = GetChunkPosition(&world, GameState->player_position.absolute_tile_x, GameState->player_position.absolute_tile_y);

    int32 player_absolute_tile_x = GameState->player_position.absolute_tile_x;
    int32 player_absolute_tile_y = GameState->player_position.absolute_tile_y;

    int32 center_x = 8;
    int32 center_y = 4;

    int32 row_border = 3;
    int32 col_border = 3;

    int32 tile_side_in_pixels = (int32) (world.tile_side_in_meters * pixels_per_meter);

    for (int32 relative_row = -row_border; relative_row < row_border; relative_row++) {
        for (int32 relative_column = -col_border; relative_column < col_border; relative_column++) {

            int32 row = player_absolute_tile_y + relative_row;
            int32 column = player_absolute_tile_x + relative_column;

            int32 TileId = GetTileValue(&world, column, row);

            float32 TileBottomLeftX = (float32) (relative_column + center_x) * tile_side_in_pixels;
            float32 TileBottomLeftY = (float32) Buffer->Height - (float32) (relative_row + center_y) * tile_side_in_pixels;

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
        vector2{ (float32)center_x, (float32)center_y } * world.tile_side_in_meters +
        GameState->player_position.relative_position_on_tile + vector2{ 0.5f, 0.5f } * world.tile_side_in_meters;
    absolute_player_position.y = Buffer->Height / pixels_per_meter - absolute_player_position.y;

    vector2 top_left = {
        absolute_player_position.x - 0.5f * character_dimensions.x,
        absolute_player_position.y - 1.0f * character_dimensions.y,
    };

    vector2 bottom_right = {
        absolute_player_position.x + 0.5f * character_dimensions.x,
        absolute_player_position.y
    };

    // vector2 relative_player_position =
    //     upcast_to_vector2(GameState->player_position.tile) * world.tile_side_in_meters +
    //     GameState->player_position.relative_position_on_tile + vector2{ 0.5f, 0.5f } * world.tile_side_in_meters;

    // relative_player_position.y = world.tile_count_y * world.tile_side_in_meters - relative_player_position.y;

    // vector2 top_left = {
    //     relative_player_position.x - 0.5f * character_dimensions.x,
    //     relative_player_position.y - 1.0f * character_dimensions.y,
    // };

    // vector2 bottom_right = {
    //     relative_player_position.x + 0.5f * character_dimensions.x,
    //     relative_player_position.y
    // };

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
