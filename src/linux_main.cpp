#include <defines.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <cerrno>

#include <asuka.hpp>
#include <os/memory.hpp>
#include <os/time.hpp>
#include <time.h>

#if SOUND_ALSA
#include <alsa/asoundlib.h>
#endif

#define KEYCODE_ESC                  9
#define KEYCODE_W                    25
#define KEYCODE_A                    38
#define KEYCODE_S                    39
#define KEYCODE_D                    40
#define KEYCODE_Z                    52
#define KEYCODE_SPACE                65
#define KEYCODE_F1                   67
#define KEYCODE_F2                   68
#define KEYCODE_F3                   69
#define KEYCODE_F4                   70
#define KEYCODE_F5                   71
#define KEYCODE_F6                   72
#define KEYCODE_F7                   73
#define KEYCODE_F8                   74
#define KEYCODE_F9                   75
#define KEYCODE_F10                  76
#define KEYCODE_F11                  95
#define KEYCODE_F12                  96
#define KEYCODE_UP                   111
#define KEYCODE_LEFT                 113
#define KEYCODE_RIGHT                114
#define KEYCODE_DOWN                 116
#define KEYCODE_CTRL                 37
#define KEYCODE_SHIFT                50

#define MOUSE_LMB                    1
#define MOUSE_MMB                    2
#define MOUSE_RMB                    3
#define MOUSE_WHEEL_UP               4
#define MOUSE_WHEEL_DOWN             5
#define MOUSE_MB_1                   8
#define MOUSE_MB_2                   9

#define GAMEPAD_EVENT_BUTTON         0x01    /* button pressed/released */
#define GAMEPAD_EVENT_AXIS           0x02    /* joystick moved */
#define GAMEPAD_EVENT_INIT           0x80    /* initial state of device */

#define GAMEPAD_AXIS_MIN             (-32767)
#define GAMEPAD_AXIS_MAX             (32767)

#define GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define GAMEPAD_TRIGGER_DEADZONE     7710


GLOBAL bool global_running;
GLOBAL v2i  global_resolution_presets[12] =
{
    { 800, 600 },
    { 960, 540 }, // Test Resolution
    { 1024, 768 },
    { 1152, 864 },
    { 1280, 1024 },
    { 1440, 900 },
    { 1600, 900 },
    { 1600, 1200 },
    { 1680, 1050 },
    { 1920, 1080 },
    { 1920, 1200 },
    { 2560, 1440 },
};
GLOBAL const char* global_gamepad_device_paths[] = {
    "/dev/input/js0",
    "/dev/input/js1",
    "/dev/input/js2",
    "/dev/input/js3",
};


struct linux_screen_buffer {
    XImage x_image;
    void* memory;
    uint32 width;
    uint32 height;
    uint32 bytes_per_pixel;
};

#if SOUND_ALSA
struct linux_sound_output {
    uint32 samples_per_second;
    uint32 channels_count;
    uint64 buffer_size;
    sound_sample_t* samples;
    snd_pcm_t* sound_device;
};
#endif

struct input_event {
    timeval timestamp;
    unsigned short type;
    unsigned short code;
    unsigned int value;
};

struct linux_mouse
{
    int32 fd;
};

INTERNAL
bool32 linux_mouse_connect(linux_mouse *mouse, const char *device_path)
{
    int32 fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) return false;

    mouse->fd = fd;
    return true;
}

struct linux_mouse_event
{
    input_event event;
};

INTERNAL
bool32 linux_mouse_next_event(linux_mouse* mouse, linux_mouse_event* event)
{
    int64 bytes = read(mouse->fd, event, sizeof(linux_mouse_event));
    // note: bytes == -1 when read is not successful
    return bytes > 0;
}

struct linux_gamepad
{
    int32 fd;
};


struct linux_gamepad_event
{
    uint32 time;   /* event timestamp in milliseconds */
    int16  value;  /* value */
    uint8  type;   /* event type */
    uint8  number; /* axis/button number */
};


INTERNAL
void print_binary(uint32 n) {
    uint32 mask = (0x1 << 31);

    while (mask) {
        printf("%d", (n & mask) > 0);
        mask >>= 1;
    }
}


