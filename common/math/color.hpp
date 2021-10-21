#ifndef ASUKA_COMMON_MATH_COLOR_HPP
#define ASUKA_COMMON_MATH_COLOR_HPP

#include <defines.hpp>


struct color24 {
    struct { f32 r; f32 g; f32 b; };
};


inline uint32 pack_to_uint32(color24 color) {
    uint32 result =
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


inline color24 lerp (const color24& a, const color24& b, f32 t) {
    color24 result = color24{ math::lerp(a.r, b.r, t), math::lerp(a.g, b.g, t), math::lerp(a.b, b.b, t) };
    return result;
}


#endif // ASUKA_COMMON_MATH_COLOR_HPP
