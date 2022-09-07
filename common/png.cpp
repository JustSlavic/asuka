/*

Reference: http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

*/


#include "png.hpp"
#include "os/file.hpp"
#include "crc.hpp"

#include "stdlib.h"

#if 1
#define STB_IMAGE_IMPLEMENTATION
#include <external/stb_image.h>
#endif


#pragma pack(push, 1)
struct PNG_ChunkHeader {
    u32 size_of_data; // in bytes
    u32 type;
};

struct PNG_IHDRHeader {
    u32 width;
    u32 height;
    u8  bit_depth;
    u8  color_type;
    u8  compression_method;
    u8  filter_method;
    u8  interlace_method;
};
#pragma pack(pop)

//
enum PNG_ColorType : u8 {
    PNG_PALETTE_USED = 0x1,
    PNG_COLOR_USED = 0x2,
    PNG_ALPHA_CHANNEL_USED = 0x4
};

// Constants for rendering intent for sRGB chunk
enum PNG_RenderingIntent : u8 {
    PNG_PERCEPTUAL = 0,
    PNG_RELATIVE_COLORIMETRIC = 1,
    PNG_SATURATION = 2,
    PNG_ABSOLUTE_COLORIMETRIC = 3,
};

// Available constants for interlace_method field in PNG_IHDRHeader struct
enum PNG_InterlaceMethod : u8 {
    PNG_INTERLACE_NONE = 0,
    PNG_INTERLACE_ADAM7 = 1,
};


u32 change_endianess(u32 n) {
    return ((n & 0xFF000000) >> 24)
         | ((n & 0x00FF0000) >> 8)
         | ((n & 0x0000FF00) << 8)
         | ((n & 0x000000FF) << 24);
}


#define PNG_MAGIC_NUMBER(a, b, c, d) ((((u32)a) << 0) | (((u32)b) << 8) | (((u32)c) << 16) | (((u32)d) << 24))

enum {
    PNG_SIGNATURE_1 = PNG_MAGIC_NUMBER(137, 80, 78, 71),
    PNG_SIGNATURE_2 = PNG_MAGIC_NUMBER(13, 10, 26, 10),
    PNG_IHDR_ID = PNG_MAGIC_NUMBER('I', 'H', 'D', 'R'),
    PNG_sRGB_ID = PNG_MAGIC_NUMBER('s', 'R', 'G', 'B'),
    PNG_IDAT_ID = PNG_MAGIC_NUMBER('I', 'D', 'A', 'T'),
    PNG_IEND_ID = PNG_MAGIC_NUMBER('I', 'E', 'N', 'D'),
};

namespace png {

INTERNAL
u8 *consume_memory(u8 **data, u64 size) {
    u8 *result = *data;
    *data += size;
    return result;
}

using crc_t = u32;

} // png

#define PNG_CONSUME_STRUCT(POINTER, TYPE) \
    (TYPE *)png::consume_memory(&POINTER, sizeof(TYPE)); void(0)


struct bit_fetcher {
    u8* buffer;
    u32 bits;
    u32 available_bits;
};

struct huffman_code {

};

INTERNAL
u32 get_bits(bit_fetcher* fetcher, u32 n) {
    ASSERT(n <= 32);

    u32 result;

    if (fetcher->available_bits < n) {
        do {
            fetcher->bits |= ((u32)(*fetcher->buffer++)) << fetcher->available_bits;
            fetcher->available_bits += 8;
        } while (fetcher->available_bits <= 24);
    }

    result = fetcher->bits & ((1 << n) - 1);
    fetcher->bits = fetcher->bits >> n;
    fetcher->available_bits -= n;

    return result;
}

INTERNAL
huffman_code generate_huffman() {
    huffman_code result {};

    return result;
}

INTERNAL
u32 decode_huffman(huffman_code *huffman) {
    u32 result {};

    return result;
}

