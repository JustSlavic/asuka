#ifndef ASUKA_COMMON_MATH_VECTOR3_HPP
#define ASUKA_COMMON_MATH_VECTOR3_HPP

#include <defines.hpp>
#include "float.hpp"
#include "vector2.hpp"


struct v3i {
    union {
        struct { i32 x, y, z; };
        struct { Vec2I xy; i32 dummy_z; };
        i32 array_[3];
    };
};


[[nodiscard]] INLINE
bool operator == (v3i a, v3i b) {
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
    return result;
}

[[nodiscard]] INLINE
bool operator != (v3i a, v3i b) {
    bool result = !(a == b);
    return result;
}


struct v3 {
    union {
        struct { f32 x, y, z; };
        struct { v2 xy; f32 dummy_z; };
        f32 array_[3];
    };

    INLINE v3& operator += (v3 other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    INLINE v3& operator -= (v3 other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    INLINE v3& operator *= (f32 c) {
        x *= c;
        y *= c;
        z *= c;
        return *this;
    }
};


[[nodiscard]] INLINE
b32 is_valid(v3 a) {
    b32 valid = math::is_valid(a.x) && math::is_valid(a.y) && math::is_valid(a.z);
    return valid;
}

[[nodiscard]] INLINE
b32 is_zero(v3 a)
{
    b32 result = is_zero(a.x) && is_zero(a.y) && is_zero(a.z);
    return result;
}

[[nodiscard]] INLINE
b32 is_equal(v3 a, v3 b)
{
    b32 result = is_equal(a.x, b.x) && is_equal(a.y, b.y) && is_equal(a.z, b.z);
    return result;
}

[[nodiscard]] INLINE
v3 operator - (v3 a) {
    v3 result = v3{ -a.x, -a.y, -a.z };
    return result;
}

[[nodiscard]] INLINE
v3 operator + (v3 a, v3 b) {
    v3 result = v3{ a.x + b.x, a.y + b.y, a.z + b.z };
    return result;
}

[[nodiscard]] INLINE
v3 operator - (v3 a, v3 b) {
    v3 result = v3{ a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

[[nodiscard]] INLINE
v3 operator * (v3 a, f32 c) {
    v3 result = v3{ c * a.x, c * a.y, c * a.z };
    return result;
}

[[nodiscard]] INLINE
v3 operator * (f32 c, v3 a) {
    v3 result = v3{ c * a.x, c * a.y, c * a.z };
    return result;
}

[[nodiscard]] INLINE
v3 operator / (v3 a, f32 c)
{
    v3 result = v3{ a.x / c, a.y / c, a.z / c };
    return result;
}

[[nodiscard]] INLINE
bool operator == (v3 a, v3 b)
{
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
    return result;
}

[[nodiscard]] INLINE
bool operator != (v3 a, v3 b)
{
    bool result = !(a == b);
    return result;
}

[[nodiscard]] INLINE
f32 dot (v3 a, v3 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

[[nodiscard]] INLINE
v3 operator * (v3 a, v3 b)
{
    v3 result { a.x * b.x, a.y * b.y, a.z * b.z };
    return result;
}

[[nodiscard]] INLINE
v3 operator / (v3 a, v3 b)
{
    v3 result { a.x / b.x, a.y / b.y, a.z / b.z };
    return result;
}

[[nodiscard]] INLINE
v3 clamp(v3 a, f32 min, f32 max)
{
    v3 result{ math::clamp(a.x, min, max), math::clamp(a.y, min, max), math::clamp(a.z, min, max) };
    return result;
}

[[nodiscard]] INLINE
v3 lerp (v3 a, v3 b, f32 t)
{
    v3 result = v3{ math::lerp(a.x, b.x, t), math::lerp(a.y, b.y, t), math::lerp(a.z, b.z, t) };
    return result;
}

[[nodiscard]] INLINE
f32 length²(v3 a)
{
    f32 result = dot(a, a);
    return result;
}

[[nodiscard]] INLINE
f32 length(v3 a)
{
    f32 result = math::sqrt(length²(a));
    return result;
}

void normalize(v3 &a)
{
    f32 n = length(a);
    if (n > 0)
    {
        a.x /= n;
        a.y /= n;
        a.z /= n;
    }
}

[[nodiscard]]
v3 normalized(v3 a)
{
    v3 result = a;
    normalize(result);
    return result;
}


template <>
[[nodiscard]] INLINE
v3 cast<v3, v3i>(v3i v)
{
    v3 result = v3{ (f32) v.x, (f32) v.y, (f32) v.z };
    return result;
}


template <typename T>
v3 V3(T value)
{
    v3 result { (f32) value, (f32) value, (f32) value };
    return result;
}

template <typename T>
v3 V3(v2 v, T z)
{
    v3 result { v.x, v.y, (f32) z };
    return result;
}

template <typename X, typename Y, typename Z>
v3 V3(X x, Y y, Z z)
{
    v3 result { (f32) x, (f32) y, (f32) z };
    return result;
}


#endif // ASUKA_COMMON_MATH_VECTOR3_HPP
