#pragma once

#include <stdio.h>


namespace console
{

enum emphasis : uint8
{
    BOLD = 1,
    FAINT = 2,
    ITALIC = 3,
    UNDERLINE = 4,
    BLINKING = 5,
    INVERSE = 7,
    HIDDEN = 8,
    STRIKETHROUGH = 9,
};



GLOBAL char const ESC = 0x1b;

void reset_formatting()
{
    int32 index = 0;
    char buffer[5];

    buffer[index++] = ESC;
    buffer[index++] = '[';
    buffer[index++] = '0';
    buffer[index++] = 'm';
    buffer[index++] = 0;

    printf("%s", buffer);
}

void set_emphasis(emphasis emph)
{
    int32 index = 0;
    char buffer[5];

    buffer[index++] = ESC;
    buffer[index++] = '[';
    buffer[index++] = '0' + emph;
    buffer[index++] = 'm';
    buffer[index++] = 0;

    printf("%s", buffer);
}

void unset_emphasis(emphasis emph)
{
    int32 index = 0;
    char buffer[6];

    buffer[index++] = ESC;
    buffer[index++] = '[';
    buffer[index++] = '2';
    buffer[index++] = '0' + emph;
    buffer[index++] = 'm';
    buffer[index++] = 0;

    printf("%s", buffer);
}

void set_fg(uint8 color_index)
{
    int32 index = 0;
    char buffer[12];

    buffer[index++] = ESC;
    buffer[index++] = '[';
    buffer[index++] = '3';
    buffer[index++] = '8';
    buffer[index++] = ';';
    buffer[index++] = '5';
    buffer[index++] = ';';
    if (color_index > 99)
    {
        buffer[index++] = '0' + (char) (color_index / 100);
        color_index %= 100;
        buffer[index++] = '0' + (char) (color_index / 10);
        color_index %= 10;
        buffer[index++] = '0' + (char) color_index;
    }
    else if (color_index > 9)
    {
        buffer[index++] = '0' + (char) (color_index / 10);
        color_index %= 10;
        buffer[index++] = '0' + (char) color_index;
    }
    else
    {
        buffer[index++] = '0' + (char) color_index;
    }
    buffer[index++] = 'm';
    buffer[index++] = 0;

    printf("%s", buffer);
}

void set_bg(uint8 color_index)
{
    int32 index = 0;
    char buffer[12];

    buffer[index++] = ESC;
    buffer[index++] = '[';
    buffer[index++] = '4';
    buffer[index++] = '8';
    buffer[index++] = ';';
    buffer[index++] = '5';
    buffer[index++] = ';';
    if (color_index > 99)
    {
        buffer[index++] = '0' + (char) (color_index / 100);
        color_index %= 100;
        buffer[index++] = '0' + (char) (color_index / 10);
        color_index %= 10;
        buffer[index++] = '0' + (char) color_index;
    }
    else if (color_index > 9)
    {
        buffer[index++] = '0' + (char) (color_index / 10);
        color_index %= 10;
        buffer[index++] = '0' + (char) color_index;
    }
    else
    {
        buffer[index++] = '0' + (char) color_index;
    }
    buffer[index++] = 'm';
    buffer[index++] = 0;

    printf("%s", buffer);
}

void set_bg(int32 r, int32 g, int32 b)
{
    int32 index = 0;
    char buffer[20];

    buffer[index++] = ESC;
    buffer[index++] = '[';
    buffer[index++] = '4'; // @todo: make background and foreground colors together, because they differs only in here
    buffer[index++] = '8';
    buffer[index++] = ';';
    buffer[index++] = '2';
    buffer[index++] = ';';

    if (r > 99)
    {
        buffer[index++] = '0' + (char) (r / 100);
        r %= 100;
        buffer[index++] = '0' + (char) (r / 10);
        r %= 10;
        buffer[index++] = '0' + (char) r;
    }
    else if (r > 9)
    {
        buffer[index++] = '0' + (char) (r / 10);
        r %= 10;
        buffer[index++] = '0' + (char) r;
    }
    else
    {
        buffer[index++] = '0' + (char) r;
    }
    buffer[index++] = ';';
    if (g > 99)
    {
        buffer[index++] = '0' + (char) (g / 100);
        g %= 100;
        buffer[index++] = '0' + (char) (g / 10);
        g %= 10;
        buffer[index++] = '0' + (char) g;
    }
    else if (g > 9)
    {
        buffer[index++] = '0' + (char) (g / 10);
        g %= 10;
        buffer[index++] = '0' + (char) g;
    }
    else
    {
        buffer[index++] = '0' + (char) g;
    }
    buffer[index++] = ';';
    if (b > 99)
    {
        buffer[index++] = '0' + (char) (b / 100);
        b %= 100;
        buffer[index++] = '0' + (char) (b / 10);
        b %= 10;
        buffer[index++] = '0' + (char) b;
    }
    else if (b > 9)
    {
        buffer[index++] = '0' + (char) (b / 10);
        b %= 10;
        buffer[index++] = '0' + (char) b;
    }
    else
    {
        buffer[index++] = '0' + (char) b;
    }
    buffer[index++] = 'm';
    buffer[index++] = 0;

    printf("%s", buffer);
}

void set_fg(int32 r, int32 g, int32 b)
{
    int32 index = 0;
    char buffer[20];

    buffer[index++] = ESC;
    buffer[index++] = '[';
    buffer[index++] = '3';
    buffer[index++] = '8';
    buffer[index++] = ';';
    buffer[index++] = '2';
    buffer[index++] = ';';

    if (r > 99)
    {
        buffer[index++] = '0' + (char) (r / 100);
        r %= 100;
        buffer[index++] = '0' + (char) (r / 10);
        r %= 10;
        buffer[index++] = '0' + (char) r;
    }
    else if (r > 9)
    {
        buffer[index++] = '0' + (char) (r / 10);
        r %= 10;
        buffer[index++] = '0' + (char) r;
    }
    else
    {
        buffer[index++] = '0' + (char) r;
    }
    buffer[index++] = ';';
    if (g > 99)
    {
        buffer[index++] = '0' + (char) (g / 100);
        g %= 100;
        buffer[index++] = '0' + (char) (g / 10);
        g %= 10;
        buffer[index++] = '0' + (char) g;
    }
    else if (g > 9)
    {
        buffer[index++] = '0' + (char) (g / 10);
        g %= 10;
        buffer[index++] = '0' + (char) g;
    }
    else
    {
        buffer[index++] = '0' + (char) g;
    }
    buffer[index++] = ';';
    if (b > 99)
    {
        buffer[index++] = '0' + (char) (b / 100);
        b %= 100;
        buffer[index++] = '0' + (char) (b / 10);
        b %= 10;
        buffer[index++] = '0' + (char) b;
    }
    else if (b > 9)
    {
        buffer[index++] = '0' + (char) (b / 10);
        b %= 10;
        buffer[index++] = '0' + (char) b;
    }
    else
    {
        buffer[index++] = '0' + (char) b;
    }
    buffer[index++] = 'm';
    buffer[index++] = 0;

    printf("%s", buffer);
}

void output_color_codes()
{
    for (int i = 0; i <= UINT8_MAX; i++)
    {
        if (i == 8 || i == 16 || ((i > 16) && (i + 2) % (6) == 0)) printf("\n");
        console::set_fg((uint8) i);
        if (i < 10) printf("00");
        else if (i < 100) printf("0");
        printf("%d ", i);
    }
    printf("\n");
    console::reset_formatting();
}

} // namespace console

