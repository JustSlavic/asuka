#include "png.hpp"
#include "os/file.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <external/stb_image.h>


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
