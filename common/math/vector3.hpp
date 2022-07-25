#pragma once

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


b32 is_valid(v3 a);
b32 is_zero(v3 a, f32 eps = EPSILON);
b32 is_equal(v3 a, v3 b, f32 eps = EPSILON);

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

v3 make_vector3 (v3i v);
v3i make_vector3i (i32 x, i32 y, i32 z);
v3i round_to_vector3i (v3 v);
v3i truncate_to_vector3i (v3 v);
f32 dot (v3 a, v3 b);
v3 hadamard (v3 a, v3 b);
v3 hadamard_div (v3 a, v3 b);
v3 clamp (v3 a, f32 min, f32 max);
v3 lerp (v3 a, v3 b, float32 t);
f32 length2 (v3 a);
f32 length(v3 a);
void normalize (v3 &a);
v3 normalized (v3 a);

#if UNITY_BUILD
#include "vector3.cpp"
#endif // UNITY_BUILD
