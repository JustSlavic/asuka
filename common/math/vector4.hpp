#pragma once

#include <defines.hpp>
#include "vector2.hpp"
#include "vector3.hpp"


namespace Asuka {

template <typename T>
struct Vector4_Base
{
    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        struct { Vector2_Base<T> xy; Vector2_Base<T> zw; };
        struct { Vector3_Base<T> xyz; };
        struct { Vector3_Base<T> rgb; };
        T e[4];
    };

    INLINE
    T operator [] (Int32 index)
    {
        ASSERT(index < ARRAY_COUNT(e));

        T result = data[index];
        return result;
    }
};

using Vector4 = Vector4_Base<Float32>;
using Vec4F = Vector4_Base<Float32>;
using Vec4I = Vector4_Base<Int32>;
using Vec4U = Vector4_Base<UInt32>;

using v4 = Vec4F;


template <typename T>
Vector4_Base<T> &operator += (Vector4_Base<T> &a, Vector4_Base<T> b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

template <typename T>
Vector4_Base<T> &operator -= (Vector4_Base<T> &a, Vector4_Base<T> b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
    return a;
}

template <typename T>
Vector4_Base<T> &operator *= (Vector4_Base<T> &a, T c)
{
    a.x *= c;
    a.y *= c;
    a.z *= c;
    a.w *= c;
    return a;
}

template <typename T>
Vector4_Base<T> &operator /= (Vector4_Base<T> &a, T c)
{
    a.x /= c;
    a.y /= c;
    a.z /= c;
    a.w /= c;
    return a;
}

template <typename T>
Vector4_Base<T> operator - (Vector4_Base<T> a)
{
    a.x = -a.x;
    a.y = -a.y;
    a.z = -a.z;
    a.w = -a.w;
    return a;
}

template <typename T>
Vector4_Base<T> operator + (Vector4_Base<T> a, Vector4_Base<T> b)
{
    a += b;
    return a;
}

template <typename T>
Vector4_Base<T> operator - (Vector4_Base<T> a, Vector4_Base<T> b)
{
    a -= b;
    return a;
}

template <typename T>
Vector4_Base<T> operator * (Vector4_Base<T> a, f32 c)
{
    a *= c;
    return a;
}

template <typename T>
Vector4_Base<T> operator * (f32 c, Vector4_Base<T> a)
{
    a *= c;
    return a;
}

template <typename T>
Vector4_Base<T> operator / (Vector4_Base<T> a, f32 c)
{
    a /= c;
    return a;
}

template <typename T>
bool operator == (Vector4_Base<T> a, Vector4_Base<T> b)
{
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
    return result;
}

template <typename T>
bool operator != (Vector4_Base<T> a, Vector4_Base<T> b)
{
    bool result = !(a == b);
    return result;
}

INLINE
f32 dot (Vec4F a, Vec4F b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}

INLINE
Vec4F hadamard (Vec4F a, Vec4F b)
{
    Vec4F result = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
    return result;
}

INLINE
Vec4F hadamard_div (Vec4F a, Vec4F b)
{
    Vec4F result = { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    return result;
}

INLINE
Vec4F clamp (Vec4F a, f32 min, f32 max)
{
    Vec4F result =
    {
        clamp(a.x, min, max),
        clamp(a.y, min, max),
        clamp(a.z, min, max),
        clamp(a.w, min, max),
    };
    return result;
}

INLINE
Vec4F lerp (Vec4F a, Vec4F b, Float32 t)
{
    Vec4F result =
    {
        lerp(a.x, b.x, t),
        lerp(a.y, b.y, t),
        lerp(a.z, b.z, t),
        lerp(a.w, b.w, t),
    };
    return result;
}

INLINE
f32 length² (Vec4F a)
{
    f32 result = dot(a, a);
    return result;
}

INLINE
f32 length(Vec4F a)
{
    f32 result = sqrt(length²(a));
    return result;
}

INLINE
void normalize (Vec4F &a)
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
Vec4F normalized (Vec4F a)
{
    Vec4F result = a;
    normalize(result);
    return result;
}

// INLINE
// b32 is_valid(v4 a) {
//     b32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z) && is_valid(a.w);
//     return valid;
// }

// INLINE
// b32 is_zero(v3 a, f32 eps = EPSILON)
// {
//     b32 result = is_zero(a.x, eps) && is_zero(a.y, eps) && is_zero(a.z, eps);
//     return result;
// }

// INLINE
// b32 is_equal(v3 a, v3 b, f32 eps = EPSILON)
// {
//     b32 result = is_equal(a.x, b.x, eps) && is_equal(a.y, b.y, eps) && is_equal(a.z, b.z, eps);
//     return result;
// }


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

} // namespace Asuka
