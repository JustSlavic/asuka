#ifndef ASUKA_COMMON_MATH_FLOAT_HPP
#define ASUKA_COMMON_MATH_FLOAT_HPP


#include <defines.hpp>
#include <math.h>


namespace math {


template <typename T>
struct consts {};


template <> struct consts<f32> {
    using self = f32;

    IN_CLASS_FUNCTION
    constexpr self pi () noexcept { return 3.14159265358979323846f; }
};


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


} // math


#endif // ASUKA_COMMON_MATH_FLOAT_HPP
