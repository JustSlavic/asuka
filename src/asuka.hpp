#pragma once

#include <defines.hpp>
#include <math.hpp>
#include <world.hpp>
#include <sim_region.hpp>
#include <bitmap.hpp>
#include <wav.hpp>
#include <array.hpp>
#include <ui/ui.hpp>

#if UI_EDITOR_ENABLED
#include <ui/ui_editor.hpp>
#endif

#include <stdio.h>


/*

    Architecture:

    - Metagame
      - Saves
      - Steam Store
      - Achievements
    - Multiple SimRegions per sim
    - Collision detection
      - Entry / Exit
      - Shapes
    - AI
      - Behaviour trees
      - Pathfinding
    - 3D positioning
      - Walk up and down the stairs
    - World Generation
    - Animation
      - Skeletal animation
      - Particle systems
    - Asset streaming
    - Audio
      - Sound effect triggers
      - Ambient sounds
      - Music
    - Debug code
      - Logging
      - Diagramming
      - GUI
        - Editor ImGui-like things
        - Console
    - GUI
      - Cursor graphics
      - Shapes
      - Buttons
      - Textfields
      - Layout containers

*/

enum PlatformCommand
{
    PLATFORM_COMMAND_NONE = 0,
    PLATFORM_COMMAND_EXIT = 1,
};

struct PlatformCommandQueue
{
    PlatformCommand commands[32];
    u32 command_count;
    u64 next_command_index; // This index always gets increased, so use % to return to bounds
};

void push_command(PlatformCommandQueue *queue, PlatformCommand command)
{
    ASSERT_MSG(queue->command_count < ARRAY_COUNT(queue->commands), "Command buffer has ended!");
    u64 index = (queue->next_command_index + queue->command_count) % ARRAY_COUNT(queue->commands);
    queue->commands[index] = command;
    queue->command_count += 1;
}

PlatformCommand pop_command(PlatformCommandQueue *queue)
{
    PlatformCommand result = PLATFORM_COMMAND_NONE;
    if (queue->command_count > 0)
    {
        result = queue->commands[queue->next_command_index % ARRAY_COUNT(queue->commands)];
        queue->command_count -= 1;
        queue->next_command_index += 1;
    }

    return result;
}

struct ThreadContext
{
    u32 thread_id;
    PlatformCommandQueue *command_queue;
};

//
// Services that the platform layer provides to the game.
//

// Include Thread into Debug_PlatformReadEntireFile(),
// Debug_PlatfromWriteEntireFile() and Debug_PlatformFreeFileMemory()

//
// Services that the game provides to the platform layer.
//

namespace Game
{

struct ButtonState
{
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
    i32 HalfTransitionCount; // How many times state was changed
};

struct AxisState
{
};

struct ControllerInput
{
    union
    {
        ButtonState Buttons[14];
        struct
        {
            ButtonState A;
            ButtonState B;
            ButtonState X;
            ButtonState Y;
            ButtonState DpadUp;
            ButtonState DpadDown;
            ButtonState DpadLeft;
            ButtonState DpadRight;
            ButtonState ShoulderLeft;
            ButtonState ShoulderRight;
            ButtonState StickLeft;
            ButtonState StickRight;
            ButtonState Back;
            ButtonState Start;
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


// We need this for extended keyboard input, e.g. pressing F1-F12 keys to enable debug drawings, etc.
struct KeyboardState
{
    union
    {
        // @todo: Consider making key position in the array equal to VK_Keycode from Win32 API?
        // for easy access.
        ButtonState buttons[21];
        struct
        {
            ButtonState Esc;

            ButtonState Ctrl;
            ButtonState Shift;

            ButtonState F1;
            ButtonState F2;
            ButtonState F3;
            ButtonState F4;
            ButtonState F5;
            ButtonState F6;
            ButtonState F7;
            ButtonState F8;
            ButtonState F9;
            ButtonState F10;
            ButtonState F11;
            ButtonState F12;

            ButtonState Z;

            ButtonState Delete;

            ButtonState ArrowLeft;
            ButtonState ArrowRight;
            ButtonState ArrowUp;
            ButtonState ArrowDown;
        };
    };
};


struct MouseState {
    // Coordinates in client area coordinate space
    v2i position;
    v2i previous_position;
    i32 wheel;

    union {
        ButtonState buttons[5];
        struct {
            ButtonState LMB;
            ButtonState MMB;
            ButtonState RMB;
            ButtonState MB_1;
            ButtonState MB_2;
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


struct Input
{
    KeyboardState keyboard;
    MouseState mouse;

    union
    {
        // 0 - Keyboard controller
        // 1-5 - Gamepad controllers
        ControllerInput ControllerInputs[5];
        struct
        {
            ControllerInput KeyboardInput;
            ControllerInput GamepadInputs[4];
        };
    };

