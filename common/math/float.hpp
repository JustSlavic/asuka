#ifndef ASUKA_COMMON_MATH_FLOAT_HPP
#define ASUKA_COMMON_MATH_FLOAT_HPP

#include <defines.hpp>
#include <math.h>


namespace math {

extern f32 pi;
extern f32 NaN;

typedef union {
    struct {
        u32 mantissa : 23;
        u32 exponent : 8;
        u32 sign : 1;
    };
    u32 u;
    f32 f;
} IEEE754_f32_repr;

inline bool32 is_nan(float32 x) {
    union {
        uint32  u;
        float32 f;
    };
    f = x;

    bool32 result = ((u & 0x7F80'0000) == 0x7F80'0000) && ((u & 0x007F'FFFF) != 0);
    return result;
}

inline bool32 is_inf(float32 x) {
    union {
        uint32  u;
        float32 f;
    };
    f = x;

    bool32 result = ((u & 0x7F80'0000) == 0x7F80'0000) && ((u & 0x007F'FFFF) == 0);
    return result;
}

inline bool32 is_valid(float32 x) {
    bool32 result = !is_nan(x) && !is_inf(x);
    return result;
}

inline float32 absolute(float32 x) {
    union {
        uint32  u;
        float32 f;
    };
    f = x;
    u = u & 0x7FFF'FFFF;

    return f;
}

inline int32 round_to_i32(float32 x) {
    float32 result = x + 0.5f;
    return (int32) result;
}

inline uint32 round_to_u32(float32 x) {
    float32 result = (x + 0.5f);
    return (uint32) result;
}

inline int32 truncate_to_int32(float32 x) {
    int32 result = (int32) x;
    return result;
}

inline int32 floor_to_int32(float32 x) {
    float32 result = floorf(x);
    return (int32) result;
}

inline float32 min(float32 x, float32 y) {
    float32 result = (x < y) ? x : y;
    return result;
}

inline float32 max(float32 x, float32 y) {
    float32 result = (x < y) ? y : x;
    return result;
}

inline int32 min(int32 x, int32 y) {
    int32 result = (x < y) ? x : y;
    return result;
}

inline int32 max(int32 x, int32 y) {
    int32 result = (x < y) ? y : x;
    return result;
}

inline float32 square(float32 x) {
    float32 result = x*x;
    return result;
}

inline float32 sqrt(float32 x) {
    ASSERT(x >= 0);

    return ::sqrtf(x);
}

inline float32 clamp(f32 x, f32 min, f32 max) {
    ASSERT(min < max);

    float32 result = x;
    if (x > max) result = max;
    if (x < min) result = min;

    return result;
}

inline f32 lerp(f32 a, f32 b, f32 t) {
    ASSERT(0.f <= t && t <= 1.f);

    return (1.f - t) * a + t * b;
}

inline int32 sign(int32 value) {
    int32 result = (value > 0) - (value < 0);
    return result;
}

inline int32 sign(float32 value) {
    int32 result = (value > 0) - (value < 0);
    return result;
}

} // namespace math

#ifdef UNITY_BUILD
#include "float.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_MATH_FLOAT_HPP
