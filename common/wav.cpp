#include "wav.hpp"
#include "os/file.hpp"


struct RIFF_Header {
    uint8  ChunkID[4];
    uint32 ChunkSize;
    uint8  Format[4];
};


struct WAV_Format {
    uint8  Subchunk1ID[4];
    uint32 Subchunk1Size;
    uint16 AudioFormat;
    uint16 NumChannels;
    uint32 SampleRate;
    uint32 ByteRate;
    uint16 BlockAlign;
    uint16 BitsPerSample;
};


struct WAV_Data {
    uint8  Subchunk2ID[4];
    uint32 Subchunk2Size;
    int16* Data;
};


wav_file_contents load_wav_file(const char* filename) {
    wav_file_contents result {};

    os::file_read_result file_contents = os::load_entire_file(filename);
    if (file_contents.size == 0) {
        // @todo: handle error
        return result;
    }

    uint8* data = (uint8*)file_contents.memory;

    RIFF_Header* header = (RIFF_Header*) data;
    if (header->ChunkID[0] != 'R' ||
        header->ChunkID[1] != 'I' ||
        header->ChunkID[2] != 'F' ||
        header->ChunkID[3] != 'F')
    {
        // @todo: handle error
        return result;
    }

    WAV_Format *wav_format = (WAV_Format *) (data + sizeof(RIFF_Header));
    if (wav_format->Subchunk1ID[0] != 'f' ||
        wav_format->Subchunk1ID[1] != 'm' ||
        wav_format->Subchunk1ID[2] != 't' ||
        wav_format->Subchunk1ID[3] != ' ')
    {
        // @todo: handle error
        return result;
    }

    WAV_Data *wav_data = (WAV_Data *) (data + sizeof(RIFF_Header) + sizeof(WAV_Format));
    if (wav_data->Subchunk2ID[0] != 'd' ||
        wav_data->Subchunk2ID[1] != 'a' ||
        wav_data->Subchunk2ID[2] != 't' ||
        wav_data->Subchunk2ID[3] != 'a')
    {
        // @todo: handle error
        return result;
    }

    result.samples_per_second = wav_format->SampleRate;
    result.channels = wav_format->NumChannels;
    result.samples = wav_data->Data;
    result.samples_count = wav_data->Subchunk2Size * 8 / wav_format->BitsPerSample;

    return result;
}

