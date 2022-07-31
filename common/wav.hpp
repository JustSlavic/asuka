#ifndef ASUKA_COMMON_WAV_HPP
#define ASUKA_COMMON_WAV_HPP

#include <defines.hpp>


struct wav_file_contents {
    i32 samples_per_second;
    i32 channels;
    sound_sample_t* samples;
    u64 samples_count;
};


wav_file_contents load_wav_file(const char* filename);


#if UNITY_BUILD
#include "wav.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_WAV_HPP
