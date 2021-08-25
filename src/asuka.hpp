#ifndef ASUKA_ASUKA_HPP
#define ASUKA_ASUKA_HPP

#include "defines.hpp"


//
// Services that the platform layer provides to the game.
//

//
// Services that the game provides to the platform layer.
//

struct Game_ButtonState {
    int32  HalfTransitionCount;
    bool32 EndedDown;
};

struct Game_AxisState {

};

struct Game_ControllerInput {
    bool32 IsAnalog;

    union {
        Game_ButtonState buttons[14];
        struct {
            Game_ButtonState A;
            Game_ButtonState B;
            Game_ButtonState X;
            Game_ButtonState Y;
            Game_ButtonState DpadUp;
            Game_ButtonState DpadDown;
            Game_ButtonState DpadLeft;
            Game_ButtonState DpadRight;
            Game_ButtonState ShoulderLeft;
            Game_ButtonState ShoulderRight;
            Game_ButtonState StickLeft;
            Game_ButtonState StickRight;
            Game_ButtonState Back;
            Game_ButtonState Start;
        };
    };

    float32 StickLXStarted;
    float32 StickLYStarted;

    float32 StickLXEnded;
    float32 StickLYEnded;

    float32 StickRXStarted;
    float32 StickRYStarted;

    float32 StickRXEnded;
    float32 StickRYEnded;

    float32 TriggerLeftStarted;
    float32 TriggerLeftEnded;

    float32 TriggerRightStarted;
    float32 TriggerRightEnded;
};

struct Game_Input {
    Game_ControllerInput Controllers[4];
};


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

struct Game_Memory {
    uint64 PermanentStorageSize;
    void*  PermanentStorage;

    uint64 TransientStorageSize;
    void*  TransientStorage;

    bool32 IsInitialized;
};

struct Game_State {
    int XOffset;
    int YOffset;
    int ToneHz;
};

// IN:
// 1. control input
// 2. bitmap buffer to fill
// 3. sound buffer to fill
// 4. timing
void Game_UpdateAndRender(
    Game_Memory* Memory,
    Game_Input* Input,
    Game_OffscreenBuffer* Buffer,
    Game_SoundOutputBuffer* SoundBuffer);


#ifdef UNITY_BUILD
#include "asuka.cpp"
#endif

#endif // ASUKA_ASUKA_HPP
