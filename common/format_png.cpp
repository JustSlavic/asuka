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


struct bit_fetcher {
    uint8* buffer;
    uint32 bits;
    uint32 available_bits;
};


uint32 get_bits(bit_fetcher* fetcher, int32 n) {
    uint32 result;

    if (fetcher->available_bits < n) {
        do {
            fetcher->bits |= ((uint32)(*fetcher->buffer++)) << fetcher->available_bits;
            fetcher->available_bits += 8;
        } while (fetcher->available_bits <= 24);
    }

    result = fetcher->bits & ((1 << n) - 1);
    fetcher->bits = fetcher->bits >> n;
    fetcher->available_bits -= n;

    return result;
}


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

            ASSERT(header.compression_method == 0); // At present, only compression method 0 (deflate/inflate compression with a sliding window of at most 32768 bytes) is defined. All standard PNG images must be compressed with this scheme.
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
            uint8 CMF = *data;
            uint8 compression_method = 0x0F & CMF;
            ASSERT(compression_method == 8); // CM=8 denotes 'deflate' algorithm.

            uint8 compression_info   = (0xF0 & CMF) >> 4;
            // compression_info = (log_2(LZ77_window_size) - 8);
            // => uint32 LZ77_window_size = 1 << (compression_info + 8);
            // CINFO=7 indicates 32768 window size.
            ASSERT(compression_info < 8); // CINFO above 7 are not allowed by the specification.
            // compression_info is not defined if compression_method != 8

            uint32 LZ77_window_size = 1 << (compression_info + 8);

            uint8 FLG = *(data + 1);

            uint8 FCHECK = (0x1F & FLG);
            uint8 FDICT  = (0x20 & FLG) >> 5;
            uint8 FLEVEL = (0xC0 & FLG) >> 6;

            ASSERT((CMF * 256 + FLG) % 31 == 0); // The FCHECK value must be such that CMF and FLG, when viewed as
                                                 // a 16-bit unsigned integer stored in MSB order (CMF*256 + FLG),
                                                 // is a multiple of 31.

            ASSERT(FDICT == 0);

            uint8* zlib_data = data + 2;

            bit_fetcher fetcher;
            fetcher.buffer = zlib_data;
            fetcher.bits = 0;
            fetcher.available_bits = 0;

            uint8 BFINAL;
            uint8 BTYPE;
            do {
                BFINAL = get_bits(&fetcher, 1); // ((*zlib_data) & 0b1000'0000) >> 7;
                BTYPE  = get_bits(&fetcher, 2); // ((*zlib_data) & 0b0110'0000) >> 5;

                ASSERT(BTYPE != 3); // @debug: reserved value for error!

                if (BTYPE == 0) { // Stored with no compression.
                    zlib_data += 1; // Skip remaining bits in currently processed byte.

                    uint16 LEN = *(uint16*)zlib_data;
                    zlib_data += 2;

                    uint16 NLEN = *(uint16*)zlib_data;
                    zlib_data += 2;

                    ASSERT(LEN == ~NLEN);
                    // Test this on something.
                } else {
                    if (BTYPE == 3) {
                        // @todo: return error;
                        return nullptr;
                    }

                    if (BTYPE == 1) {
                        // Compressed with fixed Huffman codes
                    }

                    if (BTYPE == 2) {
                        // Compressed with dynamic Huffman codes
                        // Computing Huffman codes
                        uint32 HLIT  = get_bits(&fetcher, 5) + 257;
                        uint32 HDIST = get_bits(&fetcher, 5) + 1;
                        uint32 HCLEN = get_bits(&fetcher, 4) + 4;

                        uint8 code_lengths_order[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
                        uint8 code_lengths[19] {};

                        for (int i = 0; i < HCLEN; i++) {
                            code_lengths[code_lengths_order[i]] = get_bits(&fetcher, 3);
                        }

                        // DEFLATE algorithm to generate codes
                        // 1) Count the number of codes for each code length.
                        //    Let bl_count[N] be the number of codes of length N, N >= 1.

                        uint32 bl_count[19] {};

                        for (int i = 0; i < 19; i++) {
                            bl_count[code_lengths[i]]++;
                        }

                        // 2) Find the numerical value of the smallest code for each code length:

                        int32 code = 0;
                        bl_count[0] = 0;
                        uint32 max_code;
                        uint32 next_code[16] {};

                        for (uint32 bits = 1; bits < 16; bits++) {
                            code = (code + bl_count[bits-1]) << 1;
                            next_code[bits] = code;
                        }

                        // 3) Assign numerical values to all codes, using consecutive
                        //    values for all codes of the same length with the base
                        //    values determined at step 2. Codes that are never used
                        //    (which have a bit length of zero) must not be assigned a
                        //    value.

                        // for (uint32 n = 0;  n <= max_code; n++) {
                        //     len = tree[n].Len;
                        //     if (len != 0) {
                        //         tree[n].Code = next_code[len];
                        //         next_code[len]++;
                        //     }
                        // }

                        // Apply Huffman codes
                    }

                    // Decode here
                }
            } while (!BFINAL);

            uint8 debug_ = 0;

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
