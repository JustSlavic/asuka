#include "bmp.hpp"


#pragma pack(push, 1)

struct BMP_Header {
    uint16 signature;
    uint32 filesize;
    uint32 reserved; // always 0
    uint32 data_offset;
};

enum BMP_Compression : uint32 {
    BMP_BI_RGB = 0,  // No compression
    BMP_BI_RLE8 = 1, // 8bit RLE encoding
    BMP_BI_RLE4 = 2, // 4bit RLE encoding
};

struct BMP_InfoHeader {
    uint32 size; // of this info header (always 40)
    uint32 width;
    uint32 height;
    uint16 planes; // always 1
    uint16 bits_per_pixel;
    BMP_Compression compression;
    uint32 image_size; // Can be 0 if compression is BI_RGB
    uint32 x_pixels_per_meter; // ???
    uint32 y_pixels_per_meter; // ???
    uint32 color_used; // For 8-bit per pixel bitmap it will be equal to 256
    uint32 important_colors; // 0 - all colors are important
};

// Can be ignored since we will use only uncompressed BI_RGB images
struct BMP_ColorTable {
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 reserved; // always 0
};

#pragma pack(pop)


#define BMP_MAGIC_NUMBER(a, b) ((((uint32)a) << 0) | (((uint32)b) << 8))

enum {
    BMP_SIGNATURE = BMP_MAGIC_NUMBER('B', 'M'),
};


Bitmap load_bmp_file(const char* filename) {
    Bitmap result {};

    asuka::string contents = os::load_entire_file(filename);
    if (contents.data == NULL) {
        // @todo: handle error
        return result;
    }

    uint8 *data = (uint8 *) contents.data;

    BMP_Header *bmp_header = (BMP_Header *) data;
    ASSERT(bmp_header->signature == BMP_SIGNATURE);
    ASSERT(bmp_header->reserved == 0);

    BMP_InfoHeader *bmp_info_header = (BMP_InfoHeader *) ((uint8 *) data + sizeof(BMP_Header));
    ASSERT(bmp_info_header->size == 40);
    ASSERT(bmp_info_header->planes == 1);
    ASSERT(bmp_info_header->image_size > 0); // @todo: can be 0 but how can we guard against that?

    if (bmp_info_header->bits_per_pixel < 8) {
        BMP_ColorTable *bmp_color_table = (BMP_ColorTable *) ((uint8 *) data + sizeof(BMP_Header) + sizeof(BMP_InfoHeader));
    }

    uint8 *pixels = data + bmp_header->data_offset;

    // @todo: should we convert bitmap from 24 bpp to 32 bpp?
    // @todo: can we just create bmp files that contain 32 bpp bitmaps?

    int32 bytes_per_pixel = bmp_info_header->bits_per_pixel / 8;

    // flip bitmap
    for (uint32 row = 0; row < bmp_info_header->height / 2; row++) {
        uint8 *top_row = pixels + row * bmp_info_header->width * bytes_per_pixel;
        uint8 *bottom_row = pixels + (bmp_info_header->height - row - 1) * bmp_info_header->width * bytes_per_pixel;

        for (uint32 column = 0; column < bmp_info_header->width * bytes_per_pixel; column++) {
            uint8 tmp = top_row[column];
            top_row[column] = bottom_row[column];
            bottom_row[column] = tmp;
        }
    }

    uint8 *pixel = pixels;
    for (uint32 pixel_idx = 0; pixel_idx < bmp_info_header->width * bmp_info_header->height; pixel_idx++) {
        uint8 r = pixel[0];
        uint8 b = pixel[2];

        pixel[2] = r;
        pixel[0] = b;

        pixel += (bmp_info_header->bits_per_pixel / 8);
    }

    result.pixels = pixels;
    result.size = bmp_info_header->image_size;
    result.width = bmp_info_header->width;
    result.height = bmp_info_header->height;
    result.bytes_per_pixel = bmp_info_header->bits_per_pixel / 8;

    return result;
}
