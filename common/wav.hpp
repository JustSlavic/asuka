#ifndef ASUKA_COMMON_WAV_HPP
#define ASUKA_COMMON_WAV_HPP


struct wav_file_contents {
    int32  samples_per_second;
    int32  channels;
    sound_sample_t* samples;
    uint64 samples_count;
};


wav_file_contents load_wav_file(const char* filename);


#ifdef UNITY_BUILD
#include "wav.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_WAV_HPP