INTERNAL
bool32 linux_gamepad_connect(linux_gamepad* gamepad, const char* device_path) {
    int fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) return false;

    gamepad->fd = fd;
    return true;
}


INTERNAL
void linux_gamepad_disconnect(linux_gamepad* gamepad) {
    gamepad->fd = 0;
}


INTERNAL
bool32 linux_gamepad_connected(linux_gamepad* gamepad) {
    return gamepad->fd > 0;
}


INTERNAL
bool32 linux_gamepad_lost_connection(linux_gamepad* gamepad) {
    struct stat statbuf;
    int ec = fstat(gamepad->fd, &statbuf);

    bool32 result = (ec == 0) && (statbuf.st_nlink > 0);
    return !result;
}


INTERNAL
bool32 linux_gamepad_next_event(linux_gamepad* gamepad, linux_gamepad_event* event) {
    int64 bytes = read(gamepad->fd, event, sizeof(linux_gamepad_event));
    // note: bytes == -1 when read is not successful
    return bytes > 0;
}


INTERNAL
void linux_gamepad_process_button(Game::ButtonState* Button, int16 Value) {
    Button->HalfTransitionCount += 1;
    Button->EndedDown = (Value == 1);
}


INTERNAL
float32 linux_gamepad_process_stick(int16 value, int16 deadzone) {
    if (value < -deadzone) {
        return ((float32)value + (float32)deadzone) / (32767.f - (float32)deadzone);
    } else if (value > deadzone) {
        return ((float32)value - (float32)deadzone) / (32767.f - (float32)deadzone);
    }

    return 0.f;
}


INTERNAL
float32 linux_process_controller_trigger(int16 value, int16 deadzone) {
    // In linux the value of the triggers vary between -32767 and 32767,
    // so I shift it to be completely positive value.
    uint16 shifted_value = value + 32767;
    if (shifted_value > deadzone) {
        return ((float32)shifted_value - (float32)deadzone) / (65534.f - (float32)deadzone);
    }

    return 0.f;
}


INTERNAL
void linux_process_key_event(Game::ButtonState *new_state, bool32 is_down_now)
{
    if (new_state->EndedDown != is_down_now)
    {
        new_state->EndedDown = is_down_now;
        new_state->HalfTransitionCount++;
    }
}


INTERNAL
void linux_gamepad_process_events(linux_gamepad* device, Game::ControllerInput* Controller) {
    // @bug: This function have to function properly which it doesn't at all!!!
    // @todo: figure out what is about this function is broken! I do not have a contoller fix it now.
    linux_gamepad_event event;
    while (linux_gamepad_next_event(device, &event)) {
        if (event.type & GAMEPAD_EVENT_INIT) {
            // These are synthetic events for initialization
            if (event.type & GAMEPAD_EVENT_BUTTON) {
                // printf("INIT BUTTON %d\n", event.number);
            } else if (event.type & GAMEPAD_EVENT_AXIS) {
                // printf("INIT AXIS %d\n", event.number);
            }
        } else if (event.type & GAMEPAD_EVENT_BUTTON) {
            switch (event.number) {
                case 0: linux_gamepad_process_button(&Controller->A, event.value); break;
                case 1: linux_gamepad_process_button(&Controller->B, event.value); break;
                case 2: linux_gamepad_process_button(&Controller->X, event.value); break;
                case 3: linux_gamepad_process_button(&Controller->Y, event.value); break;
                case 4: linux_gamepad_process_button(&Controller->ShoulderLeft, event.value); break;
                case 5: linux_gamepad_process_button(&Controller->ShoulderRight, event.value); break;
                case 6: linux_gamepad_process_button(&Controller->Back, event.value); break;
                case 7: linux_gamepad_process_button(&Controller->Start, event.value); break;
                // case 8: linux_gamepad_process_button(&Controller->XBox, event.value); printf("XBOX %s", event.value ? "PRESS" : "RELEASE"); break;
                case 9: linux_gamepad_process_button(&Controller->StickLeft, event.value); break;
                case 10: linux_gamepad_process_button(&Controller->StickRight, event.value);break;
            }
        } else if (event.type & GAMEPAD_EVENT_AXIS) {
            switch (event.number) {
                case 0: Controller->LeftStickEnded.x = linux_gamepad_process_stick(event.value, GAMEPAD_LEFT_THUMB_DEADZONE);    break;
                case 1: Controller->LeftStickEnded.y = linux_gamepad_process_stick(-event.value, GAMEPAD_LEFT_THUMB_DEADZONE);   break;
                case 2: Controller->TriggerLeftEnded = linux_process_controller_trigger(event.value, GAMEPAD_TRIGGER_DEADZONE);  break;
                case 3: Controller->RightStickEnded.x = linux_gamepad_process_stick(event.value, GAMEPAD_RIGHT_THUMB_DEADZONE);  break;
                case 4: Controller->RightStickEnded.y = linux_gamepad_process_stick(-event.value, GAMEPAD_RIGHT_THUMB_DEADZONE); break;
                case 5: Controller->TriggerRightEnded = linux_process_controller_trigger(event.value, GAMEPAD_TRIGGER_DEADZONE); break;
                case 6: printf("Dpad X %d\n", event.value); break;
                case 7: printf("Dpad Y %d\n", event.value); break;
            }
        } else {
            ASSERT_FAIL("Unrecognized gamepad event.");
        }
    }
}


