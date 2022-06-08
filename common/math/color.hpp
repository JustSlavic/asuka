#ifndef ASUKA_COMMON_MATH_COLOR_HPP
#define ASUKA_COMMON_MATH_COLOR_HPP

#include <defines.hpp>
#include "float.hpp"


namespace Asuka {


struct Color24 {
    struct { f32 r; f32 g; f32 b; };

    STATIC Color24 White;
    STATIC Color24 Black;

    STATIC Color24 Red;
    STATIC Color24 Green;
    STATIC Color24 Blue;
};


struct Color32 {
    union {
        struct { f32 r, g, b, a; };
        struct { Color24 rgb; f32 pad_; };
        f32 array_[4];
    };

    STATIC Color32 White;
    STATIC Color32 Black;

    STATIC Color32 Red;
    STATIC Color32 Green;
    STATIC Color32 Blue;
};


inline u32 pack_to_uint32(Color24 color) {
    u32 result =
        (round_to_u32(color.r * 255.f) << 16) |
        (round_to_u32(color.g * 255.f) << 8) |
        (round_to_u32(color.b * 255.f));

    return result;
}

inline bool operator == (Color24 lhs, Color24 rhs) {
    bool result = (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b);
    return result;
}

inline bool operator != (Color24 lhs, Color24 rhs) {
    bool result = !(lhs == rhs);
    return result;
}


template <typename R, typename G, typename B>
Color24 color24(R r, G g, B b)
{
    Color24 result = { f32(r), f32(g), f32(b) };
    return result;
}


template <typename R, typename G, typename B, typename A = f32>
Color32 color32(R r, G g, B b, A a = 0)
{
    Color32 result = { f32(r), f32(g), f32(b), f32(a) };
    return result;
}


INLINE
Color24 rgb(f32 r, f32 g, f32 b) {
    Color24 result{ r, g, b };
    return result;
}

INLINE
Color32 rgba(f32 r, f32 g, f32 b, f32 a) {
    Color32 result{ r, g, b, a };
    return result;
}

} // namespace Asuka


#if UNITY_BUILD
#include "color.cpp"
#endif

#endif // ASUKA_COMMON_MATH_COLOR_HPP
