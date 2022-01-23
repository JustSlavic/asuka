#ifndef ASUKA_COMMON_MATH_COLOR_HPP
#define ASUKA_COMMON_MATH_COLOR_HPP

#include <defines.hpp>


namespace math {


struct color24 {
    struct { f32 r; f32 g; f32 b; };

    static color24 white;
};


struct color32 {
    union {
        struct { f32 r, g, b, a; };
        struct { color24 rgb; f32 pad_; };
        f32 components[4];
    };
};


inline uint32 pack_to_uint32(color24 color) {
    uint32 result =
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

inline color24 lerp (const color24& a, const color24& b, f32 t) {
    color24 result = color24{ lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t) };
    return result;
}

} // namespace math

#if UNITY_BUILD
#include "color.cpp"
#endif

#endif // ASUKA_COMMON_MATH_COLOR_HPP
