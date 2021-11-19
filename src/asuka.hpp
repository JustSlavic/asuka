#ifndef ASUKA_ASUKA_HPP
#define ASUKA_ASUKA_HPP

#include <defines.hpp>
#include <math.hpp>
#include <wav.hpp>
#include <bmp.hpp>
#include <png.hpp>
#include "tilemap.hpp"
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
    /*
    HalfTransitionCount helps detect very quick pressing, but we are bad at slow pressing

                  +-+
                  | |
    PulsePress ---+ +---------------------

                  +------------+
                  |            |
    HoldPress  ---+            +----------
                             ^
                             | second "press" on second frame which I want not to happen
                             v
    Frames     |------------|------------|

    to get number of "presses" happened on this frame, call 'uint32 GetPressCount(Game_ButtonState)' function
    to get number of "holds" happened on this frame, call 'uint32 GetHoldsCount(Game_ButtonState)' function
    */

    bool32 EndedDown;  // Was it ended down?
    int32  HalfTransitionCount; // How many times state was changed
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

    math::v2 LeftStickStarted;
    math::v2 LeftStickEnded;

    math::v2 RightStickStarted;
    math::v2 RightStickEnded;

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


#if ASUKA_PLAYBACK_LOOP
enum Debug_PlaybackLoopState {
    PLAYBACK_LOOP_IDLE,
    PLAYBACK_LOOP_RECORDING,
    PLAYBACK_LOOP_PLAYBACK,
};
#endif // ASUKA_PLAYBACK_LOOP


struct Game_Input {
    Game_MouseState Mouse;

    union {
        // 0 - Keyboard controller
        // 1-5 - Gamepad controllers
        Game_ControllerInput ControllerInputs[5];
        struct {
            Game_ControllerInput KeyboardInput;
            Game_ControllerInput GamepadInputs[4];
        };
    };

    // Probably should go to a game no in controller input?
    float32 dt;

#if ASUKA_PLAYBACK_LOOP
    Debug_PlaybackLoopState PlaybackLoopState;
#endif // ASUKA_PLAYBACK_LOOP
};


INLINE_FUNCTION
Game_ControllerInput* GetControllerInput(Game_Input* Input, int ControllerIndex) {
    ASSERT(ControllerIndex < ARRAY_COUNT(Input->ControllerInputs));
    return &Input->ControllerInputs[ControllerIndex];
}


INLINE_FUNCTION
Game_ControllerInput* GetGamepadInput(Game_Input *Input, int32 GamepadIndex) {
    ASSERT(GamepadIndex < ARRAY_COUNT(Input->GamepadInputs));
    return &Input->GamepadInputs[GamepadIndex];
}

INLINE_FUNCTION
uint32 GetPressCount(Game_ButtonState button) {
    uint32 result = (button.HalfTransitionCount + (button.EndedDown > 0)) / 2;
    return result;
}

INLINE_FUNCTION
uint32 GetHoldsCount(Game_ButtonState button) {
    uint32 result = (button.HalfTransitionCount > 0) || (button.EndedDown > 0);
    return result;
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




struct game_world {
    tile_map tilemap;
};

enum PlayerFaceDirection {
    PLAYER_FACE_DOWN = 0,
    PLAYER_FACE_LEFT = 1,
    PLAYER_FACE_RIGHT = 2,
    PLAYER_FACE_UP = 3,
};

struct game_entity {
    tile_map_position position;
    math::v2 velocity;
    PlayerFaceDirection face_direction;
    math::v2 hitbox;
};

struct game_state {
    tile_map_position camera_position;
    game_entity player;

    game_world *world;

    memory_arena world_arena;

    wav_file_contents test_wav_file;

    bitmap wall_texture;
    bitmap floor_texture;
    bitmap grass_texture;

    bitmap player_textures[4];

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