    // Probably should go to a game no in controller input?
    float32 dt;

#if ASUKA_PLAYBACK_LOOP
    Debug_PlaybackLoopState PlaybackLoopState;
#endif // ASUKA_PLAYBACK_LOOP
};


INLINE
ControllerInput *GetControllerInput(Input *Input, u32 ControllerIndex)
{
    ASSERT(ControllerIndex < ARRAY_COUNT(Input->ControllerInputs));
    return &Input->ControllerInputs[ControllerIndex];
}

INLINE
ControllerInput *GetGamepadInput(Input *Input, u32 GamepadIndex)
{
    ASSERT(GamepadIndex < ARRAY_COUNT(Input->GamepadInputs));
    return &Input->GamepadInputs[GamepadIndex];
}

INLINE
uint32 GetPressCount(ButtonState button)
{
    uint32 result = (button.HalfTransitionCount + (button.EndedDown > 0)) / 2;
    return result;
}

INLINE
uint32 GetReleaseCount(ButtonState button)
{
    uint32 result = (button.HalfTransitionCount - (button.EndedDown > 0) + 1) / 2;
    return result;
}

INLINE
uint32 GetHoldCount(ButtonState button)
{
    uint32 result = (button.HalfTransitionCount + (button.EndedDown > 0) + 1) / 2;
    return result;
}



struct StoredEntity {
    WorldPosition world_position;

    // @todo: Compress this.
    SimEntity sim;
};


struct OffscreenBuffer {
    // Pixels are always 32-bits wide Little Endian, Memory Order BBGGRRxx
    void *Memory;
    int32 Width;
    int32 Height;
    int32 Pitch;
    int32 BytesPerPixel;
};


struct SoundOutputBuffer {
    sound_sample_t *Samples;
    int32 SampleCount;
    int32 SamplesPerSecond;
};


struct Memory {
    usize PermanentStorageSize;
    void *PermanentStorage;

    usize TransientStorageSize;
    void *TransientStorage;

    usize CustomHeapStorageSize;
    void *CustomHeapStorage;

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


struct PlayerRequest {
    u32 entity_index;
    b32 player_jump;
    f32 player_acceleration_strength;
    v3  player_acceleration_direction;
    v3  sword_velocity; // @note: should be unit length
};


struct MoveSpec {
    v3 acceleration;
    b32 jump;
};


INLINE
MoveSpec move_spec()
{
    MoveSpec spec = {};
    return spec;
}


struct GameState {
    WorldPosition camera_position;

    // @note: 0-th entity is invalid in both arrays (high entities and low entities) and should not be used (it indicates wrong index).
    uint32 entity_count;
    StoredEntity entities[10000];

    PlayerRequest player_for_controller[ARRAY_COUNT(((Input*)0)->ControllerInputs)];
    uint32 index_of_entity_for_camera_to_follow;

    World *world;

    memory::arena_allocator world_arena;
    memory::arena_allocator temp_arena;
    memory::arena_allocator ui_arena;

    wav_file_contents test_wav_file;

    byte_array training_set;

    Bitmap wall_texture;
    Bitmap tree_texture;
    Bitmap grass_texture;
    Bitmap heart_full_texture;
    Bitmap heart_empty_texture;
    Bitmap familiar_texture;
    Bitmap shadow_texture;
    Bitmap fireball_texture;
    Bitmap sword_texture;
    Bitmap cursor_texture;

    Bitmap monster_head;
    Bitmap monster_left_arm;
    Bitmap monster_right_arm;

    Bitmap player_textures[4];

    u32 test_current_sound_cursor;

    UiScene *game_hud;

    f32 exit_confirmation_time;

#if UI_EDITOR_ENABLED
    UiEditor *ui_editor;
    b32 ui_editor_enabled;
#endif
};


INLINE
StoredEntity *get_stored_entity(GameState *game_state, u32 index) {
    ASSERT(index < ARRAY_COUNT(game_state->entities));

    StoredEntity *result = NULL;
    if ((index > 0) && (index < game_state->entity_count)) {
        result = game_state->entities + index;
    }

    return result;
}

} // namespace Game


// IN:
// 1. control input
// 2. bitmap buffer to fill
// 3. sound buffer to fill
// 4. timing
#define GAME_INITIALIZE_MEMORY(NAME) void NAME(Game::Memory *memory);
typedef GAME_INITIALIZE_MEMORY(Game_InitializeMemoryT);

#define GAME_UPDATE_AND_RENDER(NAME) void NAME(ThreadContext *thread, Game::Memory *Memory, Game::Input *Input, Game::OffscreenBuffer *Buffer)
typedef GAME_UPDATE_AND_RENDER(Game_UpdateAndRenderT);

#define GAME_OUTPUT_SOUND(NAME) void NAME(ThreadContext *thread, Game::Memory *Memory, Game::SoundOutputBuffer* SoundBuffer)
typedef GAME_OUTPUT_SOUND(Game_OutputSoundT);

extern "C" {
ASUKA_DLL_EXPORT GAME_INITIALIZE_MEMORY(Game_InitializeMemory);
ASUKA_DLL_EXPORT GAME_UPDATE_AND_RENDER(Game_UpdateAndRender);
ASUKA_DLL_EXPORT GAME_OUTPUT_SOUND(Game_OutputSound);
}

#if (ASUKA_DLL && ASUKA_DLL_BUILD) || (!ASUKA_DLL_BUILD)
#include <world.cpp>
#include <sim_region.cpp>
#include <ui/ui.cpp>

#if UI_EDITOR_ENABLED
#include <ui/ui_editor.cpp>
#endif // UI_EDITOR_ENABLED

#endif

#if !ASUKA_DLL && !ASUKA_DLL_BUILD
#include <asuka.cpp>
#else
#endif
