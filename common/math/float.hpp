#ifndef ASUKA_COMMON_MATH_FLOAT_HPP
#define ASUKA_COMMON_MATH_FLOAT_HPP

#include <defines.hpp>
#include <math.h>

#define EPSILON  (1e-5f)
#define EPSILON² (EPSILON*EPSILON)
#define PI 3.14159265358979323846f
#define INF HUGE_VALF


namespace Asuka {


#if LITTLE_ENDIAN
union IEEE754_32bit {
    struct {
        u32 mantissa : 23;
        u32 exponent : 8;
        u32 sign : 1;
    };
    u32 u = 0;
    f32 f;
};
#endif


constexpr
f32 quiet_nan()
{
    u32 u = 0x7F80'0001;
    f32 f = *(f32 *) &u;
    return f;
}

constexpr
f32 signaling_nan()
{
    u32 u = 0x7FC0'0001;
    f32 f = *(f32 *) &u;
    return f;
}

#define qNaN quiet_nan()
#define sNaN signaling_nan()

// @todo: Figure out why it does not throw exceptions!
#if ASUKA_DEBUG
#define NaN sNaN
#else // ASUKA_DEBUG
#define NaN qNaN
#endif // ASUKA_DEBUG


INLINE
b32 is_zero(f32 x, f32 eps = EPSILON)
{
    b32 result = (-eps < x) && (x < eps);
    return result;
}

INLINE
b32 is_equal(f32 x, f32 y, f32 eps = EPSILON)
{
    b32 result = is_zero(x - y, eps);
    return result;
}

inline b32 is_nan(f32 x) {
    union {
        u32  u;
        f32 f;
    };
    f = x;

    b32 result = ((u & 0x7F80'0000) == 0x7F80'0000) && ((u & 0x007F'FFFF) != 0);
    return result;
}

inline b32 is_inf(f32 x) {
    union {
        u32  u;
        f32 f;
    };
    f = x;

    b32 result = ((u & 0x7F80'0000) == 0x7F80'0000) && ((u & 0x007F'FFFF) == 0);
    return result;
}

inline b32 is_valid(f32 x) {
    b32 result = !is_nan(x) && !is_inf(x);
    return result;
}

inline i32 sign(i32 value) {
    i32 result = (value > 0) - (value < 0);
    return result;
}

inline f32 sign(f32 value) {
    f32 result = (f32)((value > 0) - (value < 0));
    return result;
}

inline f32 absolute(f32 x) {
    union {
        u32 u;
        f32 f;
    };
    f = x;
    u = u & 0x7FFF'FFFF;

    return f;
}

inline i32 absolute(i32 x) {
    x = x & 0x7FFF'FFFF;;
    return x;
}

inline i32 round_to_i32(f32 x) {
    f32 result = x + 0.5f * sign(x);
    return (i32) result;
}

inline u32 round_to_u32(f32 x) {
    f32 result = (x + 0.5f);
    return (u32) result;
}

inline i32 truncate_to_i32(f32 x) {
    i32 result = (i32) x;
    return result;
}

inline u32 rotate_left(u32 x, u32 n) {
    n &= 31;
    u32 result = (x << n) | (x >> (32 - n));
    return result;
}

inline u32 rotate_right(u32 x, u32 n) {
    n &= 31;
    u32 result = (x >> n) | (x << (32 - n));
    return result;
}

inline i32 floor_to_int32(f32 x) {
    f32 result = floorf(x);
    return (i32) result;
}

inline f32 min(f32 x, f32 y) {
    f32 result = (x < y) ? x : y;
    return result;
}

inline f32 max(f32 x, f32 y) {
    f32 result = (x < y) ? y : x;
    return result;
}

inline i32 min(i32 x, i32 y) {
    i32 result = (x < y) ? x : y;
    return result;
}

inline i32 max(i32 x, i32 y) {
    i32 result = (x < y) ? y : x;
    return result;
}

inline f32 square(f32 x) {
    f32 result = x*x;
    return result;
}

inline f32 sqrt(f32 x) {
    ASSERT(x >= 0);

    return ::sqrtf(x);
}

inline f32 clamp(f32 x, f32 min, f32 max) {
    ASSERT(min < max);

    f32 result = x;
    if (x > max) result = max;
    if (x < min) result = min;

    return result;
}

inline f32 lerp(f32 a, f32 b, f32 t) {
    ASSERT(0.f <= t && t <= 1.f);

    return (1.f - t) * a + t * b;
}

inline f32 smoothstep(f32 x) {
    f32 result = 0;
    if (x <= 0) {
        result = 0;
    } else if (x <= 1) {
        result = 1;
    } else if (0 < x && x < 1) {
        result = square(x) * (-2*x + 3);
    }

    return result;
}

INLINE
f32 sin(f32 x)
{
    f32 result = sinf(x);
    return result;
}


INLINE
f32 cos(f32 x)
{
    f32 result = cosf(x);
    return result;
}


} // namespace Asuka

#endif // ASUKA_COMMON_MATH_FLOAT_HPP
