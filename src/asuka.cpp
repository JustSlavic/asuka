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

INTERNAL_FUNCTION
void RenderRectangle(
    Game_OffscreenBuffer* buffer,
    math::vector2 top_left, math::vector2 bottom_right,
    math::color24 color)
{
    math::vector2i tl = round_to_vector2i(top_left);
    math::vector2i br = round_to_vector2i(bottom_right);

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
INTERNAL_FUNCTION
void RenderBorder(Game_OffscreenBuffer* Buffer, uint32 Width, math::color24 Color) {
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Width}, {(float32)Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {(float32)Buffer->Width - Width, (float32)Width}, {(float32)Buffer->Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Buffer->Height - Width}, {(float32)Buffer->Width, (float32)Buffer->Height}, Color);
}
#endif


inline Tilemap* GetTilemap(Worldmap* world, math::vector2i tilemap_coords) {
    Tilemap* result = 0;

    if (tilemap_coords.x >= 0 && tilemap_coords.x < world->tilemap_count_x &&
        tilemap_coords.y >= 0 && tilemap_coords.y < world->tilemap_count_y)
    {
        result = &world->tilemaps[tilemap_coords.y * world->tilemap_count_x + tilemap_coords.x];
    }

    return result;
}


inline int32 GetTileValue_Unchecked(Worldmap* world, Tilemap* tilemap, math::vector2i tile) {
    int32 result = tilemap->tiles[tile.y*world->tile_count_x + tile.x];
    return result;
}


INTERNAL_FUNCTION
WorldPosition NormalizeWorldPosition(Worldmap* world, WorldPosition position) {
    WorldPosition result = position;

    if (result.relative_position_on_tile.x < 0) {
        result.relative_position_on_tile.x += world->tile_side_in_meters;
        result.tile.x -= 1;

        if (result.tile.x < 0) {
            result.tile.x += world->tile_count_x;
            result.tilemap.x -= 1;
        }
    }

    if (result.relative_position_on_tile.y < 0) {
        result.relative_position_on_tile.y += world->tile_side_in_meters;
        result.tile.y -= 1;

        if (result.tile.y < 0) {
            result.tile.y += world->tile_count_y;
            result.tilemap.y -= 1;
        }
    }

    if (result.relative_position_on_tile.x >= world->tile_side_in_meters) {
        result.relative_position_on_tile.x -= world->tile_side_in_meters;
        result.tile.x += 1;

        if (result.tile.x >= world->tile_count_x) {
            result.tile.x -= world->tile_count_x;
            result.tilemap.x += 1;
        }
    }

    if (result.relative_position_on_tile.y >= world->tile_side_in_meters) {
        result.relative_position_on_tile.y -= world->tile_side_in_meters;
        result.tile.y += 1;

        if (result.tile.y >= world->tile_count_y) {
            result.tile.y -= world->tile_count_y;
            result.tilemap.y += 1;
        }
    }

    ASSERT((0 <= result.tile.x) && (result.tile.x < world->tile_count_x));
    ASSERT((0 <= result.tile.y) && (result.tile.y < world->tile_count_y));

    ASSERT((0 <= result.relative_position_on_tile.x) && (result.relative_position_on_tile.x < world->tile_side_in_meters));
    ASSERT((0 <= result.relative_position_on_tile.y) && (result.relative_position_on_tile.y < world->tile_side_in_meters));

    return result;
}

