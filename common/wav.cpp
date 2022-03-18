#include "wav.hpp"
#include "os/file.hpp"


struct RIFF_Header {
    u32 ChunkID;
    u32 ChunkSize;
    u8  Format[4];
};


struct WAV_Format {
    u32 Subchunk1ID;
    u32 Subchunk1Size;
    u16 AudioFormat;
    u16 NumChannels;
    u32 SampleRate;
    u32 ByteRate;
    u16 BlockAlign;
    u16 BitsPerSample;
};


struct WAV_Data {
    u32 Subchunk2ID;
    u32 Subchunk2Size;
};


#define WAV_MAGIC_NUMBER(a, b, c, d) ((((u32)a) << 0) | (((u32)b) << 8) | (((u32)c) << 16) | (((u32)d) << 24))

enum {
    WAV_RIFF_ID = WAV_MAGIC_NUMBER('R', 'I', 'F', 'F'),
    WAV_FMT_ID  = WAV_MAGIC_NUMBER('f', 'm', 't', ' '),
    WAV_DATA_ID = WAV_MAGIC_NUMBER('d', 'a', 't', 'a'),
};


INTERNAL_FUNCTION
u8 *consume_memory(u8 **data, u64 size) {
    u8 *result = *data;

    *data += size;

    return result;
}

#define WAV_CONSUME_STRUCT(POINTER, TYPE) \
    (TYPE *)consume_memory(&POINTER, sizeof(TYPE)); void(0)


wav_file_contents load_wav_file(const char* filename) {
    wav_file_contents result {};

    asuka::string contents = os::load_entire_file(filename);
    if (contents.size == 0) {
        // @todo: handle error
        return result;
    }

    u8 *data = (u8 *)contents.data;

    RIFF_Header riff_header {};
    WAV_Format wav_format {};
    WAV_Data wav_data {};

    bool processing_headers = true;
    while (processing_headers) {
        u32 header_id = *(u32*)data;

        switch (header_id) {
            case WAV_RIFF_ID: {
                RIFF_Header *header = WAV_CONSUME_STRUCT(data, RIFF_Header);
                riff_header = *header;
                break;
            }
            case WAV_FMT_ID: {
                WAV_Format *header = WAV_CONSUME_STRUCT(data, WAV_Format);
                wav_format = *header;
                break;
            }
            case WAV_DATA_ID: {
                WAV_Data *header = WAV_CONSUME_STRUCT(data, WAV_Data);
                wav_data = *header;
                
                processing_headers = false;
                break;
            }
            default: {
                // @todo: handle error
                ASSERT_FAIL("Failed to read WAV chunk.\n");
                break;
            }
        }
    }

    sound_sample_t *samples = (sound_sample_t *) data;

    result.samples_per_second = wav_format.SampleRate;
    result.channels = wav_format.NumChannels;
    result.samples_count = wav_data.Subchunk2Size * 8 / wav_format.BitsPerSample;
    result.samples = samples;

    ASSERT(result.samples_per_second == 48000);
    ASSERT(result.channels == 2);

    return result;
}
