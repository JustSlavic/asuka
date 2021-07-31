#include "asuka.hpp"
#include <math.hpp>
#include <math.h>


void Game_OutputSound(Game_SoundOutputBuffer *SoundBuffer, int ToneHz) {
    static float32 tSine;

    int16 ToneVolume = 2000;

    int16 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16* SampleOut = SoundBuffer->Samples;
    for (int32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {

        float32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);

        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        tSine += 2.f * math::consts<float32>::pi() / WavePeriod;
    }
}


static void RenderGradient(Game_OffscreenBuffer* Buffer, int XOffset, int YOffset) {
    //                  Width->                                      Width * BytesPerPixel
    // BitmapMemory: 0  BB GG RR xx BB GG RR xx BB GG RR xx    ...   BB GG RR xx           <additional padding?>
    // BM + Pitch:   1  BB GG RR xx BB GG RR xx BB GG RR xx    ...   BB GG RR xx           <additional padding?>
    //
    // Pitch = Width * BytesPerPixel + Stride
    //

    int Pitch = Buffer->Width*Buffer->BytesPerPixel;
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
            uint8 Blue = x + XOffset;
            uint8 Green = y + YOffset;

            *Pixel = (Blue) | (Green << 8);
            Pixel++;
        }

        Row += Buffer->Pitch;
    }
}


void Game_UpdateAndRender(
    Game_OffscreenBuffer* Buffer, int XOffset, int YOffset,
    Game_SoundOutputBuffer* SoundBuffer, int ToneHz)
{
    // @todo: Allow sample offsets for more robust platform options
    Game_OutputSound(SoundBuffer, ToneHz);
    RenderGradient(Buffer, XOffset, YOffset);
}
