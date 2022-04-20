#ifndef ASUKA_COMMON_MATH_COLOR_HPP
#define ASUKA_COMMON_MATH_COLOR_HPP

#include <defines.hpp>
#include "float.hpp"


struct color24 {
    struct { f32 r; f32 g; f32 b; };

    static color24 white;
    static color24 red;
};


struct color32 {
    union {
        struct { f32 r, g, b, a; };
        struct { color24 rgb; f32 pad_; };
        f32 array_[4];
    };

    static color32 white;
    static color32 black;

    static color32 red;
    static color32 green;
    static color32 blue;
};

typedef color32 Color32;


inline u32 pack_to_uint32(color24 color) {
    u32 result =
        (math::round_to_u32(color.r * 255.f) << 16) |
        (math::round_to_u32(color.g * 255.f) << 8) |
        (math::round_to_u32(color.b * 255.f));

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


INLINE
color24 rgb(f32 r, f32 g, f32 b) {
    color24 result{ r, g, b };
    return result;
}

INLINE
color32 rgba(f32 r, f32 g, f32 b, f32 a) {
    color32 result{ r, g, b, a };
    return result;
}


#if UNITY_BUILD
#include "color.cpp"
#endif

#endif // ASUKA_COMMON_MATH_COLOR_HPP
