/*

Reference: http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

*/


#include "png.hpp"
#include "os/file.hpp"
#include "crc.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <external/stb_image.h>


#pragma pack(push, 1)
struct PNG_ChunkHeader {
    uint32 size_of_data; // in bytes
    uint32 type; // type code
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

//
enum PNG_ColorType : uint8 {
    PNG_PALETTE_USED = 0x1,
    PNG_COLOR_USED = 0x2,
    PNG_ALPHA_CHANNEL_USED = 0x4
};

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


uint32 change_endianess(uint32 n) {
    return ((n & 0xFF000000) >> 24)
         | ((n & 0x00FF0000) >> 8)
         | ((n & 0x0000FF00) << 8)
         | ((n & 0x000000FF) << 24);
}


#define PNG_MAGIC_NUMBER(a, b, c, d) ((((uint32)a) << 0) | (((uint32)b) << 8) | (((uint32)c) << 16) | (((uint32)d) << 24))

enum {
    PNG_SIGNATURE_1 = PNG_MAGIC_NUMBER(137, 80, 78, 71),
    PNG_SIGNATURE_2 = PNG_MAGIC_NUMBER(13, 10, 26, 10),
    PNG_IHDR_ID = PNG_MAGIC_NUMBER('I', 'H', 'D', 'R'),
    PNG_sRGB_ID = PNG_MAGIC_NUMBER('s', 'R', 'G', 'B'),
    PNG_IDAT_ID = PNG_MAGIC_NUMBER('I', 'D', 'A', 'T'),
    PNG_IEND_ID = PNG_MAGIC_NUMBER('I', 'E', 'N', 'D'),
};

namespace png {

INTERNAL_FUNCTION
uint8 *consume_memory(uint8 **data, uint64 size) {
    uint8 *result = *data;
    *data += size;
    return result;
}

using crc_t = uint32;

} // png


#define PNG_CONSUME_STRUCT(POINTER, TYPE) \
    (TYPE *)png::consume_memory(&POINTER, sizeof(TYPE)); void(0)


void decode_idat_chunk(uint8 *data, usize size) {

}

bitmap load_png_file_myself(const char *filename) {
    bitmap result {};

    os::file_read_result file_contents = os::load_entire_file(filename);

    uint8 *data = (uint8 *)file_contents.memory;

    uint32 *signature1 = PNG_CONSUME_STRUCT(data, uint32);
    uint32 *signature2 = PNG_CONSUME_STRUCT(data, uint32);

    ASSERT(*signature1 == PNG_SIGNATURE_1);
    ASSERT(*signature2 == PNG_SIGNATURE_2);

    bool end = false;
    PNG_IHDRHeader ihdr_header {};

    while (data < ((uint8 *)file_contents.memory + file_contents.size)) {
        //
        // Chunks of PNG file follow this layout:
        //
        // +-------------------+-------------+----------------+
        // |  PNG_ChunkHeader  |  Data  ...  |  PNG_ChunkCRC  |
        // +-------------------+-------------+----------------+
        // |<-    8 bytes    ->|<- N bytes ->|<-  4  bytes  ->|
        //

        PNG_ChunkHeader chunk_header = *PNG_CONSUME_STRUCT(data, PNG_ChunkHeader);
        chunk_header.size_of_data = change_endianess(chunk_header.size_of_data);

        uint32 chunk_type_size = sizeof(chunk_header.type);
        png::crc_t computed_crc = change_endianess(compute_crc(data - chunk_type_size, chunk_header.size_of_data + chunk_type_size));

        if (chunk_header.type == PNG_IHDR_ID) {
            PNG_IHDRHeader *ihdr = PNG_CONSUME_STRUCT(data, PNG_IHDRHeader);

            ihdr_header.width = change_endianess(ihdr->width);
            ihdr_header.height = change_endianess(ihdr->height);
            ihdr_header.bit_depth = ihdr->bit_depth;
            ihdr_header.color_type = ihdr->color_type;
            ihdr_header.compression_method = ihdr->compression_method;
            ihdr_header.filter_method = ihdr->filter_method;
            ihdr_header.interlace_method = ihdr->interlace_method;

            // Only compression method 0 (deflate/inflate compression with
            // a sliding window of at most 32768 bytes) is defined.
            // All standard PNG images must be compressed with this scheme.
            ASSERT(ihdr_header.compression_method == 0);

            // Adam7 interlace method is not supported.
            ASSERT(ihdr_header.interlace_method == PNG_INTERLACE_NONE);
        }

        if (chunk_header.type == PNG_sRGB_ID) {
            PNG_RenderingIntent *rendering_intent = PNG_CONSUME_STRUCT(data, PNG_RenderingIntent);
        }

        if (chunk_header.type == PNG_IDAT_ID) {
            decode_idat_chunk(data, chunk_header.size_of_data);
            data += chunk_header.size_of_data;
        }

        if (chunk_header.type == PNG_IEND_ID) {
            // IEND chunk has no data.
            end = true;
        }

        png::crc_t *chunk_crc = PNG_CONSUME_STRUCT(data, png::crc_t);
        
        ASSERT(*chunk_crc == computed_crc);
    }

    return result;
}


bitmap load_png_file(const char* filename) {
    bitmap result {};

    os::file_read_result file_contents = os::load_entire_file(filename);
    if (file_contents.memory == NULL) {
        // @todo: handle error
        return result;
    }

    int32 x, y, n;
    uint8 *pixels = stbi_load(filename, &x, &y, &n, 0);

    result.pixels = pixels;
    result.size = x * y * n;
    result.width = x;
    result.height = y;
    result.bytes_per_pixel = n;

    return result;
}