int32 tprint(char const *fmt)
{
    printf("%s", fmt);
    return (int32) cstring::size_no0(fmt);
}

template <typename T>
void tprint_helper(T x);

template <>
void tprint_helper<int8>(int8 x)
{
    printf("%hhd", x);
}

template <>
void tprint_helper<int16>(int16 x)
{
    printf("%hd", x);
}

template <>
void tprint_helper<int32>(int32 x)
{
    printf("%d", x);
}

template <>
void tprint_helper<int64>(int64 x)
{
    printf("%lld", x);
}

template <>
void tprint_helper<uint8>(uint8 x)
{
    printf("%hhu", x);
}

template <>
void tprint_helper<uint16>(uint16 x)
{
    printf("%hu", x);
}

template <>
void tprint_helper<uint32>(uint32 x)
{
    printf("%u", x);
}

template <>
void tprint_helper<uint64>(uint64 x)
{
    printf("%llu", x);
}

template <>
void tprint_helper<float32>(float32 x)
{
    printf("%f", x);
}

template <>
void tprint_helper<float64>(float64 x)
{
    printf("%lf", x);
}

template <typename T, typename... Args>
int32 tprint(char const *fmt, T x, Args... args)
{
    int32 count = 0;

    auto advance = [&count, &fmt] (int32 n = 1) { fmt += n; count += n; };
    while (*fmt)
    {
        if (*fmt == '{')
        {
            advance();
            if (*fmt == '}')
            {
                advance();
                tprint_helper(x);

                int32 n = tprint(fmt, args...);
                advance(n);
            }
            else if (*fmt == '{')
            {
                printf("{");
                advance();
            }
            else
            {
                printf("<ERROR!>");
                return count;
            }
        }
        else
        {
            printf("%c", *fmt);
            advance();
        }
    }

    return count;
}

#define DEFINE_TPRINT_FUNC(T, ARG) template <> void tprint_helper<T>(T ARG)