INTERNAL
void linux_resize_screen_buffer(linux_screen_buffer* buffer, uint32 width, uint32 height) {
    if (buffer->memory) {
        memory::free_pages(buffer->memory);
        buffer->memory = NULL;
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = 4;

    buffer->memory = memory::allocate_pages(width * height * buffer->bytes_per_pixel);
    ASSERT(buffer->memory);
}


INTERNAL
void linux_copy_buffer_to_window(linux_screen_buffer* buffer, Display* display, Window window, int screen) {
    // @TODO: Should I cache this structure int the linux_screen_buffer to avoid XInitImage call ??
    XImage x_image {};
    x_image.width = buffer->width;
    x_image.height = buffer->height;
    x_image.xoffset = 0;
    x_image.format = ZPixmap;
    x_image.data = (char*)buffer->memory;
    x_image.byte_order = LSBFirst;
    x_image.bitmap_unit = 32;
    x_image.bitmap_bit_order = LSBFirst;
    x_image.bitmap_pad = 32;
    x_image.depth = 24;
    x_image.bits_per_pixel = 32;
    x_image.red_mask = 0xFF0000;
    x_image.green_mask = 0xFF00;
    x_image.blue_mask = 0xFF;

    int good = XInitImage(&x_image);
    ASSERT(good);

    XPutImage(display, window, DefaultGC(display, screen), &x_image,
        0, 0, 0, 0, buffer->width, buffer->height);
}

#if SOUND_ALSA
linux_sound_output linux_init_alsa(uint32 samples_per_second, uint32 channels) {
    // @todo: try to use snd_pcm_set_params
    snd_pcm_t* sound_device;
    snd_pcm_hw_params_t* hw_params;

    int dir = 0;
    int err;

#define ASUKA_CALL_ALSA(FUNCTION, ...) \
    err = FUNCTION(__VA_ARGS__); \
    if (err < 0) { \
        fprintf(stderr, STRINGIFY(FUNCTION) ": %s\n", snd_strerror(err)); \
        return linux_sound_output{}; \
    } void(0)

    ASUKA_CALL_ALSA(snd_pcm_open, &sound_device, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);

    snd_pcm_hw_params_alloca(&hw_params);
    ASUKA_CALL_ALSA(snd_pcm_hw_params_any, sound_device, hw_params);
    ASUKA_CALL_ALSA(snd_pcm_hw_params_set_rate_resample, sound_device, hw_params, 0);
    ASUKA_CALL_ALSA(snd_pcm_hw_params_set_access, sound_device, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    ASUKA_CALL_ALSA(snd_pcm_hw_params_set_format, sound_device, hw_params, SND_PCM_FORMAT_S16_LE);
    ASUKA_CALL_ALSA(snd_pcm_hw_params_set_channels, sound_device, hw_params, channels);
    ASUKA_CALL_ALSA(snd_pcm_hw_params_set_buffer_size, sound_device, hw_params, samples_per_second);
    ASUKA_CALL_ALSA(snd_pcm_hw_params_set_rate, sound_device, hw_params, samples_per_second, dir);
    ASUKA_CALL_ALSA(snd_pcm_hw_params, sound_device, hw_params);

    {
        printf("PCM name: %s\n", snd_pcm_name(sound_device));
        printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(sound_device)));

        uint32 time;
        ASUKA_CALL_ALSA(snd_pcm_hw_params_get_buffer_time, hw_params, &time, &dir);
        printf("Buffer time: %u (dir: %d)\n", time, dir);

        snd_pcm_uframes_t buffer_size;
        ASUKA_CALL_ALSA(snd_pcm_hw_params_get_buffer_size, hw_params, &buffer_size);
        printf("Buffer size: %ld\n", buffer_size);

        uint32 rate;
        ASUKA_CALL_ALSA(snd_pcm_hw_params_get_rate, hw_params, &rate, &dir);
        printf("Sampling rate: %u (dir: %d)\n", rate, dir);

        int32 descriptors_count = snd_pcm_poll_descriptors_count(sound_device);
        printf("Descriptors count: %d\n", descriptors_count);
    }

    ASUKA_CALL_ALSA(snd_pcm_prepare, sound_device);

#undef ASUKA_CALL_ALSA

    linux_sound_output sound_output;
    sound_output.samples_per_second = samples_per_second;
    sound_output.channels_count = channels;
    sound_output.buffer_size = sound_output.samples_per_second * sound_output.channels_count * sizeof(int16);
    sound_output.samples = (sound_sample_t*) memory::allocate_pages(sound_output.buffer_size);
    sound_output.sound_device = sound_device;

    return sound_output;
}


