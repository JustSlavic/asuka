#pragma once

#include <defines.hpp>
#include "vector2.hpp"
#include "vector3.hpp"


namespace math {

struct vector4 {
    union {
        struct { float32 x, y, z, w; };
        struct { vector2 xy, zw; };
        struct { vector3 xyz; float32 dummy_w; };
    };


};


using vec4 = vector4;
using v4 = vector4;

inline bool32 is_valid(vector4 a) {
    bool32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z) && is_valid(a.w);
    return valid;
}

inline vector4 operator - (vector4 a) {
    vector4 result = vector4{ -a.x, -a.y, -a.z, -a.w };
    return result;
}

inline vector4 operator + (vector4 a, vector4 b) {
    vector4 result = vector4{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
    return result;
}

inline vector4 operator - (vector4 a, vector4 b) {
    vector4 result = vector4{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
    return result;
}

inline vector4 operator * (vector4 a, f32 c) {
    vector4 result = vector4{ c * a.x, c * a.y, c * a.z, c * a.w };
    return result;
}

inline vector4 operator * (f32 c, vector4 a) {
    vector4 result = vector4{ c * a.x, c * a.y, c * a.z, c * a.w };
    return result;
}

inline vector4 operator / (vector4 a, f32 c) {
    vector4 result = vector4{ a.x / c, a.y / c, a.z / c, a.w / c };
    return result;
}

inline bool operator == (vector4 a, vector4 b) {
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
    return result;
}

inline bool operator != (vector4 a, vector4 b) {
    bool result = !(a == b);
    return result;
}

inline f32 dot (vector4 a, vector4 b) {
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}

inline vector4 clamp(vector4 a, f32 min, f32 max) {
    vector4 result{ clamp(a.x, min, max), clamp(a.y, min, max), clamp(a.z, min, max), clamp(a.w, min, max) };
    return result;
}

inline vector4 lerp (vector4 a, vector4 b, f32 t) {
    vector4 result = vector4{ lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t) };
    return result;
}


} // namespace math

template <typename T>
math::v4 V4(T value)
{
    math::v4 result { (f32) value, (f32) value, (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y, typename Z, typename W>
math::v4 V4(X x, Y y, Z z, W w)
{
    math::v4 result { (f32) x, (f32) y, (f32) z, (f32) w };
    return result;
}
