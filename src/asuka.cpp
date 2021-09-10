#include "asuka.hpp"
#include <math.hpp>
#include <math.h>
#include <debug/casts.hpp>


void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, int ToneHz) {
    static float32 tSine;

    int16 ToneVolume = 2000;
    int16 WavePeriod = ToneHz != 0 ? truncate_cast_to_int16(SoundBuffer->SamplesPerSecond / ToneHz) : 1;
    int16* SampleOut = SoundBuffer->Samples;

    for (int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {

        float32 SineValue = sinf(tSine);
        int16 SampleValue = truncate_cast_to_int16(SineValue * ToneVolume);

        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        tSine += 2.f * math::consts<float32>::pi() / WavePeriod;
        if (tSine > 2.f * math::consts<float32>::pi()) {
            tSine -= 2.f * math::consts<float32>::pi();
        }
    }
}


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


void Game_UpdateAndRender(
    Game_Memory* Memory,
    Game_Input* Input,
    Game_OffscreenBuffer* Buffer,
    Game_SoundOutputBuffer* SoundBuffer)
{
    ASSERT(sizeof(Game_State) <= Memory->PermanentStorageSize);

    Game_State* GameState = (Game_State*)Memory->PermanentStorage;

    if (!Memory->IsInitialized) {
        GameState->ToneHz = 256;
        GameState->XOffset = 0;
        GameState->YOffset = 0;
        Memory->IsInitialized = true;
    }

    for (int ControllerIndex = 0; ControllerIndex < ARRAY_COUNT(Input->Controllers); ControllerIndex++) {
        Game_ControllerInput* Input0 = GetController(Input, ControllerIndex);
        if (Input0->IsAnalog) {
            GameState->ToneHz = truncate_cast_to_int32(256 + (256 * Input0->StickLYEnded));
            GameState->XOffset += truncate_cast_to_int32(Input0->StickRXEnded * 10);
            GameState->YOffset += truncate_cast_to_int32(Input0->StickRYEnded * 10);

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
    Game_OutputSound(SoundBuffer, GameState->ToneHz);
    RenderGradient(Buffer, GameState->XOffset, GameState->YOffset);
}