INTERNAL_FUNCTION
bool32 IsWorldPointEmpty(Worldmap *world, WorldPosition norm_position) {
    bool32 result = false;

    Tilemap* tilemap = GetTilemap(world, norm_position.tilemap);
    if (tilemap) {
        int32 tile_value = GetTileValue_Unchecked(world, tilemap, norm_position.tile);

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

        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, },
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 0, 0, 1, },
    };

    uint32 tiles_01[TILEMAP_TILES_Y][TILEMAP_TILES_X] = {
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 1,  1, 0, 0, 1,  0, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 1, 0,  1, 0, 0, 1,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 0,  1, 0, 0, 1,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 1, 1,  1, 0, 0, 1,  0, 0, 0, 1, },

        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 0, 0, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
    };

    uint32 tiles_10[TILEMAP_TILES_Y][TILEMAP_TILES_X] = {
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 1,  1, 0, 0, 0, },

        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 0,  1, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 0,  1, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 1,  1, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
    };

    uint32 tiles_11[TILEMAP_TILES_Y][TILEMAP_TILES_X] = {
        { 1, 0, 0, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 0, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  0, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 0, 1,  0, 0, 0, 1, },

        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1, },
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
    world.tile_side_in_meters = 1; // [meters]
    world.tile_side_in_meters = 1; // [meters];

    world.tile_count_x = TILEMAP_TILES_X;
    world.tile_count_y = TILEMAP_TILES_Y;
    world.tilemap_count_x = 2;
    world.tilemap_count_y = 2;
    world.tilemaps = (Tilemap*) tilemaps;

    Game_State* GameState = (Game_State*)Memory->PermanentStorage;

    if (!Memory->IsInitialized) {
        GameState->character_position.tilemap = { 0, 0 };
        GameState->character_position.tile = { 3, 3 };
        GameState->character_position.relative_position_on_tile = { 0.5f, 0.5f }; // in meters relative to the tile

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

    Tilemap* current_tilemap = GetTilemap(&world, GameState->character_position.tilemap);
    ASSERT(current_tilemap);

    float32 pixels_per_meter = 60.f;

    math::vector2 character_dimensions = { 0.75f, 1.0f }; // [meters x meters]
    float32 character_speed = 2.5f; // [meters/second]

    {
        Game_ControllerInput* Input0 = GetController(Input, 0);
        Input0 = &Input->KeyboardController;

        // [meters]
        float32 new_position_x =
            GameState->character_position.relative_position_on_tile.x + Input0->StickLXEnded * character_speed * Input->dt;
        float32 new_position_y =
            GameState->character_position.relative_position_on_tile.y + Input0->StickLYEnded * character_speed * Input->dt;

        WorldPosition temp_player_position = GameState->character_position;
        temp_player_position.relative_position_on_tile.x = new_position_x;
        temp_player_position.relative_position_on_tile.y = new_position_y;

        WorldPosition player_left_corner = temp_player_position;
        player_left_corner.relative_position_on_tile.x -= character_dimensions.x * 0.5f;

        WorldPosition player_right_corner = temp_player_position;
        player_right_corner.relative_position_on_tile.x += character_dimensions.x * 0.5f;

        WorldPosition normalized_player_position = NormalizeWorldPosition(&world, temp_player_position);
        WorldPosition normalized_left_corner     = NormalizeWorldPosition(&world, player_left_corner);
        WorldPosition normalized_right_corner    = NormalizeWorldPosition(&world, player_right_corner);

        bool32 tile_is_valid = true;
        tile_is_valid &= IsWorldPointEmpty(&world, normalized_left_corner);
        tile_is_valid &= IsWorldPointEmpty(&world, normalized_right_corner);

        if (tile_is_valid) {
            GameState->character_position = normalized_player_position;
        }
    }

    Game_OutputSound(SoundBuffer, GameState);

    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    int32 character_tile_x = GameState->character_position.tile.x;
    int32 character_tile_y = GameState->character_position.tile.y;
    for (int Row = 0; Row < world.tile_count_y; Row++) {
        for (int Column = 0; Column < world.tile_count_x; Column++) {
            int32 TileId = GetTileValue_Unchecked(&world, current_tilemap, { Column, Row });

            float32 TileBottomLeftX = (float32) Column * world.tile_side_in_meters * pixels_per_meter;
            float32 TileBottomLeftY = (float32) Buffer->Height - (float32) Row * world.tile_side_in_meters * pixels_per_meter;

            math::vector2 TileUpperLeft {
                TileBottomLeftX,
                TileBottomLeftY - world.tile_side_in_meters * pixels_per_meter,
                // (float32) Row    * world.tile_side_in_meters * pixels_per_meter,
                // (float32) Column * world.tile_side_in_meters * pixels_per_meter,
            };

            math::vector2 TileBottomRight {
                TileUpperLeft.x + world.tile_side_in_meters  * pixels_per_meter,
                TileUpperLeft.y + world.tile_side_in_meters * pixels_per_meter,
            };

            auto TileColor = math::color24{ 0.5f, 0.5f, 0.5f };
            if (TileId == 1) {
                TileColor = math::color24{ 0.2f, 0.3f, 0.2f };
            }

            if (Row == character_tile_y && Column == character_tile_x) {
                TileColor = math::color24{ 0.8f, 0.4f, 0.0f };
            }

            RenderRectangle(
                Buffer,
                TileUpperLeft,
                TileBottomRight,
                TileColor);
        }
    }

    math::vector2 player_bottom_left_corner_in_pixels {
        GameState->character_position.tile.x * (float32)world.tile_side_in_meters +
            GameState->character_position.relative_position_on_tile.x - 0.5f*character_dimensions.x,
        GameState->character_position.tile.y * (float32)world.tile_side_in_meters +
            GameState->character_position.relative_position_on_tile.y - character_dimensions.y
    };
    player_bottom_left_corner_in_pixels *= pixels_per_meter;
    player_bottom_left_corner_in_pixels.y = Buffer->Height - player_bottom_left_corner_in_pixels.y;

    math::vector2 player_top_right_corner_in_pixels {
        GameState->character_position.tile.x * (float32)world.tile_side_in_meters + GameState->character_position.relative_position_on_tile.x + 0.5f*character_dimensions.x,
        GameState->character_position.tile.y * (float32)world.tile_side_in_meters + GameState->character_position.relative_position_on_tile.y
    };
    player_top_right_corner_in_pixels *= pixels_per_meter;
    player_top_right_corner_in_pixels.y = Buffer->Height - player_top_right_corner_in_pixels.y;

    RenderRectangle(
        Buffer,
        { player_bottom_left_corner_in_pixels.x, player_top_right_corner_in_pixels.y - world.tile_side_in_meters * pixels_per_meter },
        player_top_right_corner_in_pixels,
        math::color24{ 0.9f, 0.9f, 0.2f });

#ifdef ASUKA_PLAYBACK_LOOP
    if (BorderVisible) {
        RenderBorder(Buffer, BorderWidth, BorderColor);
    }
#endif // ASUKA_PLAYBACK_LOOP
}
