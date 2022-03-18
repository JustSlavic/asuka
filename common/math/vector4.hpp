#pragma once

#include <defines.hpp>
#include "vector2.hpp"
#include "vector3.hpp"


struct v4 {
    union {
        struct { f32 x, y, z, w; };
        struct { v2 xy, zw; };
        struct { v3 xyz; f32 dummy_w; };
    };
};


inline b32 is_valid(v4 a) {
    b32 valid = math::is_valid(a.x) && math::is_valid(a.y) && math::is_valid(a.z) && math::is_valid(a.w);
    return valid;
}

inline v4 operator - (v4 a) {
    v4 result = v4{ -a.x, -a.y, -a.z, -a.w };
    return result;
}

inline v4 operator + (v4 a, v4 b) {
    v4 result = v4{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
    return result;
}

inline v4 operator - (v4 a, v4 b) {
    v4 result = v4{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
    return result;
}

inline v4 operator * (v4 a, f32 c) {
    v4 result = v4{ c * a.x, c * a.y, c * a.z, c * a.w };
    return result;
}

inline v4 operator * (f32 c, v4 a) {
    v4 result = v4{ c * a.x, c * a.y, c * a.z, c * a.w };
    return result;
}

inline v4 operator / (v4 a, f32 c) {
    v4 result = v4{ a.x / c, a.y / c, a.z / c, a.w / c };
    return result;
}

inline bool operator == (v4 a, v4 b) {
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
    return result;
}

inline bool operator != (v4 a, v4 b) {
    bool result = !(a == b);
    return result;
}

inline f32 dot (v4 a, v4 b) {
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}

inline v4 clamp(v4 a, f32 min, f32 max) {
    v4 result{ math::clamp(a.x, min, max), math::clamp(a.y, min, max), math::clamp(a.z, min, max), math::clamp(a.w, min, max) };
    return result;
}

inline v4 lerp (v4 a, v4 b, f32 t) {
    v4 result = v4{ math::lerp(a.x, b.x, t), math::lerp(a.y, b.y, t), math::lerp(a.z, b.z, t), math::lerp(a.w, b.w, t) };
    return result;
}


template <typename T>
v4 V4(T value)
{
    v4 result { (f32) value, (f32) value, (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y, typename Z, typename W>
v4 V4(X x, Y y, Z z, W w)
{
    v4 result { (f32) x, (f32) y, (f32) z, (f32) w };
    return result;
}
