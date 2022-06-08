#ifndef ASUKA_COMMON_MATH_VECTOR2_HPP
#define ASUKA_COMMON_MATH_VECTOR2_HPP

#include <defines.hpp>
#include "float.hpp"


namespace Asuka {


template <typename T>
struct Vector2_Base
{
    union
    {
        struct { T x, y; };
        T e[2];
    };

    INLINE
    T operator [] (Int32 index)
    {
        ASSERT(index < ARRAY_COUNT(e));

        T result = e[index];
        return result;
    }
};

using Vector2 = Vector2_Base<Float32>;
using Vec2F = Vector2_Base<Float32>;
using Vec2I = Vector2_Base<Int32>;
using Vec2U = Vector2_Base<UInt32>;

// using V2 = Vec2F;
// using V2I = Vec2I;
// using V2U = Vec2U;

using v2 = Vec2F;
using v2i = Vec2I;

template <typename T>
Vector2_Base<T> &operator += (Vector2_Base<T> &a, Vector2_Base<T> b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

template <typename T>
Vector2_Base<T> &operator -= (Vector2_Base<T> &a, Vector2_Base<T> b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

template <typename T>
Vector2_Base<T> &operator *= (Vector2_Base<T> &a, T c)
{
    a.x *= c;
    a.y *= c;
    return a;
}

template <typename T>
Vector2_Base<T> operator - (Vector2_Base<T> a) {
    Vector2_Base<T> result = { -a.x, -a.y };
    return result;
}

template <typename T>
Vector2_Base<T> operator + (Vector2_Base<T> a, Vector2_Base<T> b) {
    Vector2_Base<T> result = { a.x + b.x, a.y + b.y };
    return result;
}

template <typename T>
Vector2_Base<T> operator - (Vector2_Base<T> a, Vector2_Base<T> b) {
    Vector2_Base<T> result = { a.x - b.x, a.y - b.y };
    return result;
}

template <typename T>
Vector2_Base<T> operator * (Vector2_Base<T> a, f32 c) {
    Vector2_Base<T> result = { c * a.x, c * a.y };
    return result;
}

template <typename T>
Vector2_Base<T> operator * (f32 c, Vector2_Base<T> a) {
    Vector2_Base<T> result = { c * a.x, c * a.y };
    return result;
}

template <typename T>
Vector2_Base<T> operator / (Vector2_Base<T> a, f32 c) {
    Vector2_Base<T> result = { a.x / c, a.y / c };
    return result;
}

template <typename T>
bool operator == (Vector2_Base<T> a, Vector2_Base<T> b) {
    bool result = (a.x == b.x) && (a.y == b.y);
    return result;
}

template <typename T>
bool operator != (Vector2_Base<T> a, Vector2_Base<T> b) {
    bool result = !(a == b);
    return result;
}

template <typename T>
f32 dot (Vector2_Base<T> a, Vector2_Base<T> b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}


template <typename T>
f32 length²(Vector2_Base<T> a)
{
    f32 result = dot(a, a);
    return result;
}

template <typename T>
f32 length(Vector2_Base<T> a)
{
    f32 result = sqrt(length²(a));
    return result;
}


// @note: Hadamard product
template <typename T>
Vector2_Base<T> hadamard (Vector2_Base<T> a, Vector2_Base<T> b)
{
    Vector2_Base<T> result = { a.x * b.x, a.y * b.y };
    return result;
}

// @note: Hadamard division
template <typename T>
Vector2_Base<T> hadamard_div (Vector2_Base<T> a, Vector2_Base<T> b)
{
    Vector2_Base<T> result = { a.x / b.x, a.y / b.y };
    return result;
}

template <typename T>
void normalize(Vector2_Base<T>& a)
{
    f32 n = length(a);
    if (n > 0) {
        a.x /= n;
        a.y /= n;
    }
}


template <typename T>
Vector2_Base<T> normalized(Vector2_Base<T> a) {
    Vector2_Base<T> result = a;
    normalize(result);
    return result;
}

template <typename T>
Vector2_Base<T> clamp(Vector2_Base<T> a, f32 min, f32 max) {
    Vector2_Base<T> result{ clamp(a.x, min, max), clamp(a.y, min, max) };
    return result;
}

template <typename T>
Vector2_Base<T> lerp (Vector2_Base<T> a, Vector2_Base<T> b, f32 t) {
    Vector2_Base<T> result = Vector2_Base<T>{ lerp(a.x, b.x, t), lerp(a.y, b.y, t) };
    return result;
}

enum intersection_type {
    INTERSECTION_FOUND,      //
    INTERSECTION_PARALLEL,   //
    INTERSECTION_COLLINEAR,  //
    INTERSECTION_SKEW,       //
    INTERSECTION_OUT_BOUNDS, //
};

//
// Projects 'a' onto 'b'
//
INLINE
Vec2F project(Vec2F a, Vec2F b)
{
    Vec2F result = b * dot(a, b) / length²(b);
    return result;
}

//
// Projects 'a' onto line with normal 'norm'
//
INLINE
Vec2F project_normal(Vec2F a, Vec2F norm)
{
    Vec2F result = a - dot(a, norm) * norm;
    return result;
}


INLINE
Float32 projection(Vec2F a, Vec2F b) {
    Float32 result = dot(a, b) / length(b);
    return result;
}

struct intersection_result {
    intersection_type found;
    Vec2F intersection;
};

INLINE
Vec2F line_line_intersection(Vec2F r0, Vec2F r1, Vec2F s0, Vec2F s1) {
    Vec2F result;

    Vec2F r = r1 - r0;
    Vec2F s = s1 - s0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((s0.x - r0.x) * s.y - (s0.y - r0.y) * s.x);

    f32 t = nom / denom;
    result = r0 + t * r;

    return result;
}

INLINE
intersection_result segment_segment_intersection(Vec2F p0, Vec2F p1, Vec2F q0, Vec2F q1) {
    intersection_result result {};

    Vec2F r = p1 - p0;
    Vec2F s = q1 - q0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((q0.x - p0.x) * s.y - (q0.y - p0.y) * s.x);

    if (absolute(denom) < EPSILON && absolute(nom) < EPSILON) {
        result = { INTERSECTION_COLLINEAR, Vec2F{ NAN, NAN } };
        return result;
    }

    if (absolute(denom) < EPSILON && absolute(nom) > EPSILON) {
        result = { INTERSECTION_PARALLEL, Vec2F{ NAN, NAN } };
        return result;
    }

    f32 t = nom / denom;
    v2 intersection = p0 + t * r;

    // t < 0 checks for  X<-p0-->p1 situation
    // t > 0 requires us to check whether it's p0-->p1->X situation
    if ((t + EPSILON < 0) || (t > 0 && length²(intersection - p0) - EPSILON > length²(r))) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    // dot(intersection - q0, q1 - q0) < 0           checks for X<-q0-->q1 situation
    f32 vector_alignment = dot(intersection - q0, q1 - q0);
    // (intersection - q0).length_2() > s.length_2() checks for q0-->q1->X situation
    f32 vector_iq0_length = length²(intersection - q0);
    f32 vector_s_length = length²(s);
    if ((vector_alignment + EPSILON < 0) || (vector_iq0_length - EPSILON > vector_s_length)) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    result = { INTERSECTION_FOUND, intersection };
    return result;
}


template <typename T>
v2 V2(T value)
{
    v2 result { (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y>
v2 V2(X x, Y y)
{
    v2 result { (f32) x, (f32) y };
    return result;
}


Vec2F V2(Vec2I v)
{
    Vec2F result = { (Float32) v.x, (Float32) v.y };
    return result;
}

Vec2I V2I(i32 x, i32 y)
{
    Vec2I result { x, y };
    return result;
}

template <typename T> INLINE
Vec2I round_to_v2i(Vector2_Base<T> v) {
    Vec2I result = Vec2I{ round_to_i32(v.x), round_to_i32(v.y) };
    return result;
}

template <typename T> INLINE
Vec2I truncate_to_v2i(Vector2_Base<T> v)
{
    Vec2I result = Vec2I{ truncate_to_i32(v.x), truncate_to_i32(v.y) };
    return result;
}


} // namespace Asuka


#endif // ASUKA_COMMON_MATH_VECTOR2_HPP
