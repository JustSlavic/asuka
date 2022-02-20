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


using InputIndex = Index<Game_ControllerInput>;

INLINE
Game_ControllerInput* GetControllerInput(Game_Input* Input, InputIndex ControllerIndex) {
    ASSERT(ControllerIndex < ARRAY_COUNT(Input->ControllerInputs));
    return &Input->ControllerInputs[ControllerIndex.index];
}


INLINE
Game_ControllerInput* GetGamepadInput(Game_Input *Input, int32 GamepadIndex) {
    ASSERT(GamepadIndex < ARRAY_COUNT(Input->GamepadInputs));
    return &Input->GamepadInputs[GamepadIndex];
}

INLINE
uint32 GetPressCount(Game_ButtonState button) {
    uint32 result = (button.HalfTransitionCount + (button.EndedDown > 0)) / 2;
    return result;
}

INLINE
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


enum FaceDirection {
    FACE_DIRECTION_DOWN = 0,
    FACE_DIRECTION_LEFT = 1,
    FACE_DIRECTION_RIGHT = 2,
    FACE_DIRECTION_UP = 3,
};


enum EntityType {
    ENTITY_TYPE_NULL,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_WALL,
    ENTITY_TYPE_FAMILIAR,
    ENTITY_TYPE_MONSTER,
};


struct HighEntity;
using HighEntityIndex = Index<HighEntity>;

struct HighEntity {
    math::v3 position; // Relative to the camera
    math::v3 velocity;
    int32 chunk_z; // for moving up and down "stairs"

    f32 tBob = 0.0f;

    FaceDirection face_direction;

    LowEntityIndex low_index;
};


struct HealthPoint {
    u32 fill_level; // In percent (max 100).
    b32 shielded;
    b32 poisoned;
};


struct LowEntity {
    EntityType type;
    WorldPosition world_position;
    // @note: for "stairs"
    int32 d_abs_tile_z;

    math::v2 hitbox;
    bool32 collidable;

    int32 health_max;
    HealthPoint health[32];

    HighEntityIndex high_index;
};

struct Entity {
    HighEntityIndex high_index;
    HighEntity *high;

    LowEntityIndex low_index;
    LowEntity  *low;
};

using math::v2;
using math::color32;

struct VisiblePiece {
    // @note: offset and dimenstions in pixel, top-down screen space
    v2 offset;
    v2 dimensions;

    Bitmap *bitmap;
    color32 color;
};


struct VisiblePieceGroup {
    u32 count;
    VisiblePiece assets[8];

    f32 pixels_per_meter;
};


void push_rectangle(VisiblePieceGroup *group, math::v3 offset_in_meters, math::v2 dim_in_meters, math::color32 color) {
    // @note offset and dimensions are in world space (in meters, bottom-up coordinate space)
    ASSERT(group->count < ARRAY_COUNT(group->assets));

    VisiblePiece *asset = group->assets + (group->count++);
    memory::set(asset, 0, sizeof(VisiblePiece));

    asset->offset = v2{ offset_in_meters.x, -(offset_in_meters.y + offset_in_meters.z) } * group->pixels_per_meter;
    asset->dimensions = dim_in_meters * group->pixels_per_meter;
    asset->color = color;
}

void push_asset(VisiblePieceGroup *group, Bitmap *bitmap, math::v3 offset_in_meters, f32 alpha = 1.0f) {
    // @note offset and dimensions are in world space (in meters, bottom-up coordinate space)
    ASSERT(group->count < ARRAY_COUNT(group->assets));

    VisiblePiece *asset = group->assets + (group->count++);
    memory::set(asset, 0, sizeof(VisiblePiece));

    asset->bitmap = bitmap;
    asset->offset = v2{ offset_in_meters.x, -(offset_in_meters.y + offset_in_meters.z) } * group->pixels_per_meter;
    asset->color.a = alpha;
}


struct GameState {
    WorldPosition camera_position;

    // @note: 0-th entity is invalid in both arrays (high entities and low entities) and should not be used (it indicates wrong index).
    uint32 low_entity_count;
    LowEntity low_entities[10000];

    uint32 high_entity_count;
    HighEntity high_entities[256];

    LowEntityIndex player_index_for_controller[ARRAY_COUNT(((Game_Input*)0)->ControllerInputs)];
    LowEntityIndex index_of_entity_for_camera_to_follow;
    LowEntityIndex index_of_controller_for_camera_to_follow;

    World *world;

    memory::arena_allocator world_arena;

    wav_file_contents test_wav_file;

    Bitmap wall_texture;
    Bitmap tree_texture;
    Bitmap grass_texture;
    Bitmap heart_full_texture;
    Bitmap heart_empty_texture;
    Bitmap familiar_texture;
    Bitmap shadow_texture;

    Bitmap monster_head;
    Bitmap monster_left_arm;
    Bitmap monster_right_arm;

    Bitmap player_textures[4];

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
