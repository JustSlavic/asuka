#pragma once

#include <stdio.h>
#include <defines.hpp>
#include <math.hpp>
#include <wav.hpp>
#include <bmp.hpp>
#include <png.hpp>
#include <string.hpp>
#include <index.hpp>
#include <world.hpp>
#include <memory_arena.hpp>
#include "sim_region.hpp"
#include "entity.hpp"

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

    To get number of "presses" happened on this frame, call 'u32 GetPressCount(Game_ButtonState)' function.
    To get number of "holds" happened on this frame, call 'u32 GetHoldsCount(Game_ButtonState)' function.

    In the example, on frame one there should be 1 press and 1 hold. On frame 2 there should be 0 presses and 1 hold.
    */

    b32 EndedDown;  // Was it ended down?
    i32  HalfTransitionCount; // How many times state was changed
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

    v2 LeftStickStarted;
    v2 LeftStickEnded;

    v2 RightStickStarted;
    v2 RightStickEnded;

    f32 TriggerLeftStarted;
    f32 TriggerLeftEnded;

    f32 TriggerRightStarted;
    f32 TriggerRightEnded;
};


struct Game_MouseState {
    // Coordinates in client area coordinate space
    u32 X;
    u32 Y;
    i32 Wheel;

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
    f32 dt;

#if ASUKA_PLAYBACK_LOOP
    Debug_PlaybackLoopState PlaybackLoopState;
#endif // ASUKA_PLAYBACK_LOOP
};


using InputIndex = Index<Game_ControllerInput>;

inline
Game_ControllerInput* GetControllerInput(Game_Input* Input, InputIndex ControllerIndex)
{
    ASSERT(ControllerIndex < ARRAY_COUNT(Input->ControllerInputs));
    return &Input->ControllerInputs[ControllerIndex.index];
}


inline
Game_ControllerInput* GetGamepadInput(Game_Input *Input, i32 GamepadIndex)
{
    ASSERT(GamepadIndex < ARRAY_COUNT(Input->GamepadInputs));
    return &Input->GamepadInputs[GamepadIndex];
}

inline
u32 GetPressCount(Game_ButtonState button)
{
    u32 result = (button.HalfTransitionCount + (button.EndedDown > 0)) / 2;
    return result;
}

inline
u32 GetReleaseCount(Game_ButtonState button)
{
    u32 result = (button.HalfTransitionCount - (button.EndedDown > 0) + 1) / 2;
    return result;
}

inline
u32 GetHoldsCount(Game_ButtonState button)
{
    u32 result = (button.HalfTransitionCount + (button.EndedDown > 0) + 1) / 2;
    return result;
}



struct StoredEntity {
    WorldPosition world_position;

    // @todo: Compress this.
    SimEntity sim;
};


struct Game_OffscreenBuffer {
    // Pixels are always 32-bits wide Little Endian, Memory Order BBGGRRxx
    void *Memory;
    i32 Width;
    i32 Height;
    i32 Pitch;
    i32 BytesPerPixel;
};


struct Game_SoundOutputBuffer {
    sound_sample_t *Samples;
    i32 SampleCount;
    i32 SamplesPerSecond;
};


struct Game_Memory {
    u64 PermanentStorageSize;
    void *PermanentStorage;

    u64 TransientStorageSize;
    void *TransientStorage;

    b32 IsInitialized;
};


struct VisiblePiece {
    // @note: offset and dimenstions in pixel, top-down screen space
    v2 offset;
    v2 dimensions;

    Bitmap *bitmap;
    color32 color;
};


struct VisiblePieceGroup {
    u32 count;
    VisiblePiece assets[16];

    f32 pixels_per_meter;
};


struct GameState {
    WorldPosition camera_position;

    // @note: 0-th entity is invalid in both arrays (high entities and low entities) and should not be used (it indicates wrong index).
    u32 entity_count;
    StoredEntity entities[10000];

    u32 player_index_for_controller[ARRAY_COUNT(((Game_Input*)0)->ControllerInputs)];
    u32 index_of_entity_for_camera_to_follow;
    u32 index_of_controller_for_camera_to_follow;

    World *world;

    memory::arena_allocator world_arena;
    memory::arena_allocator sim_arena;

    wav_file_contents test_wav_file;

    Bitmap wall_texture;
    Bitmap tree_texture;
    Bitmap grass_texture;
    Bitmap heart_full_texture;
    Bitmap heart_empty_texture;
    Bitmap familiar_texture;
    Bitmap shadow_texture;
    Bitmap fireball_texture;
    Bitmap sword_texture;

    Bitmap monster_head;
    Bitmap monster_left_arm;
    Bitmap monster_right_arm;

    Bitmap player_textures[4];

    u32 test_current_sound_cursor;
};


inline
StoredEntity *get_stored_entity(GameState *game_state, u32 index) {
    ASSERT(index < ARRAY_COUNT(game_state->entities));

    StoredEntity *result = NULL;
    if ((index > 0) && (index < game_state->entity_count)) {
        result = game_state->entities + index;
    }

    return result;
}


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


#include "world.cpp"
#include "sim_region.cpp"


#if defined(UNITY_BUILD) && !defined(ASUKA_DLL_BUILD)
#include "asuka.cpp"
#endif
