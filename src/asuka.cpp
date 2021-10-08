#include "asuka.hpp"
#include <math.hpp>
#include <math.h>
#include <debug/casts.hpp>


typedef int16 sound_sample_t;


void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, Game_State* GameState) {
    int16 ToneVolume = 2000;
    int16 WavePeriod = GameState->ToneHz != 0 ? truncate_cast_to_int16(SoundBuffer->SamplesPerSecond / GameState->ToneHz) : 1;
    sound_sample_t* SampleOut = SoundBuffer->Samples;

    for (int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {

        float32 SineValue = sinf(GameState->SineTime);
        sound_sample_t SampleValue = truncate_cast_to_int16(SineValue * ToneVolume);

        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        GameState->SineTime += 2.f * math::consts<float32>::pi() / WavePeriod;
        if (GameState->SineTime > 2.f * math::consts<float32>::pi()) {
            GameState->SineTime -= 2.f * math::consts<float32>::pi();
        }
    }
}


#ifdef ASUKA_DEBUG
static void RenderCharacter(Game_OffscreenBuffer* Buffer, int X, int Y) {
    int Width = 10;
    int Height = 10;

    if (Y < 0 || (Buffer->Height - Height) < Y || X < 0 || (Buffer->Width - Width) < X) {
        return;
    }

    uint8* Row = (uint8*)Buffer->Memory + Y*Buffer->Pitch;
    for (int y = 0; y < Height; y++) {
        uint32* Pixel = (uint32*) Row + X;
        for (int x = 0; x < Width; x++) {
            *Pixel = 0xFF'FF'FF'FF;
            Pixel++;
        }

        Row += Buffer->Pitch;
    }
}


static void RenderBorder(Game_OffscreenBuffer* Buffer, uint32 Width, uint32 Color) {
    uint8* Row = (uint8*) Buffer->Memory;

    for (uint32 y = 0; y < Width; y++) {
        uint32* Pixel = (uint32*) Row;
        for (int x = 0; x < Buffer->Width; x++) {
            *Pixel = Color;
            Pixel++;
        }

        Row += Buffer->Pitch;
    }

    Row = (uint8*) Buffer->Memory + (Buffer->Height - Width)*Buffer->Pitch;
    for (uint32 y = 0; y < Width; y++) {
        uint32* Pixel = (uint32*) Row;
        for (int x = 0; x < Buffer->Width; x++) {
            *Pixel = Color;
            Pixel++;
        }

        Row += Buffer->Pitch;
    }

    Row = (uint8*) Buffer->Memory + (Width * Buffer->Pitch);
    for (uint32 y = 0; y < (Buffer->Height - 2*Width); y++) {
        uint32* LeftPixel = (uint32*) Row;
        uint32* RightPixel = (uint32*) Row + (Buffer->Width - Width);
        for (uint32 x = 0; x < Width; x++) {
            *LeftPixel++ = Color;
            *RightPixel++ = Color;
        }

        Row += Buffer->Pitch;
    }
}
#endif


static void RenderGradient(Game_OffscreenBuffer* Buffer, int XOffset, int YOffset) {
    //                  Width->                                      Width * BytesPerPixel
    // BitmapMemory: 0  BB GG RR xx BB GG RR xx BB GG RR xx    ...   BB GG RR xx           <additional padding?>
    // BM + Pitch:   1  BB GG RR xx BB GG RR xx BB GG RR xx    ...   BB GG RR xx           <additional padding?>
    //
    // Pitch = Width * BytesPerPixel + Stride
    //
    uint8* Row = (uint8*)Buffer->Memory;

    for (int y = 0; y < Buffer->Height; y++) {
        uint32* Pixel = (uint32*) Row;
        for (int x = 0; x < Buffer->Width; x++) {
            //
            // Pixel in memory: 00 00 00 00
            //                  BB GG RR xx
            // BUT ARCHITECTURE IS LITTLE ENDIAN!!!
            // Pixel in registry: ARGB
            //
            uint8 Blue = truncate_cast_to_uint8(x + XOffset);
            uint8 Green = truncate_cast_to_uint8(y + YOffset);

            *Pixel = (Blue) | (Green << 8);
            Pixel++;
        }

        Row += Buffer->Pitch;
    }
}


GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    ASSERT(sizeof(Game_State) <= Memory->PermanentStorageSize);

    Game_State* GameState = (Game_State*)Memory->PermanentStorage;

    if (!Memory->IsInitialized) {
        GameState->SineTime = 0.f;
        GameState->ToneHz = 256;
        GameState->XOffset = 0;
        GameState->YOffset = 0;

#ifdef ASUKA_DEBUG
        GameState->CharacterPositionX = 100;
        GameState->CharacterPositionY = 100;

        GameState->BorderWidth = 10;
        GameState->BorderColor = 0xFF'FF'FF'00;
        GameState->BorderVisible = false;
#endif // ASUKA_DEBUG

        Memory->IsInitialized = true;
    }

    for (int ControllerIndex = 0; ControllerIndex < ARRAY_COUNT(Input->Controllers); ControllerIndex++) {
        Game_ControllerInput* Input0 = GetController(Input, ControllerIndex);
        if (Input0->IsAnalog) {
            GameState->ToneHz = truncate_cast_to_int32(256 + (256 * Input0->StickRYEnded));
            GameState->XOffset += truncate_cast_to_int32(Input0->StickRXEnded * 10);
            GameState->YOffset += truncate_cast_to_int32(Input0->StickRYEnded * 10);

#ifdef ASUKA_DEBUG
            GameState->CharacterPositionX += truncate_cast_to_int32(Input0->StickLXEnded * 2);
            GameState->CharacterPositionY -= truncate_cast_to_int32(Input0->StickLYEnded * 2);
#endif // ASUKA_DEBUG

            if (Input0->A.EndedDown) {
                GameState->YOffset += 100;
            }
        } else {
            if (Input0->DpadUp.EndedDown) {
                GameState->YOffset += 10;
            }
        }
    }

    // @todo: Allow sample offsets for more robust platform options
    Game_OutputSound(SoundBuffer, GameState);
    RenderGradient(Buffer, GameState->XOffset, GameState->YOffset);

#ifdef ASUKA_DEBUG
    RenderCharacter(Buffer, GameState->CharacterPositionX, GameState->CharacterPositionY);
    RenderCharacter(Buffer, Input->Mouse.X, Input->Mouse.Y);
    if (GameState->BorderVisible) {
        RenderBorder(Buffer, GameState->BorderWidth, GameState->BorderColor);
    }
#endif // ASUKA_DEBUG
}
