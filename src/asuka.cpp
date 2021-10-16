#include "asuka.hpp"
#include <math.hpp>
#include <debug/casts.hpp>


typedef int16 sound_sample_t;


void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, Game_State* GameState) {
    sound_sample_t* SampleOut = SoundBuffer->Samples;

    for (int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
        sound_sample_t SampleValue = 0;

        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
    }
}


static void RenderRectangle(
    Game_OffscreenBuffer* buffer,
    math::vector2 top_left, math::vector2 bottom_right,
    math::color24 color)
{
    math::vector2i tl = round_vector2(top_left);
    math::vector2i br = round_vector2(bottom_right);

    if (tl.x < 0) tl.x = 0;
    if (tl.y < 0) tl.y = 0;
    if (br.x > buffer->Width)  br.x = buffer->Width;
    if (br.y > buffer->Height) br.y = buffer->Height;

    math::vector2i dimensions = br - tl;

    uint8* Row = (uint8*)buffer->Memory + tl.y*buffer->Pitch + tl.x*buffer->BytesPerPixel;

    for (int y = 0; y < dimensions.y; y++) {
        uint32* Pixel = (uint32*) Row;

        for (int x = 0; x < dimensions.x; x++) {
            *Pixel = pack_to_uint32(color);
            Pixel++;
        }

        Row += buffer->Pitch;
    }
}


#ifdef ASUKA_DEBUG
static void RenderBorder(Game_OffscreenBuffer* Buffer, uint32 Width, math::color24 Color) {
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Width}, {(float32)Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {(float32)Buffer->Width - Width, (float32)Width}, {(float32)Buffer->Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Buffer->Height - Width}, {(float32)Buffer->Width, (float32)Buffer->Height}, Color);
}
#endif


inline Tilemap* GetTilemap(Worldmap* world, int32 tilemap_x, int32 tilemap_y) {
    Tilemap* result = 0;

    if (tilemap_x >= 0 && tilemap_x < world->tilemap_count_x &&
        tilemap_y >= 0 && tilemap_y < world->tilemap_count_y)
    {
        result = &world->tilemaps[tilemap_y * world->tilemap_count_x + tilemap_x];
    }

    return result;
}


inline int32 GetTileValue_Unchecked(Worldmap* world, Tilemap* tilemap, int32 tile_x, int32 tile_y) {
    int32 result = tilemap->tiles[tile_y*world->tile_count_x + tile_x];
    return result;
}


static bool32 IsTilemapPointEmpty(Worldmap* world, Tilemap *tilemap, float32 x, float32 y) {
    bool32 result = false;

    if ((world != NULL) && (tilemap != NULL)) {
        int32 tile_x = truncate_cast_to_int32(x / world->tile_width);
        int32 tile_y = truncate_cast_to_int32(y / world->tile_height);

        if (x >= 0 && y >= 0 &&
            tile_x >= 0 && tile_x < world->tile_count_x &&
            tile_y >= 0 && tile_y < world->tile_count_y)
        {
            int32 tile_value = GetTileValue_Unchecked(world, tilemap, tile_x, tile_y);
            result = (tile_value == 0);
        }
    }

    return result;
}


static NormalizedWorldPosition NormalizeWorldPosition(Worldmap* world, WorldPosition position) {
    NormalizedWorldPosition result {};

    result.tilemap_x = position.tilemap_x;
    result.tilemap_y = position.tilemap_y;

    result.tile_x = math::floor_to_int32(position.x / world->tile_width);
    result.tile_y = math::floor_to_int32(position.y / world->tile_height);

    result.x = position.x - result.tile_x * world->tile_width;
    result.y = position.y - result.tile_y * world->tile_height;

    if (result.tile_x < 0) {
        result.tile_x += world->tile_count_x;
        result.tilemap_x = position.tilemap_x - 1;
    }

    if (result.tile_y < 0) {
        result.tile_y += world->tile_count_y;
        result.tilemap_y = position.tilemap_y - 1;
    }

    if (result.tile_x >= world->tile_count_x) {
        result.tile_x -= world->tile_count_x;
        result.tilemap_x = position.tilemap_x + 1;
    }

    if (result.tile_y >= world->tile_count_y) {
        result.tile_y -= world->tile_count_y;
        result.tilemap_y = position.tilemap_y + 1;
    }

    if (result.tilemap_y == 2) {
        int x = 0;
    }

    return result;
}


