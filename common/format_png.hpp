#ifndef ASUKA_COMMON_FORMAT_PNG_HPP
#define ASUKA_COMMON_FORMAT_PNG_HPP


struct png_image {
    uint8* data;
    uint64 size;
    uint32 bytes_per_pixel;
    uint32 width;
    uint32 height;
    uint32 stride; // or pitch (Pitch = Width * BytesPerPixel + Stride)
};

png_image load_png_file(const char* filename);


#ifdef UNITY_BUILD
#include "format_png.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_FORMAT_PNG_HPP
