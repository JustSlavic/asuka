#ifndef ASUKA_ASUKA_HPP
#define ASUKA_ASUKA_HPP

#include <defines.hpp>
#include <math.hpp>
#include <wav.hpp>
#include <bmp.hpp>
#include <png.hpp>
#include <string.hpp>
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
    HalfTransitionCount helps detect very quick pressing.

                  frame 1       frame 2
                  +-+
                  | |
    PulsePress |--+ +-------|------------|

                  +------------+
                  |            |
    HoldPress  |--+         |  +---------|
                               ^
                               second "press" on second frame which I want not to happen

    To get number of "presses" happened on this frame, call 'uint32 GetPressCount(Game_ButtonState)' function.
    To get number of "holds" happened on this frame, call 'uint32 GetHoldsCount(Game_ButtonState)' function.

    In the example, on frame one there should be 1 press and 1 hold. On frame 2 there should be 0 presses and 1 hold.
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
    uint32 result = (button.HalfTransitionCount + (button.EndedDown > 0) + 1) / 2;
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


struct Game_World {
    Tilemap tilemap;
};


enum FaceDirection {
    FACE_DIRECTION_DOWN = 0,
    FACE_DIRECTION_LEFT = 1,
    FACE_DIRECTION_RIGHT = 2,
    FACE_DIRECTION_UP = 3,
};

enum EntityResidence {
    ENTITY_RESIDENCE_NOT_EXIST = 0,
    ENTITY_RESIDENCE_LOW,
    ENTITY_RESIDENCE_HIGH,
};

enum EntityType {
    ENTITY_TYPE_NULL,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_WALL,
};

struct HighFrequencyEntity {
    math::v3 position; // Relative to the camera
    math::v3 velocity;
    int32 absolute_tile_z; // for moving up and down "stairs"

    FaceDirection face_direction;
};

struct LowFrequencyEntity {
    EntityType type;
    TilemapPosition tilemap_position;
    // @note: for "stairs"
    int32 d_abs_tile_z;

    math::v2 hitbox;
    bool32 collidable;
};

struct Entity {
    EntityResidence residence;
    HighFrequencyEntity *high;
    LowFrequencyEntity  *low;
};

struct GameState {
    TilemapPosition camera_position;

    // Note: 0-th entity is invalid and should not be used
    uint32 entity_count;
    EntityResidence     residence_table[256];
    HighFrequencyEntity high_frequency_entity_table[256];
    LowFrequencyEntity  low_frequency_entity_table[256];

    uint32 player_index_for_controller[ARRAY_COUNT(((Game_Input*)0)->ControllerInputs)];
    uint32 index_of_entity_for_camera_to_follow;
    uint32 index_of_controller_for_camera_to_follow;

    Game_World *world;

    MemoryArena world_arena;

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
