#ifndef ASUKA_COMMON_MATH_VECTOR3_HPP
#define ASUKA_COMMON_MATH_VECTOR3_HPP

#include <defines.hpp>
#include "float.hpp"
#include "vector2.hpp"


template <typename T>
struct Vector3
{
    union {
        struct { T x, y, z; };
        struct { T r, g, b; };
        struct { Vector2<T> xy; };
        T e[3];
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
Vector3<T> &operator += (Vector3<T> &a, Vector3<T> b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

template <typename T>
Vector3<T> &operator -= (Vector3<T> &a, Vector3<T> b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

template <typename T>
Vector3<T> &operator *= (Vector3<T> &a, T c)
{
    a.x *= c;
    a.y *= c;
    a.z *= c;
    return a;
}

template <typename T>
Vector3<T> operator - (Vector3<T> a)
{
    Vector3<T> result = { -a.x, -a.y, -a.z };
    return result;
}

template <typename T>
Vector3<T> operator + (Vector3<T> a, Vector3<T> b)
{
    a += b;
    return a;
}

template <typename T>
Vector3<T> operator - (Vector3<T> a, Vector3<T> b)
{
    a -= b;
    return a;
}

template <typename T>
Vector3<T> operator * (Vector3<T> a, f32 c)
{
    a *= c;
    return a;
}

template <typename T>
Vector3<T> operator * (f32 c, Vector3<T> a)
{
    a *= c;
    return a;
}

template <typename T>
Vector3<T> operator / (Vector3<T> a, f32 c)
{
    a /= c;
    return a;
}

template <typename T>
bool operator == (Vector3<T> a, Vector3<T> b)
{
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
    return result;
}

template <typename T>
bool operator != (Vector3<T> a, Vector3<T> b)
{
    bool result = !(a == b);
    return result;
}


using vector3  = Vector3<f32>;
using vector3i = Vector3<i32>;
using vector3u = Vector3<u32>;

using v3  = Vector3<f32>;
using v3i = Vector3<i32>;
using v3u = Vector3<u32>;


INLINE
b32 is_valid(v3 a) {
    b32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z);
    return valid;
}

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

template <typename T>
v3 make_vector3 (T value)
{
    v3 result = { (f32) value, (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y, typename Z>
v3 make_vector3 (X x, Y y, Z z)
{
    v3 result = { (f32) x, (f32) y, (f32) z };
    return result;
}

template <typename Z>
v3 make_vector3 (v2 v, Z z)
{
    v3 result = { v.x, v.y, (f32) z };
    return result;
}

INLINE
v3 make_vector3 (v3i v)
{
    v3 result = { (f32) v.x, (f32) v.y, (f32) v.z };
    return result;
}

INLINE
v3i make_vector3i (i32 x, i32 y, i32 z)
{
    v3i result = { x, y, z };
    return result;
}

INLINE
v3i round_to_vector3i (v3 v)
{
    v3i result = { round_to_i32(v.x), round_to_i32(v.y), round_to_i32(v.z) };
    return result;
}

INLINE
v3i truncate_to_vector3i (v3 v)
{
    v3i result = { truncate_to_i32(v.x), truncate_to_i32(v.y), truncate_to_i32(v.z) };
    return result;
}


INLINE
f32 dot (v3 a, v3 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

INLINE
v3 hadamard (v3 a, v3 b)
{
    v3 result = { a.x * b.x, a.y * b.y, a.z * b.z };
    return result;
}

INLINE
v3 hadamard_div (v3 a, v3 b)
{
    v3 result = { a.x / b.x, a.y / b.y, a.z / b.z };
    return result;
}

INLINE
v3 clamp (v3 a, f32 min, f32 max)
{
    v3 result = { clamp(a.x, min, max), clamp(a.y, min, max), clamp(a.z, min, max) };
    return result;
}

INLINE
v3 lerp (v3 a, v3 b, float32 t)
{
    v3 result = { lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t) };
    return result;
}

INLINE
f32 length2 (v3 a)
{
    f32 result = dot(a, a);
    return result;
}

INLINE
f32 length(v3 a)
{
    f32 result = sqrt(length2(a));
    return result;
}

INLINE
void normalize (v3 &a)
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
v3 normalized (v3 a)
{
    Vector3 result = a;
    normalize(result);
    return result;
}


#endif // ASUKA_COMMON_MATH_VECTOR3_HPP
