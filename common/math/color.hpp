#ifndef ASUKA_COMMON_MATH_COLOR_HPP
#define ASUKA_COMMON_MATH_COLOR_HPP

#include <defines.hpp>


namespace math {


struct color24 {
    struct { f32 r; f32 g; f32 b; };
};


inline uint32 pack_to_uint32(color24 color) {
    return (round_to_u32(color.r * 255.f) << 16) |
           (round_to_u32(color.g * 255.f) << 8) |
           (round_to_u32(color.b * 255.f));
}


inline bool operator == (color24 lhs, color24 rhs) {
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b);
}


inline bool operator != (color24 lhs, color24 rhs) {
    return !(lhs == rhs);
}


inline color24 lerp (const color24& a, const color24& b, f32 t) {
    return { lerp(a.r, b.r, t), lerp(a.g, b.g, t), lerp(a.b, b.b, t) };
}



} // math

#endif // ASUKA_COMMON_MATH_COLOR_HPP
