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


b32 is_valid (vector4 a);
b32 is_zero (vector4 a, f32 eps = EPSILON);
b32 is_equal (vector4 a, vector4 b, f32 eps = EPSILON);

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

vector4 make_vector4 (v4i v);
v4i make_vector4i (i32 x, i32 y, i32 z, i32 w);
v4i round_to_vector4i (vector4 v);
v4i truncate_to_vector4i (vector4 v);
f32 dot (vector4 a, vector4 b);
vector4 hadamard (vector4 a, vector4 b);
vector4 hadamard_div (vector4 a, vector4 b);
vector4 clamp (vector4 a, f32 min, f32 max);
vector4 lerp (vector4 a, vector4 b, float32 t);
f32 length2 (vector4 a);
f32 length (vector4 a);
void normalize (vector4 &a);
vector4 normalized (vector4 a);

#if UNITY_BUILD
#include "vector4.cpp"
#endif // UNITY_BUILD
