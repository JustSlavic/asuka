#include <stdio.h>
#include <string.h>

#include <os.hpp>
#include <bitmap.hpp>
#include <png.hpp>


void print_usage()
{
    printf("./png_to_c INPUT                         \n"
           "                                         \n"
           "    -h --help    Prints this message.    \n"
           "                                         \n"
           );
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("You have to specify name of png file and output file\n");
        return 1;
    }

    if (argc > 2)
    {
        printf("Too many arguments!\n");
        return 1;
    }

    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
    {
        print_usage();
        return 0;
    }

    char *filename = argv[1];
    usize filename_length = strlen(filename);
    if (filename_length < 4)
    {
        printf("Filename have to contain '.png' at the end, but it's too short.\n");
        return 1;
    }

    char name[1024] = {};
    usize name_length = filename_length - 4;

    memcpy(name, filename, name_length);

    name[name_length] = '.';
    name[name_length + 1] = 'c';
    name[name_length + 2] = 'p';
    name[name_length + 3] = 'p';

    Bitmap png = load_png_file(argv[1]);
    ASSERT(png.pixels);

    FILE *output_file = fopen(name, "wb");

    fprintf(output_file, "GLOBAL_VARIABLE u32 png_pixels_%.*s_ [] = {", (int) name_length, name);

    u8 *bytes = (u8 *) png.pixels;
    for (usize idx = 0;
        idx < png.size;
        idx += 1)
    {
        u8 value = *(bytes + idx);
        fprintf(output_file, "%u,", value);
    }

    fprintf(output_file,
        "};\n"
        "\n"
        "Bitmap get_%.*s_png()\n"
        "{\n"
        "    Bitmap result {};\n"
        "    result.pixels = png_pixels_%.*s_;\n"
        "    result.size = %llu;\n"
        "    result.width = %u;\n"
        "    result.height = %u;\n"
        "    result.bytes_per_pixel = %u;\n"
        "    \n"
        "    return result;\n"
        "}\n",
            (int) name_length, (char *) name,
            (int) name_length, (char *) name,
            png.size, png.width, png.height, png.bytes_per_pixel
        );

    fclose(output_file);
}
