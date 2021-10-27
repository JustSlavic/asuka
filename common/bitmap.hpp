#ifndef ASUKA_COMMON_BITMAP_HPP
#define ASUKA_COMMON_BITMAP_HPP


struct bitmap {
    void*  pixels;
    usize  size;   // in bytes
    uint32 width;  // in pixels
    uint32 height; // in pixels
    uint32 bytes_per_pixel;
};


#endif // ASUKA_COMMON_BITMAP_HPP
