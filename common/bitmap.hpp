#ifndef ASUKA_COMMON_BITMAP_HPP
#define ASUKA_COMMON_BITMAP_HPP


struct Bitmap {
    void* pixels;
    usize size; // in bytes
    u32 width;  // in pixels
    u32 height; // in pixels
    u32 bytes_per_pixel;
};


#endif // ASUKA_COMMON_BITMAP_HPP
