#include "format_png.hpp"
#include "crc.hpp"


const static uint32 PNG_SIGNATURE_SIZE = 8;

uint32 change_endianess(uint32 n) {
    return ((n & 0xFF000000) >> 24)
         | ((n & 0x00FF0000) >> 8)
         | ((n & 0x0000FF00) << 8)
         | ((n & 0x000000FF) << 24);
}

DECLARE_BIT_FIELD_T(PNG_ColorType, uint8,
    PNG_PALETTE_USED = 0x1,
    PNG_COLOR_USED = 0x2,
    PNG_ALPHA_CHANNEL_USED = 0x4
);

#pragma pack(push, 1)
struct PNG_ChunkHeader {
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
    PNG_ColorType color_type;
    uint8  compression_method;
    uint8  filter_method;
    uint8  interlace_method;
};
#pragma pack(pop)

// Constants for rendering intent for sRGB chunk
enum PNG_RenderingIntent : uint8 {
    PNG_PERCEPTUAL = 0,
    PNG_RELATIVE_COLORIMETRIC = 1,
    PNG_SATURATION = 2,
    PNG_ABSOLUTE_COLORIMETRIC = 3,
};

// Available constants for interlace_method field in PNG_IHDRHeader struct
enum PNG_InterlaceMethod : uint8 {
    PNG_INTERLACE_NONE = 0,
    PNG_INTERLACE_ADAM7 = 1,
};

void* load_png_file(const char* filename) {
    auto result = os::load_entire_file(filename);

    uint8* signature = (uint8*) result.memory;
    // @todo: do runtime checking in all builds
    ASSERT(signature[0] == 137 &&
           signature[1] == 80  &&
           signature[2] == 78  &&
           signature[3] == 71  &&
           signature[4] == 13  &&
           signature[5] == 10  &&
           signature[6] == 26  &&
           signature[7] == 10);

    uint8* chunk = (uint8*)((uint8*)result.memory + PNG_SIGNATURE_SIZE);

    bool end = false;
    PNG_IHDRHeader header{};

    // @todo: check if comparing pointers is allowed
    while (chunk < ((uint8*)result.memory + result.size)) {
        //
        // Chunks of PNG file follow this layout:
        //
        // +-------------------+-------------+----------------+
        // |  PNG_ChunkHeader  |  Data  ...  |  PNG_ChunkCRC  |
        // +-------------------+-------------+----------------+
        // |<-    8 bytes    ->|<- N bytes ->|<-  4  bytes  ->|
        //
        PNG_ChunkHeader* chunk_header = (PNG_ChunkHeader*)chunk;
        uint32 size = change_endianess(chunk_header->size_of_data);
        uint8* data = chunk + sizeof(PNG_ChunkHeader);
        uint32 crc  = change_endianess(*(uint32*)(data + size));

        if (chunk_header->type[0] == 'I' &&
            chunk_header->type[1] == 'H' &&
            chunk_header->type[2] == 'D' &&
            chunk_header->type[3] == 'R')
        {
            PNG_IHDRHeader* ihdr = (PNG_IHDRHeader*)data;
            header.width = change_endianess(ihdr->width);
            header.height = change_endianess(ihdr->height);
            header.bit_depth = ihdr->bit_depth;
            header.color_type = ihdr->color_type;
            header.compression_method = ihdr->compression_method;
            header.filter_method = ihdr->filter_method;
            header.interlace_method = ihdr->interlace_method;

            ASSERT(header.interlace_method == PNG_INTERLACE_NONE); // Adam7 interlace method is not supported.

            OutputDebugStringA(header.color_type.get(PNG_ColorType::PNG_ALPHA_CHANNEL_USED) ? "true" : "false");
        }
        else if (chunk_header->type[0] == 's' &&
                 chunk_header->type[1] == 'R' &&
                 chunk_header->type[2] == 'G' &&
                 chunk_header->type[3] == 'B')
        {
            PNG_RenderingIntent rendering_intent = *(PNG_RenderingIntent*)data;

            OutputDebugStringA("Rendering intent: ");
            switch (rendering_intent) {
                case PNG_PERCEPTUAL: {
                    OutputDebugStringA("PNG_PERCEPTUAL\n");
                    break;
                }
                case PNG_RELATIVE_COLORIMETRIC: {
                    OutputDebugStringA("PNG_RELATIVE_COLORIMETRIC\n");
                    break;
                }
                case PNG_SATURATION: {
                    OutputDebugStringA("PNG_SATURATION\n");
                    break;
                }
                case PNG_ABSOLUTE_COLORIMETRIC: {
                    OutputDebugStringA("PNG_ABSOLUTE_COLORIMETRIC\n");
                    break;
                }
            }
        }
        else if (chunk_header->type[0] == 'I' &&
                 chunk_header->type[1] == 'D' &&
                 chunk_header->type[2] == 'A' &&
                 chunk_header->type[3] == 'T')
        {
            int x = 0;
        }
        else if (chunk_header->type[0] == 'I' &&
                 chunk_header->type[1] == 'E' &&
                 chunk_header->type[2] == 'N' &&
                 chunk_header->type[3] == 'D')
        {
            // IEND chunk has no data.
            end = true;
        }

        // A 4-byte CRC calculated on the preceding bytes in the chunk,
        // including the chunk type code and chunk data fields,
        // but not including the length field.
        uint32 data_crc = png::crc(data - 4, size + 4);

        ASSERT_MSG(data_crc == crc, "Debug break when CRCs do not match.\n");
        if (data_crc != crc) {
            // @todo: log the error in CRC and free memory correctly
            break;
        }

        if (end) break;
        chunk = data + size + sizeof(PNG_ChunkCRC);
    }

    return nullptr;
}
