#ifndef ASUKA_ASUKA_HPP
#define ASUKA_ASUKA_HPP

#include <defines.hpp>
#include <math.hpp>
#include <wav.hpp>
#include "memory_arena.hpp"


struct ThreadContext {
};


//
// Services that the platform layer provides to the game.
//

// Include ThreadContext into Debug_PlatformReadEntireFile(),
// Debug_PlatfromWriteEntireFile() and Debug_PlatformFreeFileMemory()

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
    union {
        Game_ButtonState Buttons[14];
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


struct Game_MouseState {
    // Coordinates in client area coordinate space
    uint32 X;
    uint32 Y;
    int32 Wheel;

    union {
        Game_ButtonState Buttons[5];
        struct {
            Game_ButtonState LMB;
            Game_ButtonState MMB;
            Game_ButtonState RMB;
            Game_ButtonState MB_1;
            Game_ButtonState MB_2;
        };
    };
};


#ifdef ASUKA_PLAYBACK_LOOP
enum Debug_PlaybackLoopState {
    PLAYBACK_LOOP_IDLE,
    PLAYBACK_LOOP_RECORDING,
    PLAYBACK_LOOP_PLAYBACK,
};
#endif // ASUKA_PLAYBACK_LOOP


struct Game_Input {
    Game_MouseState Mouse;
    Game_ControllerInput KeyboardController;
    // 0 - Keyboard controller
    // 1-5 - Gamepad controllers
    Game_ControllerInput Controllers[4];

    // Probably should go to a game no in controller input?
    float32 dt;

#ifdef ASUKA_PLAYBACK_LOOP
    Debug_PlaybackLoopState PlaybackLoopState;
#endif // ASUKA_PLAYBACK_LOOP
};


inline Game_ControllerInput* GetController(Game_Input* Input, int ControllerIndex) {
    ASSERT(ControllerIndex < ARRAY_COUNT(Input->Controllers));
    return &Input->Controllers[ControllerIndex];
}


struct Game_OffscreenBuffer {
    // Pixels are always 32-bits wide Little Endian, Memory Order BBGGRRxx
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};


struct Game_SoundOutputBuffer {
    sound_sample_t* Samples;
    int32 SampleCount;
    int32 SamplesPerSecond;
};

struct Game_Memory {
    uint64 PermanentStorageSize;
    void*  PermanentStorage;

    uint64 TransientStorageSize;
    void*  TransientStorage;

    bool32 IsInitialized;
};


struct tile_chunk {
    int32 *tiles;
};


struct tile_chunk_position {
    // Allow chunks grow from the center of a map in any direction
    int32 tilechunk_x;
    int32 tilechunk_y;

    // Inside a chunk there could not be tiles on negative coordinates
    uint32 chunk_relative_x;
    uint32 chunk_relative_y;
};


struct World {
    float32 tile_side_in_meters;

    uint32 tile_count_x;
    uint32 tile_count_y;

    uint32 tilechunk_count_x;
    uint32 tilechunk_count_y;

    tile_chunk* tilechunks;

    uint32 chunk_shift;
    uint32 chunk_mask;
};


struct WorldPosition {
    // Coordinates of the tilemap
    // vector2i tilemap;

    // Coordinates of the tile
    // vector2i tile;

    int32 absolute_tile_x;
    int32 absolute_tile_y;

    // uint32 tile_x;
    // uint32 tile_y;

    // In pixels inside a tile
    vector2 relative_position_on_tile;
};


struct Game_State {
    WorldPosition player_position;
    vector2 player_velocity;

    memory_arena world_arena;

    wav_file_contents test_wav_file;
    uint32 test_current_sound_cursor;
};


// IN:
// 1. control input
// 2. bitmap buffer to fill
// 3. sound buffer to fill
// 4. timing
#define GAME_UPDATE_AND_RENDER(name) void name(ThreadContext* Thread, Game_Memory* Memory, Game_Input* Input, Game_OffscreenBuffer* Buffer, Game_SoundOutputBuffer* SoundBuffer)
typedef GAME_UPDATE_AND_RENDER(Game_UpdateAndRenderT);


extern "C" {
ASUKA_DLL_EXPORT GAME_UPDATE_AND_RENDER(Game_UpdateAndRender);
}


#if defined(UNITY_BUILD) && !defined(ASUKA_DLL_BUILD)
#include "asuka.cpp"
#endif

#endif // ASUKA_ASUKA_HPP
