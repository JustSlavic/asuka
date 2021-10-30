#ifndef ASUKA_COMMON_MATH_FLOAT_HPP
#define ASUKA_COMMON_MATH_FLOAT_HPP

#include <defines.hpp>
#include <math.h>


namespace math {

extern f32 pi;

inline float32 abs(float32 x) {
    union {
        uint32  u;
        float32 f;
    };
    f = x;
    u = u & 0x7FFF'FFFF;

    return f;
}

inline int32 round_to_i32(float32 x) {
    return (int32) (x + 0.5f);
}

inline uint32 round_to_u32(float32 x) {
    return (int32) (x + 0.5f);
}

inline int32 truncate_to_int32(float32 x) {
    return (int32) x;
}

inline int32 floor_to_int32(float32 x) {
    return (int32) floorf(x);
}

inline float32 sqrt(float32 x) {
    ASSERT(x >= 0);

    return ::sqrtf(x);
}

inline f32 lerp (f32 a, f32 b, f32 t) {
    ASSERT(0.f <= t && t <= 1.f);

    return (1.f - t) * a + t * b;
}

} // namespace math

#ifdef UNITY_BUILD
#include "float.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_MATH_FLOAT_HPP