static void linux_send_sound_buffer(snd_pcm_t* sound_device, linux_sound_output* sound_output, uint32 frames_to_write) {
    int err = snd_pcm_writei(sound_device, sound_output->samples, frames_to_write);
    if (err < 0) {
        fprintf(stderr, "snd_pcm_writei: %s\n", snd_strerror(err));
        return;
    }
}
#endif // SOUND_ALSA

int32 main(int32 argc, char** argv)
{
    Display* display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        printf("Cannot open display\n");
        return 1;
    }

    int screen = XDefaultScreen(display);

    uint32 process_timer_granularity_ms = 1000 / sysconf(_SC_CLK_TCK);
    printf("process_timer_granularity_ms = %u\n", process_timer_granularity_ms);
    uint32 monitor_refresh_hz = 60;
    uint32 game_update_hz = monitor_refresh_hz / 2;
    float32 target_seconds_per_frame = 1.0f / (float32) game_update_hz;


    const i32 resolution_index = 1;
    STATIC_ASSERT_MSG(resolution_index < ARRAY_COUNT(global_resolution_presets), "Resolution Index is too high");
    v2i resolution = global_resolution_presets[resolution_index];

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        0, // WIN_X,
        0, // WIN_Y,
        resolution.x,
        resolution.y,
        1, // WIN_BORDER,
        BlackPixel(display, screen),
        WhitePixel(display, screen));

    linux_screen_buffer screen_buffer {};
    linux_resize_screen_buffer(&screen_buffer, resolution.x, resolution.y);

    // linux_mouse mouse;

    // bool32 success = linux_mouse_connect(&mouse, "/dev/input/mouse0");
    // printf("mouse connection %s\n", success ? "successfull" : "failure");

    // return 0;

#if SOUND_ALSA
    linux_sound_output sound_output = linux_init_alsa(48000, 2);
    if (sound_output.sound_device) {
        printf("ALSA initialized successfully!\n");
    } else {
        fprintf(stderr, "Could not initialize ALSA!\n");
        return 1;
    }
#endif

    // Process window close event through event handler so XNextEvent does not fail
    Atom del_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &del_window, 1);

    // Select kind of events we are interested in
    XSelectInput(display, window,
        ExposureMask |
        KeyPressMask |
        KeyReleaseMask |
        PointerMotionMask |
        ButtonPressMask |
        ButtonReleaseMask);

    // Display the window
    XMapWindow(display, window);

#ifdef ASUKA_DEBUG
    void* base_address = (void*)TERABYTES(1);
#else
    void* base_address = 0;
