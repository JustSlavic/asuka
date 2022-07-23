#pragma once

#include <defines.hpp>
#include "vector2.hpp"
#include "vector3.hpp"


namespace Asuka
{

namespace Internal
{

template <typename T>
struct Vector4
{
    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        struct { Internal::Vector2<T> xy; Internal::Vector2<T> zw; };
        struct { Internal::Vector2<T> rg; Internal::Vector2<T> ba; };
        struct { Internal::Vector3<T> xyz; };
        struct { Internal::Vector3<T> rgb; };
        T e[4];
    };

    INLINE
    T operator [] (int32 index)
    {
        ASSERT(index < ARRAY_COUNT(e));

        T result = e[index];
        return result;
    }
};


template <typename T>
Vector4<T> &operator += (Vector4<T> &a, Vector4<T> b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

template <typename T>
Vector4<T> &operator -= (Vector4<T> &a, Vector4<T> b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
    return a;
}

template <typename T>
Vector4<T> &operator *= (Vector4<T> &a, T c)
{
    a.x *= c;
    a.y *= c;
    a.z *= c;
    a.w *= c;
    return a;
}

template <typename T>
Vector4<T> &operator /= (Vector4<T> &a, T c)
{
    a.x /= c;
    a.y /= c;
    a.z /= c;
    a.w /= c;
    return a;
}

template <typename T>
Vector4<T> operator - (Vector4<T> a)
{
    a.x = -a.x;
    a.y = -a.y;
    a.z = -a.z;
    a.w = -a.w;
    return a;
}

template <typename T>
Vector4<T> operator + (Vector4<T> a, Vector4<T> b)
{
    a += b;
    return a;
}

template <typename T>
Vector4<T> operator - (Vector4<T> a, Vector4<T> b)
{
    a -= b;
    return a;
}

template <typename T>
Vector4<T> operator * (Vector4<T> a, f32 c)
{
    a *= c;
    return a;
}

template <typename T>
Vector4<T> operator * (f32 c, Vector4<T> a)
{
    a *= c;
    return a;
}

template <typename T>
Vector4<T> operator / (Vector4<T> a, f32 c)
{
    a /= c;
    return a;
}

template <typename T>
bool operator == (Vector4<T> a, Vector4<T> b)
{
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
    return result;
}

template <typename T>
bool operator != (Vector4<T> a, Vector4<T> b)
{
    bool result = !(a == b);
    return result;
}

} // namespace Internal


using Vector4  = Internal::Vector4<f32>;
using Vector4i = Internal::Vector4<i32>;
using Vector4u = Internal::Vector4<u32>;

using vector4  = Internal::Vector4<f32>;
using vector4i = Internal::Vector4<i32>;
using vector4u = Internal::Vector4<u32>;

// @todo: remove typedefs below
using Vec4F = Vector4;
using Vec4I = Vector4i;
using Vec4U = Vector4u;

using v4  = vector4;
using v4i = vector4i;
using v4u = vector4u;


INLINE
b32 is_valid (Vector4 a) {
    b32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z) && is_valid(a.w);
    return valid;
}

INLINE
b32 is_zero (Vector4 a, f32 eps = EPSILON)
{
    b32 result = is_zero(a.x, eps) && is_zero(a.y, eps) && is_zero(a.z, eps) && is_zero(a.w, eps);
    return result;
}

INLINE
b32 is_equal (Vector4 a, Vector4 b, f32 eps = EPSILON)
{
    b32 result = is_equal(a.x, b.x, eps) && is_equal(a.y, b.y, eps) && is_equal(a.z, b.z, eps) && is_equal(a.w, b.w, eps);
    return result;
}

template <typename T>
Vector4 make_vector4 (T value)
{
    Vector4 result = { (f32) value, (f32) value, (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y, typename Z, typename W>
Vector4 make_vector4 (X x, Y y, Z z, W w)
{
    Vector4 result = { (f32) x, (f32) y, (f32) z, (f32) w };
    return result;
}

Vector4 make_vector4 (Vector4i v)
{
    Vector4 result = { (f32) v.x, (f32) v.y, (f32) v.z, (f32) v.w };
    return result;
}

INLINE
Vector4i make_vector4i (i32 x, i32 y, i32 z, i32 w)
{
    Vector4i result = { x, y, z, w };
    return result;
}

INLINE
Vector4i round_to_vector4i (Vector4 v)
{
    Vector4i result = { round_to_i32(v.x), round_to_i32(v.y), round_to_i32(v.z), round_to_i32(v.w) };
    return result;
}

INLINE
Vector4i truncate_to_vector4i (Vector4 v)
{
    Vector4i result = { truncate_to_i32(v.x), truncate_to_i32(v.y), truncate_to_i32(v.z), truncate_to_i32(v.w) };
    return result;
}

INLINE
f32 dot (Vector4 a, Vector4 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}

INLINE
Vector4 hadamard (Vector4 a, Vector4 b)
{
    Vector4 result = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
    return result;
}

INLINE
Vector4 hadamard_div (Vector4 a, Vector4 b)
{
    Vector4 result = { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    return result;
}

INLINE
Vector4 clamp (Vector4 a, f32 min, f32 max)
{
    Vector4 result = { clamp(a.x, min, max), clamp(a.y, min, max), clamp(a.z, min, max), clamp(a.w, min, max) };
    return result;
}

INLINE
Vector4 lerp (Vector4 a, Vector4 b, float32 t)
{
    Vector4 result = { lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t) };
    return result;
}

INLINE
f32 length2 (Vector4 a)
{
    f32 result = dot(a, a);
    return result;
}

INLINE
f32 length (Vector4 a)
{
    f32 result = sqrt(length2(a));
    return result;
}

INLINE
void normalize (Vector4 &a)
{
    f32 n = length(a);
    if (n > 0)
    {
        a.x /= n;
        a.y /= n;
        a.z /= n;
        a.w /= n;
    }
}

INLINE
Vector4 normalized (Vector4 a)
{
    Vector4 result = a;
    normalize(result);
    return result;
}


} // namespace Asuka
