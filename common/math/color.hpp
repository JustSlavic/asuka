#pragma once

#include <defines.hpp>
#include "float.hpp"


struct color24 {
    struct { f32 r; f32 g; f32 b; };

    STATIC color24 White;
    STATIC color24 Black;

    STATIC color24 Red;
    STATIC color24 Green;
    STATIC color24 Blue;
};


struct color32 {
    union {
        struct { f32 r, g, b, a; };
        struct { color24 rgb; f32 pad_; };
        f32 array_[4];
    };

    STATIC color32 White;
    STATIC color32 Black;

    STATIC color32 Red;
    STATIC color32 Green;
    STATIC color32 Blue;
};


inline u32 pack_to_uint32(color24 color) {
    u32 result =
        (round_to_u32(color.r * 255.f) << 16) |
        (round_to_u32(color.g * 255.f) << 8) |
        (round_to_u32(color.b * 255.f));

    return result;
}

inline bool operator == (color24 lhs, color24 rhs) {
    bool result = (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b);
    return result;
}

inline bool operator != (color24 lhs, color24 rhs) {
    bool result = !(lhs == rhs);
    return result;
}


template <typename R, typename G, typename B>
color24 make_color24(R r, G g, B b)
{
    color24 result{ float32(r), float32(g), float32(b) };
    return result;
}


template <typename R, typename G, typename B>
color24 make_rgb(R r, G g, B b)
{
    return make_color24<R, G, B>(r, g, b);
}


template <typename R, typename G, typename B, typename A>
color32 make_color32(R r, G g, B b, A a)
{
    color32 result{ float32(r), float32(g), float32(b), float32(a) };
    return result;
}


template <typename R, typename G, typename B, typename A>
color32 make_rgba(R r, G g, B b, A a)
{
    return make_color32<R, G, B, A>(r, g, b, a);
}


#if UNITY_BUILD
#include "color.cpp"
#endif
