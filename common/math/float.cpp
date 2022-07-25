#include "float.hpp"
#include <math.h>


f32 infinity()
{
    return HUGE_VALF;
}

f32 pi()
{
    return M_PI;
}

f32 nan()
{
    return NAN;
}

b32 is_zero(f32 x, f32 eps)
{
    b32 result = (-eps < x) && (x < eps);
    return result;
}

b32 is_equal(f32 x, f32 y, f32 eps)
{
    b32 result = is_zero(x - y, eps);
    return result;
}

b32 is_nan(f32 x)
{
    union
    {
        u32 u;
        f32 f;
    };
    f = x;

    b32 result = ((u & 0x7F80'0000) == 0x7F80'0000) && ((u & 0x007F'FFFF) != 0);
    return result;
}

b32 is_inf(f32 x)
{
    union
    {
        u32 u;
        f32 f;
    };
    f = x;

    b32 result = ((u & 0x7F80'0000) == 0x7F80'0000) && ((u & 0x007F'FFFF) == 0);
    return result;
}

b32 is_valid(f32 x)
{
    b32 result = !is_nan(x) && !is_inf(x);
    return result;
}

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

i32 absolute(i32 x)
{
    x = x & 0x7FFF'FFFF;;
    return x;
}

i32 sign(i32 value)
{
    i32 result = (value > 0) - (value < 0);
    return result;
}

f32 sign(f32 value)
{
    f32 result = (f32)((value > 0) - (value < 0));
    return result;
}

i32 round_to_i32(f32 x)
{
    f32 result = x + 0.5f * sign(x);
    return (i32) result;
}

u32 round_to_u32(f32 x)
{
    f32 result = (x + 0.5f);
    return (u32) result;
}

i32 truncate_to_i32(f32 x)
{
    i32 result = (i32) x;
    return result;
}

i32 floor_to_int32(f32 x)
{
    f32 result = floorf(x);
    return (i32) result;
}

f32 square(f32 x)
{
    f32 result = x*x;
    return result;
}

f32 square_root(f32 x)
{
    ASSERT(x >= 0);

    return sqrtf(x);
}

f32 clamp(f32 x, f32 min, f32 max)
{
    ASSERT(min < max);

    f32 result = x;
    if (x > max) result = max;
    if (x < min) result = min;

    return result;
}

f32 lerp(f32 a, f32 b, f32 t)
{
    ASSERT(0.f <= t && t <= 1.f);

    return (1.f - t) * a + t * b;
}

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


u32 rotate_left(u32 x, u32 n)
{
    n &= 31;
    u32 result = (x << n) | (x >> (32 - n));
    return result;
}

u32 rotate_right(u32 x, u32 n)
{
    n &= 31;
    u32 result = (x >> n) | (x << (32 - n));
    return result;
}

f32 min(f32 x, f32 y)
{
    f32 result = (x < y) ? x : y;
    return result;
}

f32 max(f32 x, f32 y)
{
    f32 result = (x < y) ? y : x;
    return result;
}

i32 min(i32 x, i32 y)
{
    i32 result = (x < y) ? x : y;
    return result;
}

i32 max(i32 x, i32 y)
{
    i32 result = (x < y) ? y : x;
    return result;
}

f32 sin(f32 x)
{
    f32 result = sinf(x);
    return result;
}

f32 cos(f32 x)
{
    f32 result = cosf(x);
    return result;
}

} // namespace math
