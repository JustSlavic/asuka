#pragma once

#include <defines.hpp>

f32 infinity();
f32 pi();
f32 nan();

b32 is_zero(f32 x, f32 eps = EPSILON);
b32 is_equal(f32 x, f32 y, f32 eps = EPSILON);
b32 is_nan(f32 x);
b32 is_inf(f32 x);
b32 is_valid(f32 x);
f32 absolute(f32 x);
i32 absolute(i32 x);
i32 sign(i32 value);
f32 sign(f32 value);
i32 round_to_i32(f32 x);
u32 round_to_u32(f32 x);
i32 truncate_to_i32(f32 x);
i32 floor_to_int32(f32 x);
f32 square(f32 x);
f32 square_root(f32 x);
f32 clamp(f32 x, f32 min, f32 max);
f32 lerp(f32 a, f32 b, f32 t);
f32 smoothstep(f32 x);

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



u32 rotate_left(u32 x, u32 n);
u32 rotate_right(u32 x, u32 n);
f32 min(f32 x, f32 y);
f32 max(f32 x, f32 y);
i32 min(i32 x, i32 y);
i32 max(i32 x, i32 y);
f32 sin(f32 x);
f32 cos(f32 x);

} // namespace math

#if UNITY_BUILD
#include "float.cpp"
#endif // UNITY_BUILD