static bool32 IsWorldPointEmpty(Worldmap *world, WorldPosition position) {
    bool32 result = false;

    NormalizedWorldPosition norm_position = NormalizeWorldPosition(world, position);

    Tilemap* tilemap = GetTilemap(world, norm_position.tilemap_x, norm_position.tilemap_y);
    if (tilemap) {
        int32 tile_value = GetTileValue_Unchecked(world, tilemap, norm_position.tile_x, norm_position.tile_y);

        result = (tile_value == 0);
    }

    return result;
}


GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    ASSERT(sizeof(Game_State) <= Memory->PermanentStorageSize);

#define TILEMAP_TILES_X 16
#define TILEMAP_TILES_Y 9

    uint32 tiles_00[TILEMAP_TILES_Y][TILEMAP_TILES_X] = {
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 1,  1, 0, 1, 1,  1, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 1, 0,  1, 0, 1, 0,  1, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 0,  1, 0, 1, 0,  1, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 1,  1, 0, 1, 1,  1, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, },
        { 1, 1, 1, 1,  1, 1, 1, 0,  0, 1, 1, 1,  1, 1, 1, 1, },
    };

    uint32 tiles_01[TILEMAP_TILES_Y][TILEMAP_TILES_X] = {
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 1,  1, 0, 0, 1,  0, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 1, 0,  1, 0, 0, 1,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 0,  1, 0, 0, 1,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 1,  1, 0, 0, 1,  0, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 1, 1, 1,  1, 1, 1, 0,  0, 1, 1, 1,  1, 1, 1, 1, },
    };

    uint32 tiles_10[TILEMAP_TILES_Y][TILEMAP_TILES_X] = {
        { 1, 1, 1, 1,  1, 1, 1, 0,  0, 1, 1, 1,  1, 1, 1, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 1,  1, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 0,  1, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 0,  1, 0, 0, 0, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 1,  1, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
    };

    uint32 tiles_11[TILEMAP_TILES_Y][TILEMAP_TILES_X] = {
        { 1, 1, 1, 1,  1, 1, 1, 0,  0, 1, 1, 1,  1, 1, 1, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  0, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  0, 0, 0, 1, },
        { 0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  0, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
    };

    Tilemap tilemaps[2][2];

    tilemaps[0][0].tiles = (int32 *) tiles_00;

    tilemaps[0][1] = tilemaps[0][0];
    tilemaps[0][1].tiles = (int32 *) tiles_01;

    tilemaps[1][0] = tilemaps[0][0];
    tilemaps[1][0].tiles = (int32 *) tiles_10;

    tilemaps[1][1] = tilemaps[0][0];
    tilemaps[1][1].tiles = (int32 *) tiles_11;


    Worldmap world;
    world.tile_width = 60; // [pixels]
    world.tile_height = 60; // [pixels]
    world.tile_count_x = TILEMAP_TILES_X;
    world.tile_count_y = TILEMAP_TILES_Y;
    world.tilemap_count_x = 2;
    world.tilemap_count_y = 2;
    world.tilemaps = (Tilemap*) tilemaps;

    Game_State* GameState = (Game_State*)Memory->PermanentStorage;

    if (!Memory->IsInitialized) {
        GameState->character_tilemap = { 0, 0 };
        GameState->character_position = { 100, 100 }; // in pixels

        Memory->IsInitialized = true;
    }

#ifdef ASUKA_PLAYBACK_LOOP
    math::color24 BorderColor {};
    uint32 BorderWidth {};
    bool32 BorderVisible {};

    switch (Input->PlaybackLoopState) {
        case PLAYBACK_LOOP_IDLE: {
            BorderVisible = false;
            break;
        }
        case PLAYBACK_LOOP_RECORDING: {
            BorderVisible = true;
            BorderWidth = 10;
            BorderColor = math::color24{1.f, 1.f, 0.f};
            break;
        }
        case PLAYBACK_LOOP_PLAYBACK: {
            BorderVisible = true;
            BorderWidth = 10;
            BorderColor = math::color24{0.f, 1.f, 0.f};
            break;
        }
    }
#endif // ASUKA_PLAYBACK_LOOP

    Tilemap* current_tilemap = GetTilemap(&world, GameState->character_tilemap.x, GameState->character_tilemap.y);
    ASSERT(current_tilemap);

    float32 character_width = 30;  // [pixels]
    float32 character_height = 30; // [pixels]
    float32 character_speed = 100; // [pixels/second]

    {
        Game_ControllerInput* Input0 = GetController(Input, 0);

        float32 new_position_x = GameState->character_position.x + truncate_cast_to_int32(Input0->StickLXEnded * character_speed * Input->dt);
        float32 new_position_y = GameState->character_position.y - truncate_cast_to_int32(Input0->StickLYEnded * character_speed * Input->dt);

        WorldPosition player_position = {
            GameState->character_tilemap.x,
            GameState->character_tilemap.y,
            new_position_x,
            new_position_y,
        };

        WorldPosition player_left_corner = player_position;
        player_left_corner.x -= character_width*0.5f;

        WorldPosition player_right_corner = player_position;
        player_right_corner.x += character_width*0.5f;

        NormalizedWorldPosition normalized_player_position = NormalizeWorldPosition(&world, player_position);

        bool32 tile_is_valid = true;
        tile_is_valid &= IsWorldPointEmpty(&world, player_left_corner);
        tile_is_valid &= IsWorldPointEmpty(&world, player_right_corner);

        if (tile_is_valid) {
            GameState->character_position = {
                normalized_player_position.tile_x * world.tile_width + normalized_player_position.x,
                normalized_player_position.tile_y * world.tile_height + normalized_player_position.y,
            };
            GameState->character_tilemap.x = normalized_player_position.tilemap_x;
            GameState->character_tilemap.y = normalized_player_position.tilemap_y;
        }
    }

    Game_OutputSound(SoundBuffer, GameState);

    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    int32 character_tile_x = truncate_cast_to_int32(GameState->character_position.x / world.tile_width);
    int32 character_tile_y = truncate_cast_to_int32(GameState->character_position.y / world.tile_height);
    for (int Column = 0; Column < TILEMAP_TILES_Y; Column++) {
        for (int Row = 0; Row < TILEMAP_TILES_X; Row++) {
            int32 TileId = GetTileValue_Unchecked(&world, current_tilemap, Row, Column);

            float TileX = ((float32)Row)*world.tile_width;
            float TileY = ((float32)Column)*world.tile_height;

            auto TileColor = math::color24{ 0.5f, 0.5f, 0.5f };
            if (TileId == 1) {
                TileColor = math::color24{ 0.2f, 0.3f, 0.2f };
            }

            if (Column == character_tile_y && Row == character_tile_x) {
                TileColor = math::color24{ 0.8f, 0.4f, 0.0f };
            }

            RenderRectangle(Buffer, {TileX, TileY}, {TileX + world.tile_width, TileY + world.tile_height}, TileColor);
        }
    }

    RenderRectangle(
        Buffer,
        GameState->character_position - math::vector2{character_width*0.5f, character_height},
        GameState->character_position + math::vector2{character_width*0.5f, 0.f},
        math::color24{0.9f, 0.9f, 0.2f});

#ifdef ASUKA_PLAYBACK_LOOP
    if (BorderVisible) {
        RenderBorder(Buffer, BorderWidth, BorderColor);
    }
#endif // ASUKA_PLAYBACK_LOOP
}
