#pragma once

#include <string.hpp>
#include <allocator.hpp>

#include <stdio.h>


int32 tprint(char const *fmt)
{
    osOutputDebugString("%s", fmt);
    return (int32) cstring::size_no0(fmt);
}

template <typename T>
void tprint_helper(T x);

template <>
void tprint_helper<int8>(int8 x)
{
    osOutputDebugString("%hhd", x);
}

template <>
void tprint_helper<int16>(int16 x)
{
    osOutputDebugString("%hd", x);
}

template <>
void tprint_helper<int32>(int32 x)
{
    osOutputDebugString("%d", x);
}

template <>
void tprint_helper<int64>(int64 x)
{
    osOutputDebugString("%lld", x);
}

template <>
void tprint_helper<uint8>(uint8 x)
{
    osOutputDebugString("%hhu", x);
}

template <>
void tprint_helper<uint16>(uint16 x)
{
    osOutputDebugString("%hu", x);
}

template <>
void tprint_helper<uint32>(uint32 x)
{
    osOutputDebugString("%u", x);
}

template <>
void tprint_helper<uint64>(uint64 x)
{
    osOutputDebugString("%llu", x);
}

template <>
void tprint_helper<float32>(float32 x)
{
    osOutputDebugString("%f", x);
}

template <>
void tprint_helper<float64>(float64 x)
{
    osOutputDebugString("%lf", x);
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
                osOutputDebugString("{");
                advance();
            }
            else
            {
                osOutputDebugString("<ERROR!>");
                return count;
            }
        }
        else
        {
            osOutputDebugString("%c", *fmt);
            advance();
        }
    }

    return count;
}
