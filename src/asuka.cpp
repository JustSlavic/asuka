#include "asuka.hpp"
#include <math.hpp>
#include <math.h>
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


static void RenderCharacter(Game_OffscreenBuffer* Buffer, float32 x, float32 y) {
    float32 character_width = 30;
    float32 character_height = 30;
    RenderRectangle(
        Buffer,
        math::vector2{x - character_width*0.5f, y - character_height},
        math::vector2{x + character_width*0.5f, y},
        math::color24{0.9f, 0.9f, 0.2f});
}


#ifdef ASUKA_DEBUG
static void RenderBorder(Game_OffscreenBuffer* Buffer, uint32 Width, math::color24 Color) {
    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Width}, {(float32)Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {(float32)Buffer->Width - Width, (float32)Width}, {(float32)Buffer->Width, (float32)Buffer->Height - Width}, Color);
    RenderRectangle(Buffer, {0, (float32)Buffer->Height - Width}, {(float32)Buffer->Width, (float32)Buffer->Height}, Color);
}
#endif


GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    ASSERT(sizeof(Game_State) <= Memory->PermanentStorageSize);

    Game_State* GameState = (Game_State*)Memory->PermanentStorage;

    if (!Memory->IsInitialized) {
        GameState->character_position = { 100, 100 };

#ifdef ASUKA_DEBUG
        GameState->BorderWidth = 10;
        GameState->BorderColor = math::color24{1.f, 1.f, 0.f};
        GameState->BorderVisible = false;
#endif // ASUKA_DEBUG

        Memory->IsInitialized = true;
    }

    float32 character_speed = 100;

    {
        Game_ControllerInput* Input0 = GetController(Input, 0);
        GameState->character_position.x += truncate_cast_to_int32(Input0->StickLXEnded * character_speed * Input->dt);
        GameState->character_position.y -= truncate_cast_to_int32(Input0->StickLYEnded * character_speed * Input->dt);
    }
    {
        Game_ControllerInput* KeyboardController = &Input->KeyboardController;
        if (KeyboardController->DpadUp.EndedDown) {
            GameState->character_position.y -= character_speed*Input->dt;
        }
        if (KeyboardController->DpadDown.EndedDown) {
            GameState->character_position.y += character_speed*Input->dt;
        }
        if (KeyboardController->DpadLeft.EndedDown) {
            GameState->character_position.x -= character_speed*Input->dt;
        }
        if (KeyboardController->DpadRight.EndedDown) {
            GameState->character_position.x += character_speed*Input->dt;
        }
    }

    uint32 tile_width = 60;
    uint32 tile_height = 60;
    uint32 tilemap[9][16] = {
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0, },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  0, 1, 0, 0, },
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0, },

        { 1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 1, },
        { 1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 1, 0, },
        { 1, 1, 0, 0,  0, 0, 0, 1,  0, 0, 0, 0,  0, 1, 0, 0, },

        { 1, 0, 1, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0, },
        { 1, 0, 0, 1,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, },
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, },
    };

    Game_OutputSound(SoundBuffer, GameState);

    RenderRectangle(Buffer, {0, 0}, {(float32)Buffer->Width, (float32)Buffer->Height}, {1.f, 0.f, 1.f});

    for (int Column = 0; Column < 9; Column++) {
        for (int Row = 0; Row < 16; Row++) {
            uint32 TileId = tilemap[Column][Row];

            float TileX = ((float32)Row)*tile_width;
            float TileY = ((float32)Column)*tile_height;

            auto TileColor = math::color24{ 0.5f, 0.5f, 0.5f };
            if (TileId == 1) {
                TileColor = math::color24{ 0.2f, 0.3f, 0.2f };
            }
            RenderRectangle(Buffer, {TileX, TileY}, {TileX + tile_width, TileY + tile_height}, TileColor);
        }
    }

    RenderCharacter(Buffer, GameState->character_position.x, GameState->character_position.y);

#ifdef ASUKA_DEBUG
    if (GameState->BorderVisible) {
        RenderBorder(Buffer, GameState->BorderWidth, GameState->BorderColor);
    }
#endif // ASUKA_DEBUG
}
