#ifndef ASUKA_COMMON_MATH_VECTOR2_HPP
#define ASUKA_COMMON_MATH_VECTOR2_HPP

#include <defines.hpp>
#include "float.hpp"


template <typename T>
struct Vector2
{
    union
    {
        struct { T x, y; };
        T e[2];
    };

    T & operator [] (int32 index)
    {
        ASSERT_MSG(index < ARRAY_COUNT(e), "Attempt to ");

        T & result = e[index];
        return result;
    }
};


template <typename T>
Vector2<T> &operator += (Vector2<T> &a, Vector2<T> b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

template <typename T>
Vector2<T> &operator -= (Vector2<T> &a, Vector2<T> b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

template <typename T>
Vector2<T> &operator *= (Vector2<T> &a, T c)
{
    a.x *= c;
    a.y *= c;
    return a;
}

template <typename T>
Vector2<T> operator - (Vector2<T> a) {
    Vector2<T> result = { -a.x, -a.y };
    return result;
}

template <typename T>
Vector2<T> operator + (Vector2<T> a, Vector2<T> b) {
    Vector2<T> result = { a.x + b.x, a.y + b.y };
    return result;
}

template <typename T>
Vector2<T> operator - (Vector2<T> a, Vector2<T> b) {
    Vector2<T> result = { a.x - b.x, a.y - b.y };
    return result;
}

template <typename T>
Vector2<T> operator * (Vector2<T> a, f32 c) {
    Vector2<T> result = { c * a.x, c * a.y };
    return result;
}

template <typename T>
Vector2<T> operator * (f32 c, Vector2<T> a) {
    Vector2<T> result = { c * a.x, c * a.y };
    return result;
}

template <typename T>
Vector2<T> operator / (Vector2<T> a, f32 c) {
    Vector2<T> result = { a.x / c, a.y / c };
    return result;
}

template <typename T>
bool operator == (Vector2<T> a, Vector2<T> b) {
    bool result = (a.x == b.x) && (a.y == b.y);
    return result;
}

template <typename T>
bool operator != (Vector2<T> a, Vector2<T> b) {
    bool result = !(a == b);
    return result;
}


using vector2  = Vector2<float32>;
using vector2i = Vector2<int32>;
using vector2u = Vector2<uint32>;

using v2  = Vector2<float32>;
using v2i = Vector2<int32>;
using v2u = Vector2<uint32>;


INLINE
b32 is_valid(vector2 a)
{
    b32 valid = is_valid(a.x) && is_valid(a.y);
    return valid;
}

INLINE
b32 is_zero(vector2 a, f32 eps = EPSILON)
{
    b32 result = is_zero(a.x, eps) && is_zero(a.y, eps);
    return result;
}

INLINE
b32 is_equal(vector2 a, vector2 b, f32 eps = EPSILON)
{
    b32 result = is_equal(a.x, b.x, eps) && is_equal(a.y, b.y, eps);
    return result;
}

// @note: I have to make it template function to generate all kinds of argument reloadings, because
// stupid C++ considers 1.0 being a double and you have to use f suffix to denote float value: 1.0f
// This quickly gets really annoying. I want to write make_vector2(1.0, 1.0), but I also need all
// kinds of overload functions, because I want to use make_vector2(x, 0.0) where x is a float and
// 0.0 is a double. It's also useful to have integer oveerloadings to write make_vector2(0, 0).
// @todo: Consider making a compile-time check to allow primitive types only.
template <typename T>
vector2 make_vector2 (T value)
{
    vector2 result = { (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y>
vector2 make_vector2 (X x, Y y)
{
    vector2 result = { (f32) x, (f32) y };
    return result;
}

INLINE
vector2 make_vector2 (vector2i v)
{
    vector2 result = { (f32) v.x, (f32) v.y };
    return result;
}

INLINE
vector2i make_vector2i (i32 x, i32 y)
{
    vector2i result = { x, y };
    return result;
}

INLINE
vector2i round_to_v2i (vector2 v) {
    vector2i result = { round_to_i32(v.x), round_to_i32(v.y) };
    return result;
}

INLINE
vector2i truncate_to_v2i (vector2 v)
{
    vector2i result = { truncate_to_i32(v.x), truncate_to_i32(v.y) };
    return result;
}

INLINE
f32 dot (vector2 a, vector2 b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

INLINE
f32 length2 (vector2 a)
{
    f32 result = dot(a, a);
    return result;
}

INLINE
f32 length (vector2 a)
{
    f32 result = square_root(length2(a));
    return result;
}

// @note: Hadamard product
// @todo: Should I make it operator* ?
INLINE
vector2 hadamard (vector2 a, vector2 b)
{
    vector2 result = { a.x * b.x, a.y * b.y };
    return result;
}

// @note: Hadamard division
INLINE
vector2 hadamard_div (vector2 a, vector2 b)
{
    vector2 result = { a.x / b.x, a.y / b.y };
    return result;
}

INLINE
void normalize (vector2 &a)
{
    f32 n = length(a);
    if (n > 0) {
        a.x /= n;
        a.y /= n;
    }
}

INLINE
vector2 normalized (vector2 a) {
    vector2 result = a;
    normalize(result);
    return result;
}

INLINE
vector2 clamp (vector2 a, f32 min, f32 max) {
    vector2 result = { clamp(a.x, min, max), clamp(a.y, min, max) };
    return result;
}

INLINE
vector2 lerp (vector2 a, vector2 b, f32 t) {
    vector2 result = { lerp(a.x, b.x, t), lerp(a.y, b.y, t) };
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
v2 project(v2 a, v2 b)
{
    v2 result = b * dot(a, b) / length2(b);
    return result;
}

//
// Projects 'a' onto line with normal 'norm'
v2 project_normal(v2 a, v2 norm)
{
    v2 result = a - dot(a, norm) * norm;
    return result;
}
f32 projection(v2 a, v2 b) {
    f32 result = dot(a, b) / length(b);
    return result;
}

struct intersection_result {
    intersection_type found;
    v2 intersection;
};

v2 line_line_intersection(v2 r0, v2 r1, v2 s0, v2 s1) {
    v2 result;

    v2 r = r1 - r0;
    v2 s = s1 - s0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((s0.x - r0.x) * s.y - (s0.y - r0.y) * s.x);

    f32 t = nom / denom;
    result = r0 + t * r;

    return result;
}

intersection_result segment_segment_intersection(v2 p0, v2 p1, v2 q0, v2 q1) {
    intersection_result result {};

    v2 r = p1 - p0;
    v2 s = q1 - q0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((q0.x - p0.x) * s.y - (q0.y - p0.y) * s.x);

    if (absolute(denom) < EPSILON && absolute(nom) < EPSILON) {
        result = { INTERSECTION_COLLINEAR, v2{ NAN, NAN } };
        return result;
    }

    if (absolute(denom) < EPSILON && absolute(nom) > EPSILON) {
        result = { INTERSECTION_PARALLEL, v2{ NAN, NAN } };
        return result;
    }

    f32 t = nom / denom;
    Vector2 intersection = p0 + t * r;

    // t < 0 checks for  X<-p0-->p1 situation
    // t > 0 requires us to check whether it's p0-->p1->X situation
    if ((t + EPSILON < 0) || (t > 0 && length2(intersection - p0) - EPSILON > length2(r))) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    // dot(intersection - q0, q1 - q0) < 0           checks for X<-q0-->q1 situation
    f32 vector_alignment = dot(intersection - q0, q1 - q0);
    // (intersection - q0).length_2() > s.length_2() checks for q0-->q1->X situation
    f32 vector_iq0_length = length2(intersection - q0);
    f32 vector_s_length = length2(s);
    if ((vector_alignment + EPSILON < 0) || (vector_iq0_length - EPSILON > vector_s_length)) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    result = { INTERSECTION_FOUND, intersection };
    return result;
}


#endif // ASUKA_COMMON_MATH_VECTOR2_HPP
