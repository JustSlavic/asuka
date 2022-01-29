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
Entity *entity(GameState *game_state, EntityIndex index) {
    ASSERT(index < ARRAY_COUNT(game_state->entities));

    Entity *result = NULL;

    if (index > 0) {
        result = game_state->entities + index;
    }

    return result;
}


INTERNAL_FUNCTION
Entity *get_entity(GameState *game_state, EntityIndex index) {
    ASSERT(index < ARRAY_COUNT(game_state->entities));

    Entity *entity = game_state->entities + index.index;
    return entity;
}


INTERNAL_FUNCTION
Entity *add_entity(GameState *game_state) {
    ASSERT(game_state->entity_count < ARRAY_COUNT(game_state->entities));

    EntityIndex index { game_state->entity_count++ };
    Entity *entity = get_entity(game_state, index);
    return entity;
}


INTERNAL_FUNCTION
void set_camera_position(GameState *game_state, TilemapPosition new_camera_position) {
    game_state->camera_position = new_camera_position;
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
        add_entity(game_state);

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

        // Tilechunks 256x256
        tilemap->chunk_shift = 3;
        tilemap->chunk_mask  = (1 << tilemap->chunk_shift) - 1;
        tilemap->tile_count_x = 1 << tilemap->chunk_shift;
        tilemap->tile_count_y = 1 << tilemap->chunk_shift;

        i32 map_side_in_chunks = 10;

        for (int32 chunk_y = 0; chunk_y < map_side_in_chunks; chunk_y++) {
            for (int32 chunk_x = 0; chunk_x < map_side_in_chunks; chunk_x++) {
                for (uint32 tile_y = 0; tile_y < tilemap->tile_count_y; tile_y++) {
                    for (uint32 tile_x = 0; tile_x < tilemap->tile_count_x; tile_x++) {
                        i32 abs_x = chunk_x * tilemap->tile_count_x + tile_x;
                        i32 abs_y = chunk_y * tilemap->tile_count_y + tile_y;

                        SetTileValue(arena, tilemap, abs_x, abs_y, TILE_FREE);
                    }
                }
            }
        }

        TilemapPosition camera_position;
        camera_position.absolute_tile_x = map_side_in_chunks * tilemap->tile_count_x / 2;
        camera_position.absolute_tile_y = map_side_in_chunks * tilemap->tile_count_y / 2;
        set_camera_position(game_state, camera_position);

        Memory->IsInitialized = true;
    }

    Tilemap *tilemap = game_state->tilemap;
    MemoryArena *arena = &game_state->world_arena;

    float32 pixels_per_meter = 60.f; // [pixels/m]

    for (uint32 ControllerIndex = 0; ControllerIndex < ARRAY_COUNT(Input->ControllerInputs); ControllerIndex++) {
        Game_ControllerInput* ControllerInput = GetControllerInput(Input, ControllerIndex);


        if (GetHoldsCount(ControllerInput->A)) {
            SetTileValue(arena, tilemap,
                game_state->camera_position.absolute_tile_x,
                game_state->camera_position.absolute_tile_y,
                TILE_GRASS);
        }


        TilemapPosition new_camera_position = game_state->camera_position;

        f32 threshold = 0.8f;
        if (GetPressCount(ControllerInput->DpadLeft) || ControllerInput->LeftStickEnded.x < -threshold) {
            new_camera_position.absolute_tile_x -= 1;
        }

        if (GetPressCount(ControllerInput->DpadRight) || ControllerInput->LeftStickEnded.x > threshold) {
            new_camera_position.absolute_tile_x += 1;
        }

        if (GetPressCount(ControllerInput->DpadUp) || ControllerInput->LeftStickEnded.y > threshold) {
            new_camera_position.absolute_tile_y += 1;
        }

        if (GetPressCount(ControllerInput->DpadDown) || ControllerInput->LeftStickEnded.y < -threshold) {
            new_camera_position.absolute_tile_y -= 1;
        }

        set_camera_position(game_state, new_camera_position);
    }

    // ===================== RENDERING ===================== //

    // Render pink background to see pixels I didn't drew.
    DrawRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    // Background grass
    // DrawBitmap(Buffer, { 0, 0 }, { (float32)Buffer->Width, (f32)Buffer->Height }, &game_state->grass_texture);

    TilemapPosition *camera_p = &game_state->camera_position;

#if 0
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
                }
                case TILE_FREE: {
                    TileColor = color24{ 0.5f, 0.5f, 0.5f };

                    if (relative_row == 0 && relative_column == 0) {
                        TileColor = color24{ 0.8f, 0.5f, 0.2f };
                        break;
                    } else
                    if ((row % tilemap->tile_count_y == 0 || column % tilemap->tile_count_x == 0))
                    {
                        TileColor = color24{ 0.0f, 0.4f, 0.8f };
                        break;
                    }

                    break;
                }
                case TILE_GRASS: {
                    TileColor = color24{ 0.3f, 0.8f, 0.3f };
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

    for (EntityIndex entity_index { 0 }; entity_index < game_state->entity_count; entity_index++) {
        Entity *entity = get_entity(game_state, entity_index);
        if (entity != NULL) {
            v2 entity_position_in_pixels =
                0.5f * v2{ (f32)Buffer->Width, (f32)Buffer->Height } +
                v2{ entity->position.x, -entity->position.y } * pixels_per_meter;

            auto TileColor = color24{ 0.2f, 0.3f, 0.2f };

            v2 top_left = entity_position_in_pixels - 0.5f * entity->hitbox * pixels_per_meter;
            v2 bottom_right = top_left + entity->hitbox * pixels_per_meter;
            DrawRectangle(
                Buffer,
                top_left,
                bottom_right,
                TileColor, true);
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
