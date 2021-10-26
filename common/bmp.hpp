#ifndef ASUKA_COMMON_BMP_HPP
#define ASUKA_COMMON_BMP_HPP

#include <defines.hpp>


struct bmp_file_contents {
    void*  pixels;
    usize  size;   // in bytes
    uint32 width;  // in pixels
    uint32 height; // in pixels
    uint32 bytes_per_pixel;
};


bmp_file_contents load_bmp_file(const char* filename);


#ifdef UNITY_BUILD
#include "bmp.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_BMP_HPP
