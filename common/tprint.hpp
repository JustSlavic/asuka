#pragma once

#include <string.hpp>
#include <allocator.hpp>

#include <stdio.h>


GLOBAL memory::arena_allocator temporary_storage;


int32 tprint(char const *fmt)
{
    return printf("%s", fmt);
}

template <typename T>
int32 tprint_helper(char const *fmt, T x);

template <>
int32 tprint_helper<int>(char const *fmt, int x)
{
    return printf("%d", x);
}

template <>
int32 tprint_helper<float>(char const *fmt, float x)
{
    return printf("%5.2f", x);
}

template <typename T, typename... Args>
int32 tprint(char const *fmt, T x, Args... args)
{
    int32 count = 0;
    while (*fmt)
    {
        if (*fmt == '{')
        {
            fmt++;
            if (*fmt == '}')
            {
                fmt++;
                count += tprint_helper(fmt, x);

                int32 n = tprint(fmt, args...);
                count += n;
                fmt += n;
            }
            else if (*fmt == '{')
            {
                count += printf("{");
            }
            else
            {
                printf("<ERROR!>");
                return count;
            }
        }
        else
        {
            printf("%c", *fmt++);
            count += 1;
        }
    }

    return count;
}
