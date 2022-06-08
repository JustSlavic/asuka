#ifndef ASUKA_COMMON_MATH_VECTOR3_HPP
#define ASUKA_COMMON_MATH_VECTOR3_HPP

#include <defines.hpp>
#include "float.hpp"
#include "vector2.hpp"

namespace Asuka {

template <typename T>
struct Vector3_Base
{
    union {
        struct { T x, y, z; };
        struct { T r, g, b; };
        struct { Vector2_Base<T> xy; };
        T e[3];
    };

    INLINE
    T operator [] (Int32 index)
    {
        ASSERT(index < ARRAY_COUNT(e));

        T result = data[index];
        return result;
    }
};

using Vector3 = Vector3_Base<Float32>;
using Vec3F = Vector3_Base<Float32>;
using Vec3I = Vector3_Base<Int32>;
using Vec3U = Vector3_Base<UInt32>;

// using V3 = Vec3F;
// using V3I = Vec3I;
// using V3U = Vec3U;

using v3 = Vector3;


template <typename T>
Vector3_Base<T> &operator += (Vector3_Base<T> &a, Vector3_Base<T> b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

template <typename T>
Vector3_Base<T> &operator -= (Vector3_Base<T> &a, Vector3_Base<T> b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

template <typename T>
Vector3_Base<T> &operator *= (Vector3_Base<T> &a, T c) {
    a.x *= c;
    a.y *= c;
    a.z *= c;
    return a;
}

template <typename T>
Vector3_Base<T> operator - (Vector3_Base<T> a)
{
    Vector3_Base<T> result = { -a.x, -a.y, -a.z };
    return result;
}

template <typename T>
Vector3_Base<T> operator + (Vector3_Base<T> a, Vector3_Base<T> b)
{
    Vector3_Base<T> result = { a.x + b.x, a.y + b.y, a.z + b.z };
    return result;
}

template <typename T>
Vector3_Base<T> operator - (Vector3_Base<T> a, Vector3_Base<T> b)
{
    Vector3_Base<T> result = { a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

template <typename T>
Vector3_Base<T> operator * (Vector3_Base<T> a, f32 c)
{
    Vector3_Base<T> result = { c * a.x, c * a.y, c * a.z };
    return result;
}

template <typename T>
Vector3_Base<T> operator * (f32 c, Vector3_Base<T> a)
{
    Vector3_Base<T> result = a * c;
    return result;
}

template <typename T>
Vector3_Base<T> operator / (Vector3_Base<T> a, f32 c)
{
    Vector3_Base<T> result = { a.x / c, a.y / c, a.z / c };
    return result;
}

template <typename T>
bool operator == (Vector3_Base<T> a, Vector3_Base<T> b)
{
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
    return result;
}

template <typename T>
bool operator != (Vector3_Base<T> a, Vector3_Base<T> b)
{
    bool result = !(a == b);
    return result;
}

INLINE
f32 dot (Vec3F a, Vec3F b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

INLINE
Vec3F hadamard (Vec3F a, Vec3F b)
{
    Vec3F result = { a.x * b.x, a.y * b.y, a.z * b.z };
    return result;
}

INLINE
Vec3F hadamard_div (Vec3F a, Vec3F b)
{
    Vec3F result = { a.x / b.x, a.y / b.y, a.z / b.z };
    return result;
}

INLINE
Vec3F clamp (Vec3F a, f32 min, f32 max)
{
    Vec3F result = {
        clamp(a.x, min, max),
        clamp(a.y, min, max),
        clamp(a.z, min, max)};
    return result;
}

INLINE
Vec3F lerp (Vec3F a, Vec3F b, Float32 t)
{
    Vec3F result = {
        lerp(a.x, b.x, t),
        lerp(a.y, b.y, t),
        lerp(a.z, b.z, t)};
    return result;
}

INLINE
f32 length² (Vec3F a)
{
    f32 result = dot(a, a);
    return result;
}

INLINE
f32 length(Vec3F a)
{
    f32 result = sqrt(length²(a));
    return result;
}

INLINE
void normalize (Vec3F &a)
{
    f32 n = length(a);
    if (n > 0)
    {
        a.x /= n;
        a.y /= n;
        a.z /= n;
    }
}

INLINE
Vec3F normalized (Vec3F a)
{
    Vec3F result = a;
    normalize(result);
    return result;
}

INLINE
Vec3I round_to_v3i (Vec3F v)
{
    Vec3I result = { round_to_i32(v.x), round_to_i32(v.y), round_to_i32(v.z) };
    return result;
}

// INLINE
// b32 is_valid(v3 a) {
//     b32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z);
//     return valid;
// }

INLINE
b32 is_zero(v3 a, f32 eps = EPSILON)
{
    b32 result = is_zero(a.x, eps) && is_zero(a.y, eps) && is_zero(a.z, eps);
    return result;
}

INLINE
b32 is_equal(v3 a, v3 b, f32 eps = EPSILON)
{
    b32 result = is_equal(a.x, b.x, eps) && is_equal(a.y, b.y, eps) && is_equal(a.z, b.z, eps);
    return result;
}


INLINE
Vec3F cast_to_v3(Vec3I v)
{
    Vec3F result = { (f32) v.x, (f32) v.y, (f32) v.z };
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

} // namespace Asuka

#endif // ASUKA_COMMON_MATH_VECTOR3_HPP