#endif

    ThreadContext context =  {};

    Game::Memory game_memory = {};
    game_memory.PermanentStorageSize = MEGABYTES(64);
    game_memory.TransientStorageSize = GIGABYTES(2);

    uint64 total_size = game_memory.PermanentStorageSize + game_memory.TransientStorageSize;

    game_memory.PermanentStorage = memory::allocate_pages(base_address, total_size);
    game_memory.TransientStorage = (uint8*)game_memory.PermanentStorage + game_memory.PermanentStorageSize;

    game_memory.CustomHeapStorageSize = MEGABYTES(10);
    game_memory.CustomHeapStorage = memory::allocate_pages((void *)TERABYTES(2), game_memory.CustomHeapStorageSize);

    Game::Input Input = {};
    Input.dt = target_seconds_per_frame;

    global_running = true;
    os::timepoint last_counter = os::get_wall_clock();
    uint64 last_cycles = os::get_processor_cycles();

#if SOUND_ALSA
    snd_pcm_sframes_t play_cursor = 0;
    snd_pcm_sframes_t write_cursor = (sound_output.buffer_size / sizeof(sound_sample_t)) - snd_pcm_avail(sound_output.sound_device);
#endif

    linux_gamepad gamepad_devices[ARRAY_COUNT(global_gamepad_device_paths)] {};

    while (global_running)
    {
        auto *mouse = &Input.mouse;
        mouse->previous_position = mouse->position;
        for (i32 button_index = 0; button_index < ARRAY_COUNT(mouse->buttons); button_index++)
        {
            mouse->buttons[button_index].HalfTransitionCount = 0;
        }
        auto *keyboard = &Input.keyboard;
        for (i32 key_index = 0; key_index < ARRAY_COUNT(keyboard->buttons); key_index++)
        {
            keyboard->buttons[key_index].HalfTransitionCount = 0;
        }
        auto *keyboard_controller = &Input.KeyboardInput;
        for (int32 button_index = 0; button_index < ARRAY_COUNT(keyboard_controller->Buttons); button_index++)
        {
            keyboard_controller->Buttons[button_index].HalfTransitionCount = 0;
        }

        // @todo: clear up half transition count for other controllers.
        for (int gamepad_index = 0; gamepad_index < ARRAY_COUNT(gamepad_devices); gamepad_index++) {
            auto *gamepad_input = GetGamepadInput(&Input, gamepad_index);// GGame_ControllerInput* Controller = &Input.Controllers[gamepad_index];
            linux_gamepad* device = &gamepad_devices[gamepad_index];

            if (!linux_gamepad_connected(device)) {
                // Check if new gamepad is connected
                bool32 success = linux_gamepad_connect(device, global_gamepad_device_paths[gamepad_index]);
                if (!success) continue;

                // Success: new gamepad is connected
                // printf("Found new gamepad id: %d\n", gamepad_index);
                // @todo: Should I read events right away?
            } else {
                // Checking if already connected gamepad is plugged-off (by the player, by battery running out, or another reason)
                if (linux_gamepad_lost_connection(device)) {
                    // Gamepad is plugged off
                    linux_gamepad_disconnect(device);
                    // printf("Gamepad %d plugged off\n", gamepad_index);
                } else {
                    // Clear out HalfTransitionCount at the start of the frame.
                    for (int32 button_index = 0; button_index < ARRAY_COUNT(gamepad_input->Buttons); button_index++)
                    {
                        gamepad_input->Buttons[button_index].HalfTransitionCount = 0;
                    }

                    // 2. If gamepad is alive, read events from it:
                    linux_gamepad_process_events(device, gamepad_input);
                }
            }
        }

        XEvent event;
        while (XPending(display))
        {
            XNextEvent(display, &event);

            //
            // @note: X11 key-repeat BS:
            // The X-Server always sends you repeated keys. It's bad, because I want
            // to be precise when player hits/releases the keyboard key.
            // The event stream looks like this:
            //
            //    ... [real-press] ... [repeat-release]-[repeat-press]  [repeat-release]-[repeat-press]  [real-release] ...
            //
            // Repeat events always go in pairs: RELEASE-PRESS, and always have the same timestamp.
            // I detect that in the code below, and just skip them.
            //
            XEvent next_event;
            if (XPending(display))
            {
                // @note: XPeekEvent is blocking call. XPending makes sure I call XPeekEvent only when
                // there are events present in the queue.
                XPeekEvent(display, &next_event);
                if ((event.type == KeyRelease) && (next_event.type == KeyPress))
                {
                    if (event.xkey.time == next_event.xkey.time)
                    {
                        XNextEvent(display, &next_event); // Poll next event from the queue
                        continue; // Skip
                    }
                }
            }

            switch (event.type)
            {
                case MotionNotify:
                {
                    mouse->position = make_vector2i(event.xmotion.x, event.xmotion.y);
                }
                break;

                case ButtonPress:
                case ButtonRelease:
                {
                    b32 is_down = (event.type == ButtonPress);

                    if (event.xbutton.button == MOUSE_LMB)
                    {
                        linux_process_key_event(&mouse->LMB, is_down);
                    }
                    else if (event.xbutton.button == MOUSE_MMB)
                    {
                        linux_process_key_event(&mouse->MMB, is_down);
                    }
                    else if (event.xbutton.button == MOUSE_RMB)
                    {
                        linux_process_key_event(&mouse->RMB, is_down);
                    }
                }
                break;

                case KeyPress:
                case KeyRelease:
                {
                    b32 is_down = (event.type == KeyPress);
                    printf("%s (%d): vkcode: %d [%ld]\n", is_down ? "PRESS  " : "RELEASE", event.type, event.xkey.keycode, event.xkey.time);

                    if (event.xkey.keycode == KEYCODE_ESC)
                    {
                        global_running = false;
                    }
                    else if (event.xkey.keycode == KEYCODE_SPACE)
                    {
                        linux_gamepad_process_button(&keyboard_controller->Start, is_down);
                        // linux_process_key_event(&keyboard->Space, is_down);
                    }
                    else if (event.xkey.keycode == KEYCODE_W)
                    {
                        if (is_down) {
                            keyboard_controller->LeftStickEnded.y = clamp(keyboard_controller->LeftStickEnded.y + 1, -1, 1);
                        } else {
                            keyboard_controller->LeftStickEnded.y = clamp(keyboard_controller->LeftStickEnded.y - 1, -1, 1);
                        }
                    }
                    else if (event.xkey.keycode == KEYCODE_A)
                    {
                        if (is_down) {
                            keyboard_controller->LeftStickEnded.x = clamp(keyboard_controller->LeftStickEnded.x - 1, -1, 1);
                        } else {
                            keyboard_controller->LeftStickEnded.x = clamp(keyboard_controller->LeftStickEnded.x + 1, -1, 1);
                        }
                    }
                    else if (event.xkey.keycode == KEYCODE_S)
                    {
                        if (is_down) {
                            keyboard_controller->LeftStickEnded.y = clamp(keyboard_controller->LeftStickEnded.y - 1, -1, 1);
                        } else {
                            keyboard_controller->LeftStickEnded.y = clamp(keyboard_controller->LeftStickEnded.y + 1, -1, 1);
                        }
                    }
                    else if (event.xkey.keycode == KEYCODE_D)
                    {
                        if (is_down) {
                            keyboard_controller->LeftStickEnded.x = clamp(keyboard_controller->LeftStickEnded.x + 1, -1, 1);
                        } else {
                            keyboard_controller->LeftStickEnded.x = clamp(keyboard_controller->LeftStickEnded.x - 1, -1, 1);
                        }
                    }
                    else if (event.xkey.keycode == KEYCODE_LEFT)
                    {
                        linux_process_key_event(&keyboard->ArrowLeft, is_down);
                        linux_gamepad_process_button(&keyboard_controller->X, is_down);
                    }
                    else if (event.xkey.keycode == KEYCODE_RIGHT)
                    {
                        linux_process_key_event(&keyboard->ArrowRight, is_down);
                        linux_gamepad_process_button(&keyboard_controller->B, is_down);
                    }
                    else if (event.xkey.keycode == KEYCODE_UP)
                    {
                        linux_process_key_event(&keyboard->ArrowUp, is_down);
                        linux_gamepad_process_button(&keyboard_controller->Y, is_down);
                    }
                    else if (event.xkey.keycode == KEYCODE_DOWN)
                    {
                        linux_process_key_event(&keyboard->ArrowDown, is_down);
                        linux_gamepad_process_button(&keyboard_controller->A, is_down);
                    }
                    else if (event.xkey.keycode == KEYCODE_F1)
                    {
                        linux_process_key_event(&keyboard->F1, is_down);
                    }
                    else if (event.xkey.keycode == KEYCODE_CTRL)
                    {
                        linux_process_key_event(&keyboard->Ctrl, is_down);
                    }
                    else if (event.xkey.keycode == KEYCODE_SHIFT)
                    {
                        linux_process_key_event(&keyboard->Shift, is_down);
                    }
                    else if (event.xkey.keycode == KEYCODE_Z)
                    {
                        linux_process_key_event(&keyboard->Z, is_down);
                    }
                }
                break;

                // printf("keycode release: %d\n", e.keycode);
                // print_binary(e.state);
                // printf("\n");
                // printf(
                //     "[ B1 B2 B3 B4 B5 Shift Lock Control M1 M2 M3 M4 M5 ]\n"
                //     "[ %2d %2d %2d %2d %2d %5d %4d %7d %2d %2d %2d %2d %2d ]\n",
                //     (e.state & Button1Mask) > 0, // LMB
                //     (e.state & Button2Mask) > 0, // MMB
                //     (e.state & Button3Mask) > 0, // RMB
                //     (e.state & Button4Mask) > 0, //
                //     (e.state & Button5Mask) > 0, //
                //     (e.state & ShiftMask) > 0,   // Shift
                //     (e.state & LockMask) > 0,    // CapsLock
                //     (e.state & ControlMask) > 0, //
                //     (e.state & Mod1Mask) > 0,    //
                //     (e.state & Mod2Mask) > 0,    // NumLock
                //     (e.state & Mod3Mask) > 0,    //
                //     (e.state & Mod4Mask) > 0,    // Win/Super Key
                //     (e.state & Mod5Mask) > 0);   //

                case ClientMessage:
                    global_running = false;
                    break;
                case Expose:
                    int x = event.xexpose.x;
                    int y = event.xexpose.y;
                    int w = event.xexpose.width;
                    int h = event.xexpose.height;
                    // @todo: get new window buffer size
                    // linux_resize_screen_buffer(&screen_buffer, x + w, y + h);
                    break;
            }
        }

        os::duration target_microseconds_elapsed_for_frame { 33333 };

        Game::SoundOutputBuffer SoundBuffer {};

#if SOUND_ALSA
        snd_pcm_sframes_t buffer_size_in_frames = sound_output.buffer_size / sizeof(sound_sample_t);

        snd_pcm_sframes_t available_sound_frames = snd_pcm_avail(sound_output.sound_device);
        snd_pcm_sframes_t to_the_right = buffer_size_in_frames - write_cursor;
        snd_pcm_sframes_t to_the_left  = available_sound_frames - to_the_right;

        ASSERT(to_the_left + to_the_right == available_sound_frames);

        play_cursor = to_the_left;
        ASSERT(to_the_left + (buffer_size_in_frames - to_the_right - to_the_left) == write_cursor);

        // os::duration time_left_for_frame = target_microseconds_elapsed_for_frame - (os::get_wall_clock() - last_counter);
        // os::duration time_for_this_frame_and_the_next_one = time_left_for_frame + target_microseconds_elapsed_for_frame;

        snd_pcm_sframes_t n_sound_frames = target_microseconds_elapsed_for_frame.us * sound_output.samples_per_second / 1'000'000;

        // uint32 maximum_sound_frames = sound_output.buffer_size / sizeof(sound_sample_t);
        // uint32 already_written_samples = maximum_sound_frames - available_sound_frames;

        // uint32 nsamples = time_for_this_frame_and_the_next_one.us * sound_output.samples_per_second / 1'000'000;
        // // nsamples = nsamples - already_written_samples;
        // // nsamples = n_sound_frames;
        // printf(
        //     "[-----------------]\n"
        //     " play_cursor = %lu\n"
        //     "write_cursor = %lu\n"
        //     "snd_pcm_avail() => %lu\n"
        //     "latency = %5.2f\n"
        //     "time two flips ahead: %llu\n"
        //     "sound frames for one video frame: %lu\n"
        //     "sound frames for this and next one video frames: %u\n"
        //     // "already_written:  %u sound_frames\n"
        //     // "samples to write: %u sound_frames\n"
            // ,
            // play_cursor,
            // write_cursor,
            // available_sound_frames,
            // (float32)(write_cursor - play_cursor) / (float32)sound_output.samples_per_second
        //     time_for_this_frame_and_the_next_one.us,
        //     n_sound_frames,
        //     nsamples
        //     // already_written_samples, nsamples
            // );

        // How many frames I still need to write?
        // |-------------------|-------------------|
        //        ^               ^
        //       Now         WriteCursor
        //    PlayCursor

        SoundBuffer.SamplesPerSecond = sound_output.samples_per_second;
        SoundBuffer.SampleCount = n_sound_frames;
        SoundBuffer.Samples = sound_output.samples;
#endif

        Game::OffscreenBuffer GraphicsBuffer {};
        GraphicsBuffer.Memory = screen_buffer.memory;
        GraphicsBuffer.Width = screen_buffer.width;
        GraphicsBuffer.Height = screen_buffer.height;
        GraphicsBuffer.Pitch = screen_buffer.width * screen_buffer.bytes_per_pixel;
        GraphicsBuffer.BytesPerPixel = screen_buffer.bytes_per_pixel;

        Game_UpdateAndRender(&context, &game_memory, &Input, &GraphicsBuffer);

        // linux_send_sound_buffer(sound_output.sound_device, &sound_output, n_sound_frames);
        // {
        //     write_cursor = (write_cursor + n_sound_frames) % (sound_output.buffer_size / sizeof(sound_sample_t));
        // }

        linux_copy_buffer_to_window(&screen_buffer, display, window, screen);

        os::timepoint counter = os::get_wall_clock();
        // uint64 cycles = os::get_processor_cycles();

        os::duration microseconds_elapsed = counter - last_counter;
        // uint64 megacycles_elapsed = cycles - last_cycles;

        os::duration microseconds_elapsed_for_frame = microseconds_elapsed;

        if (microseconds_elapsed_for_frame < target_microseconds_elapsed_for_frame) {
            // printf("%u us/f\n", microseconds_elapsed_for_frame);
            while (microseconds_elapsed_for_frame < target_microseconds_elapsed_for_frame) {
                os::duration sleep_time = target_microseconds_elapsed_for_frame - microseconds_elapsed;
                // @todo: achieve sleep granularity or just switch to OpenGL's vsync stuff
                // {
                //     timespec req {};
                //     timespec rem {};
                //     req.tv_nsec = sleep_time.us * 1000;
                //     nanosleep(&req, &rem);
                // }
                // if (sleep_time.us > process_timer_granularity_ms * 1000) {
                //     usleep(sleep_time.us - process_timer_granularity_ms);
                // }
                counter = os::get_wall_clock();
                microseconds_elapsed_for_frame = counter - last_counter;
            }

            if (microseconds_elapsed_for_frame < target_microseconds_elapsed_for_frame) {
                // @note: slept for a good time
            } else {
                // @todo: handle missed frame rate!
                // printf("Missed frame!\n");
            }
        } else {
            // @todo: handle missed frame rate!
            // printf("Missed frame!!!\n");
        }

        last_counter = counter;

        {
            // printf("%llu us/f; %6.3f fps\n", microseconds_elapsed_for_frame.us, 1000000.f / microseconds_elapsed_for_frame.us);
        }

        // last_cycles = os::get_processor_cycles();

        // printf("milliseconds elapsed: %5.2f\n", microseconds_elapsed / 1000.f);

        {
            f32 milliseconds_elapsed = microseconds_elapsed_for_frame.us / 1000.0f;
            f32 fps = 1000000.0f / microseconds_elapsed_for_frame.us;

            char window_text[256];
            sprintf(window_text, "%f ms/f; fps: %f", milliseconds_elapsed, fps);
            XStoreName(display, window, window_text);
        }
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
