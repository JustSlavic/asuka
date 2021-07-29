#ifndef ASUKA_ASUKA_HPP
#define ASUKA_ASUKA_HPP

#include "defines.hpp"


//
// Services that the platform layer provides to the game.
//

//
// Services that the game provides to the platform layer.
//


struct Game_OffscreenBuffer {
    // Pixels are always 32-bits wide Little Endian, Memory Order BBGGRRxx
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct Game_SoundOutputBuffer {
    int16*  Samples;
    int32   SampleCount;
    int32   SamplesPerSecond;
};

// IN:
// 1. control input
// 2. bitmap buffer to fill
// 3. sound buffer to fill
// 4. timing
void Game_UpdateAndRender(
    Game_OffscreenBuffer* Buffer, int XOffset, int YOffset,
    Game_SoundOutputBuffer* SoundBuffer, int ToneHz);


#ifdef UNITY_BUILD
#include "asuka.cpp"
#endif

#endif // ASUKA_ASUKA_HPP
