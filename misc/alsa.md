# ALSA

This file provides information to programmers about ALSA API, its terms, and how to use it.

## Buffer

There's two buffers: one on the sound card, one in application memory.
When buffer is sufficiently full, kernel generates an interrupt, to copy buffer to the sound card
using Direct Memory Access (DMA).
These buffers are ring buffers, meaning the data wraps back to the start when the end of the buffer is reached.

The size of the buffer can be quite large. To solve latency issues of copying large buffers, ALSA splits the buffer up
into a series of periods (fragments in OSS) and transfers the data in units of a period.

A period stores frames, each of which contains samples captured at one point in time. For a stereo device, the frame contains
samples for two channels.

    Sample:  [LSB|MSB]       // Least/Most Significant Byte (the order of the little-endian machine)
    Frame:   [LEFT|RIGHT]    // For 2 channels in interleaved
    Period:  [LR|LR|...|LR]  // Period consists of several frames (for ALSA DMA optimisations)
    Buffer:  [P|P|...|P]     // Buffer conststs of periods

When a sound device is active, data is transferred continuously between the hardware and application.

## Usage

Programs that use the PCM interface generally follow following algorithm:

```
sound_device = open_interface();
set_hardware_parameters(sound_device); // access mode, data format, channels, rate, etc.

while (there_is_data_to_be_processed) {
   write_PCM_data(sound_device);
}

close_interface(sound_device);
```

### Types and functions

**TODO:** give sufficient explanations of used types and function

#### Types

`snd_pcm_t` -

`snd_pcm_hw_params_t`

#### Functions

`snd_pcm_open`

`snd_pcm_open`

`snd_pcm_hw_params_malloc`

`snd_pcm_hw_params_any`

`snd_pcm_hw_params_set_rate_resample`

`snd_pcm_hw_params_set_access`

`snd_pcm_hw_params_set_format`

`snd_pcm_hw_params_set_channels`

`snd_pcm_hw_params_set_buffer_time`

`snd_pcm_hw_params_set_buffer_size`

`snd_pcm_hw_params_set_rate_near`

`snd_pcm_hw_params`

`snd_pcm_hw_params_get_buffer_time`

`snd_pcm_hw_params_get_buffer_size`

`snd_pcm_hw_params_get_rate`

`snd_pcm_prepare`