INTERNAL
void decode_idat_chunk(u8 *data, usize size, Bitmap *result) {
    u8 CMF = *data;
    u8 CM  = 0x0F & CMF; // CompressionMethod

    ASSERT(CM == 8); // CM=8 denotes 'deflate' algorithm of compression

    u8 CINFO = (0xF0 & CMF) >> 4; // CompressionINFO

    // CINFO = log_2(LZ77_window_size) - 8;
    // Therefore => u32 LZ77_window_size = 1 << (CINFO + 8);
    //
    // CINFO=7 indicates 32768 window size.

    ASSERT(CINFO < 8); // CINFO above 7 are not allowed by the specification
    // CompressionINFO is not defined if CompressionMethod != 8

    u32 LZ77_window_size = 1 << (CINFO + 8);

    u8 FLG = *(data + 1); // Flags

    u8 FCHECK = (0x1F & FLG);
    u8 FDICT  = (0x20 & FLG) >> 5;
    u8 FLEVEL = (0xC0 & FLG) >> 6;

    ASSERT((CMF * 256 + FLG) % 31 == 0); // The FCHECK value must be such that CMF and FLG, when viewed as
                                         // a 16-bit unsigned integer stored in MSB order (CMF*256 + FLG),
                                         // is a multiple of 31.
    ASSERT(FDICT == 0);

    u8* zlib_data = data + 2;

    bit_fetcher fetcher;
    fetcher.buffer = zlib_data;
    fetcher.bits = 0;
    fetcher.available_bits = 0;

    u8 BFINAL;
    u8 BTYPE;
    do {
        BFINAL = (u8)get_bits(&fetcher, 1); // ((*zlib_data) & 0b0000'0001);
        BTYPE  = (u8)get_bits(&fetcher, 2); // ((*zlib_data) & 0b0000'0110) >> 1;

        ASSERT(BTYPE != 3); // @debug: reserved value for error!

        if (BTYPE == 0) { // Stored with no compression.
            zlib_data += 1; // Skip remaining bits in currently processed byte.

            u16 LEN = *(u16*)zlib_data;
            zlib_data += 2;

            u16 NLEN = *(u16*)zlib_data;
            zlib_data += 2;

            ASSERT(LEN == ~NLEN);
            // Test this on something.
        } else {
            if (BTYPE == 2) { // Compressed with dynamic Huffman code

                // read representation of code trees (see subsection below)

                u32 HLIT  = get_bits(&fetcher, 5) + 257;
                u32 HDIST = get_bits(&fetcher, 5) + 1;
                u32 HCLEN = get_bits(&fetcher, 4) + 4;

                // (HCLEN + 4) x 3 bits: code lengths for the code length
                // alphabet given just above, in the order: 16, 17, 18,
                // 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15

                ASSERT(HCLEN <= 19);
                u8 code_lengths_order[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
                u8 code_lengths[19] {};

                for (u32 i = 0; i < HCLEN; i++) {
                    code_lengths[code_lengths_order[i]] = (u8)get_bits(&fetcher, 3);
                }

                // DEFLATE algorithm to generate codes
                // 1) Count the number of codes for each code length.
                //    Let bl_count[N] be the number of codes of length N, N >= 1.

                u32 bl_count[19] {};

                for (int i = 0; i < 19; i++) {
                    bl_count[code_lengths[i]]++;
                }

                // 2) Find the numerical value of the smallest code for each code length:

                i32 code = 0;
                bl_count[0] = 0;
                u32 next_code[16] {};

                for (u32 bits = 1; bits < 16; bits++) {
                    code = (code + bl_count[bits-1]) << 1;
                    next_code[bits] = code;
                }

                // u32 max_code; // should be used later? ??

                // 3) Assign numerical values to all codes, using consecutive
                //    values for all codes of the same length with the base
                //    values determined at step 2. Codes that are never used
                //    (which have a bit length of zero) must not be assigned a
                //    value.

                // for (u32 n = 0;  n <= max_code; n++) {
                //     len = tree[n].Len;
                //     if (len != 0) {
                //         tree[n].Code = next_code[len];
                //         next_code[len]++;
                //     }
                // }

            } else if (BTYPE == 1) {
                // Compressed with fixed Huffman code
                // ???
            }

#if 0
            while (true /* until end of block recognized */) {
                huffman_code test_ {};
                u32 literal_length_value = decode_huffman(&test_);
                if (literal_length_value < 256) {
                    // copy value (literal byte) to output stream
                } else {
                    if (literal_length_value == 256) // end of block
                        break;
                    else {
                        // decode distance from input stream

                        // move backwards distance bytes in the output
                        // stream, and copy length bytes from this
                        // position to the output stream.

                    }
                }
            }
#endif

        }
    } while (BFINAL == 0);

    u8 debug_ = 0;

    // do
    //    read block header from input stream.
    //    if stored with no compression
    //       skip any remaining bits in current partially
    //          processed byte
    //       read LEN and NLEN (see next section)
    //       copy LEN bytes of data to output
    //    otherwise
    //       if compressed with dynamic Huffman codes
    //          read representation of code trees (see
    //             subsection below)
    //       loop (until end of block code recognized)
    //          decode literal/length value from input stream
    //          if value < 256
    //             copy value (literal byte) to output stream
    //          otherwise
    //             if value = end of block (256)
    //                break from loop
    //             otherwise (value = 257..285)
    //                decode distance from input stream

    //                move backwards distance bytes in the output
    //                stream, and copy length bytes from this
    //                position to the output stream.
    //       end loop
    // while not last block
}

Bitmap load_png_file_myself(const char *filename) {
    Bitmap result {};

    byte_array contents = os::load_entire_file(filename);

    u8 *data = (u8 *) contents.get_data();

    u32 signature1 = *PNG_CONSUME_STRUCT(data, u32);
    u32 signature2 = *PNG_CONSUME_STRUCT(data, u32);

    ASSERT(signature1 == PNG_SIGNATURE_1);
    ASSERT(signature2 == PNG_SIGNATURE_2);

    bool end = false;

    while (data < ((u8 *)contents.data + contents.size)) {
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

        u32 chunk_type_size = sizeof(chunk_header.type);
        png::crc_t computed_crc = change_endianess(compute_crc(data - chunk_type_size, chunk_header.size_of_data + chunk_type_size));

        if (chunk_header.type == PNG_IHDR_ID) {
            PNG_IHDRHeader *ihdr = PNG_CONSUME_STRUCT(data, PNG_IHDRHeader);

            result.width = change_endianess(ihdr->width);
            result.height = change_endianess(ihdr->height);
            result.bytes_per_pixel = ihdr->bit_depth / 8;
            result.size = result.width * result.height * result.bytes_per_pixel;
            result.pixels = malloc(result.size);

            // Only compression method 0 (deflate/inflate compression with
            // a sliding window of at most 32768 bytes) is defined.
            // All standard PNG images must be compressed with this scheme.
            ASSERT(ihdr->compression_method == 0);

            // Adam7 interlace method is not supported.
            ASSERT(ihdr->interlace_method == PNG_INTERLACE_NONE);
        }

        if (chunk_header.type == PNG_sRGB_ID) {
            PNG_RenderingIntent *rendering_intent = PNG_CONSUME_STRUCT(data, PNG_RenderingIntent);
        }

        if (chunk_header.type == PNG_IDAT_ID) {
            decode_idat_chunk(data, chunk_header.size_of_data, &result);
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


Bitmap load_png_file(const char* filename) {
    Bitmap result {};

#if 1
    byte_array contents = os::load_entire_file(filename);
    if (contents.is_empty()) {
        // @todo: handle error
        return result;
    }

    i32 x, y, n;
    u8 *pixels = stbi_load(filename, &x, &y, &n, 0);

    result.pixels = pixels;
    result.size = x * y * n;
    result.width = x;
    result.height = y;
    result.bytes_per_pixel = n;
#endif

    return result;
}

