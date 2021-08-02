#include "format_png.hpp"


const static uint32 PNG_SIGNATURE_SIZE = 8;

uint32 change_endianess(uint32 n) {
    return ((n & 0xFF000000) >> 24)
         | ((n & 0x00FF0000) >> 8)
         | ((n & 0x0000FF00) << 8)
         | ((n & 0x000000FF) << 24);
}

#pragma pack(push, 1)
struct PNG_Chunk {
    uint32 size_of_data; // in bytes
    uint8  type[4]; // type code
};

struct PNG_ChunkCRC {
    uint32 crc;
};

struct PNG_IHDRHeader {
    uint32 width;
    uint32 height;
    uint8  bit_depth;
    uint8  color_type;
    uint8  compression_method;
    uint8  filter_method;
    uint8  interlace_method;
};
#pragma pack(pop)

enum rendering_intent {
    perceptual = 0,
    relative_colorimetric = 1,
    saturation = 2,
    absolute_colorimetric = 3,
};

void* load_png_file(const char* filename) {
    auto result = os::load_entire_file(filename);

    uint8* signature = (uint8*) result.memory;
    ASSERT(signature[0] == 137 &&
           signature[1] == 80  &&
           signature[2] == 78  &&
           signature[3] == 71  &&
           signature[4] == 13  &&
           signature[5] == 10  &&
           signature[6] == 26  &&
           signature[7] == 10);

    uint8* chunks = (uint8*)((uint8*)result.memory + PNG_SIGNATURE_SIZE);

    bool end = false;
    PNG_IHDRHeader header{};

    // @todo: check if comparing pointers is allowed
    while (chunks < ((uint8*)result.memory + result.size)) {
        //
        // Chunks of PNG file follow this layout:
        //
        // +-------------+-------------+----------------+
        // |  PNG_Chunk  |  Data  ...  |  PNG_ChunkCRC  |
        // +-------------+-------------+----------------+
        // |<- 8 bytes ->|<- N bytes ->|<-  4  bytes  ->|
        //
        PNG_Chunk* chunk_header = (PNG_Chunk*)chunks;
        uint32 size = change_endianess(chunk_header->size_of_data);
        uint8* data = chunks + sizeof(PNG_Chunk);

        if (chunk_header->type[0] == 'I' &&
            chunk_header->type[1] == 'H' &&
            chunk_header->type[2] == 'D' &&
            chunk_header->type[3] == 'R')
        {
            // This is IHDR chunk
            PNG_IHDRHeader* ihdr = (PNG_IHDRHeader*)data;
            header.width = change_endianess(ihdr->width);
            header.height = change_endianess(ihdr->height);
            header.bit_depth = ihdr->bit_depth;
            header.color_type = ihdr->color_type;
            header.compression_method = ihdr->compression_method;
            header.filter_method = ihdr->filter_method;
            header.interlace_method = ihdr->interlace_method;
        }
        else if (chunk_header->type[0] == 's' &&
                 chunk_header->type[1] == 'R' &&
                 chunk_header->type[2] == 'G' &&
                 chunk_header->type[3] == 'B')
        {
            //
            // The sRGB chunk contains:
            // Rendering intent: 1 byte
            // 
            // The following values are defined for the rendering intent:
            //    0: Perceptual
            //    1: Relative colorimetric
            //    2: Saturation
            //    3: Absolute colorimetric
            //
            int x = 0;
        }
        else if (chunk_header->type[0] == 'I' &&
                 chunk_header->type[1] == 'D' &&
                 chunk_header->type[2] == 'A' &&
                 chunk_header->type[3] == 'T')
        {
            // This is IDAT chunk
            int x = 0;
        }
        else if (chunk_header->type[0] == 'I' &&
                 chunk_header->type[1] == 'E' &&
                 chunk_header->type[2] == 'N' &&
                 chunk_header->type[3] == 'D')
        {
            // This is IEND chunk
            end = true;
        }

        // @todo: check CRC for correctness
        PNG_ChunkCRC* chunk_crc = (PNG_ChunkCRC*)(data + size);
        uint32 crc = change_endianess(chunk_crc->crc);

        if (end) break;
        chunks = data + size + sizeof(PNG_ChunkCRC);
    }

    return nullptr;
}

