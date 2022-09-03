#ifndef ASUKA_COMMON_MATH_FLOAT_HPP
#define ASUKA_COMMON_MATH_FLOAT_HPP

#include <defines.hpp>
#include <math.h>

#define EPSILON  (1e-5f)
#define EPSILON2 (EPSILON*EPSILON)
#define PI 3.14159265358979323846f
#define INF HUGE_VALF


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

INLINE
b32 is_nan(f32 x) {
    union {
        u32 u;
        f32 f;
    };
    f = x;

    b32 result = ((u & 0x7F80'0000) == 0x7F80'0000) && ((u & 0x007F'FFFF) != 0);
    return result;
}

INLINE
b32 is_inf(f32 x) {
    union {
        u32 u;
        f32 f;
    };
    f = x;

    b32 result = ((u & 0x7F80'0000) == 0x7F80'0000) && ((u & 0x007F'FFFF) == 0);
    return result;
}

INLINE
b32 is_valid(f32 x)
{
    b32 result = !is_nan(x) && !is_inf(x);
    return result;
}

INLINE
f32 absolute(f32 x)
{
    union
    {
        u32 u;
        f32 f;
    };
    f = x;
    u = u & 0x7FFF'FFFF;

    return f;
}

INLINE
i32 absolute(i32 x)
{
    x = x & 0x7FFF'FFFF;;
    return x;
}

INLINE
i32 sign(i32 value)
{
    i32 result = (value > 0) - (value < 0);
    return result;
}

INLINE
f32 sign(f32 value)
{
    f32 result = (f32)((value > 0) - (value < 0));
    return result;
}

INLINE
i32 round_to_i32(f32 x)
{
    f32 result = x + 0.5f * sign(x);
    return (i32) result;
}

INLINE
u32 round_to_u32(f32 x)
{
    f32 result = (x + 0.5f);
    return (u32) result;
}

INLINE
i32 truncate_to_i32(f32 x)
{
    i32 result = (i32) x;
    return result;
}

INLINE
i32 floor_to_int32(f32 x)
{
    f32 result = floorf(x);
    return (i32) result;
}

INLINE
f32 square(f32 x)
{
    f32 result = x*x;
    return result;
}

INLINE
f32 square_root(f32 x)
{
    ASSERT(x >= 0);

    return sqrtf(x);
}

INLINE
f32 to_radians(f32 x)
{
    f32 radians = x * PI / 180.0f;
    return radians;
}

INLINE
f32 to_degrees(f32 x)
{
    f32 degrees = x * 180.0f / PI;
    return degrees;
}

INLINE
f32 clamp(f32 x, f32 min, f32 max)
{
    ASSERT(min < max);

    f32 result = x;
    if (x > max) result = max;
    if (x < min) result = min;

    return result;
}

INLINE
f32 lerp(f32 a, f32 b, f32 t)
{
    ASSERT(0.f <= t && t <= 1.f);

    return (1.f - t) * a + t * b;
}

INLINE
f32 smoothstep(f32 x)
{
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


namespace math
{


#if BYTE_ORDER == LITTLE_ENDIAN
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
u32 rotate_left(u32 x, u32 n)
{
    n &= 31;
    u32 result = (x << n) | (x >> (32 - n));
    return result;
}

INLINE
u32 rotate_right(u32 x, u32 n)
{
    n &= 31;
    u32 result = (x >> n) | (x << (32 - n));
    return result;
}

INLINE
f32 min_(f32 x, f32 y)
{
    f32 result = (x < y) ? x : y;
    return result;
}

INLINE
f32 max_(f32 x, f32 y)
{
    f32 result = (x < y) ? y : x;
    return result;
}

INLINE
i32 min_(i32 x, i32 y)
{
    i32 result = (x < y) ? x : y;
    return result;
}

INLINE
i32 max_(i32 x, i32 y)
{
    i32 result = (x < y) ? y : x;
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


} // namespace math

#endif // ASUKA_COMMON_MATH_FLOAT_HPP
