#pragma once

#include <defines.hpp>
#include "vector2.hpp"
#include "vector3.hpp"


template <typename T>
struct Vector4
{
    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        struct { Vector2<T> xy; Vector2<T> zw; };
        struct { Vector2<T> rg; Vector2<T> ba; };
        struct { Vector3<T> xyz; };
        struct { Vector3<T> rgb; };
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


using vector4  = Vector4<f32>;
using vector4i = Vector4<i32>;
using vector4u = Vector4<u32>;

using v4  = Vector4<f32>;
using v4i = Vector4<i32>;
using v4u = Vector4<u32>;


INLINE
b32 is_valid (vector4 a) {
    b32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z) && is_valid(a.w);
    return valid;
}

INLINE
b32 is_zero (vector4 a, f32 eps = EPSILON)
{
    b32 result = is_zero(a.x, eps) && is_zero(a.y, eps) && is_zero(a.z, eps) && is_zero(a.w, eps);
    return result;
}

INLINE
b32 is_equal (vector4 a, vector4 b, f32 eps = EPSILON)
{
    b32 result = is_equal(a.x, b.x, eps) && is_equal(a.y, b.y, eps) && is_equal(a.z, b.z, eps) && is_equal(a.w, b.w, eps);
    return result;
}

template <typename T>
vector4 make_vector4 (T value)
{
    vector4 result = { (f32) value, (f32) value, (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y, typename Z, typename W>
vector4 make_vector4 (X x, Y y, Z z, W w)
{
    vector4 result = { (f32) x, (f32) y, (f32) z, (f32) w };
    return result;
}

vector4 make_vector4 (v4i v)
{
    vector4 result = { (f32) v.x, (f32) v.y, (f32) v.z, (f32) v.w };
    return result;
}

INLINE
v4i make_vector4i (i32 x, i32 y, i32 z, i32 w)
{
    v4i result = { x, y, z, w };
    return result;
}

INLINE
v4i round_to_vector4i (vector4 v)
{
    v4i result = { round_to_i32(v.x), round_to_i32(v.y), round_to_i32(v.z), round_to_i32(v.w) };
    return result;
}

INLINE
v4i truncate_to_vector4i (vector4 v)
{
    v4i result = { truncate_to_i32(v.x), truncate_to_i32(v.y), truncate_to_i32(v.z), truncate_to_i32(v.w) };
    return result;
}

INLINE
f32 dot (vector4 a, vector4 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}

INLINE
vector4 hadamard (vector4 a, vector4 b)
{
    vector4 result = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
    return result;
}

INLINE
vector4 hadamard_div (vector4 a, vector4 b)
{
    vector4 result = { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    return result;
}

INLINE
vector4 clamp (vector4 a, f32 min, f32 max)
{
    vector4 result = { clamp(a.x, min, max), clamp(a.y, min, max), clamp(a.z, min, max), clamp(a.w, min, max) };
    return result;
}

INLINE
vector4 lerp (vector4 a, vector4 b, float32 t)
{
    vector4 result = { lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t) };
    return result;
}

INLINE
f32 length2 (vector4 a)
{
    f32 result = dot(a, a);
    return result;
}

INLINE
f32 length (vector4 a)
{
    f32 result = sqrt(length2(a));
    return result;
}

INLINE
void normalize (vector4 &a)
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
vector4 normalized (vector4 a)
{
    Vector4 result = a;
    normalize(result);
    return result;
}

