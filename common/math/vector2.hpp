#ifndef ASUKA_COMMON_MATH_VECTOR2_HPP
#define ASUKA_COMMON_MATH_VECTOR2_HPP

#include <defines.hpp>
#include "float.hpp"


namespace Asuka
{

namespace Internal
{

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

} // namespace Internal


using Vector2  = Internal::Vector2<float32>;
using Vector2f = Internal::Vector2<float32>;
using Vector2i = Internal::Vector2<int32>;
using Vector2u = Internal::Vector2<uint32>;

using vector2  = Vector2;
using vector2f = Vector2f;
using vector2i = Vector2i;
using vector2u = Vector2u;

// @todo: remove typedefs below
using Vec2F = Vector2;
using Vec2I = Vector2i;
using Vec2U = Vector2u;

using v2  = Vector2;
using v2i = Vector2i;
using v2u = Vector2u;


INLINE
b32 is_valid(Vector2 a) {
    b32 valid = is_valid(a.x) && is_valid(a.y);
    return valid;
}

INLINE
b32 is_zero(Vector2 a, f32 eps = EPSILON)
{
    b32 result = is_zero(a.x, eps) && is_zero(a.y, eps);
    return result;
}

INLINE
b32 is_equal(Vector2 a, Vector2 b, f32 eps = EPSILON)
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
Vector2 make_vector2 (T value)
{
    Vector2 result = { (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y>
Vector2 make_vector2 (X x, Y y)
{
    Vector2 result = { (f32) x, (f32) y };
    return result;
}

INLINE
Vector2 make_vector2 (Vector2i v)
{
    Vector2 result = { (f32) v.x, (f32) v.y };
    return result;
}

INLINE
Vector2i make_vector2i (i32 x, i32 y)
{
    Vector2i result = { x, y };
    return result;
}

INLINE
Vector2i round_to_v2i (Vector2 v) {
    Vector2i result = { round_to_i32(v.x), round_to_i32(v.y) };
    return result;
}

INLINE
Vector2i truncate_to_v2i (Vector2 v)
{
    Vector2i result = { truncate_to_i32(v.x), truncate_to_i32(v.y) };
    return result;
}

INLINE
f32 dot (Vector2 a, Vector2 b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

INLINE
f32 length2 (Vector2 a)
{
    f32 result = dot(a, a);
    return result;
}

INLINE
f32 length (Vector2 a)
{
    f32 result = sqrt(length2(a));
    return result;
}

// @note: Hadamard product
// @todo: Should I make it operator* ?
INLINE
Vector2 hadamard (Vector2 a, Vector2 b)
{
    Vector2 result = { a.x * b.x, a.y * b.y };
    return result;
}

// @note: Hadamard division
INLINE
Vector2 hadamard_div (Vector2 a, Vector2 b)
{
    Vector2 result = { a.x / b.x, a.y / b.y };
    return result;
}

INLINE
void normalize (Vector2 &a)
{
    f32 n = length(a);
    if (n > 0) {
        a.x /= n;
        a.y /= n;
    }
}

INLINE
Vector2 normalized (Vector2 a) {
    Vector2 result = a;
    normalize(result);
    return result;
}

INLINE
Vector2 clamp (Vector2 a, f32 min, f32 max) {
    Vector2 result = { clamp(a.x, min, max), clamp(a.y, min, max) };
    return result;
}

INLINE
Vector2 lerp (Vector2 a, Vector2 b, f32 t) {
    Vector2 result = { lerp(a.x, b.x, t), lerp(a.y, b.y, t) };
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
Vector2 project(Vector2 a, Vector2 b)
{
    Vector2 result = b * dot(a, b) / length2(b);
    return result;
}

//
// Projects 'a' onto line with normal 'norm'
Vector2 project_normal(Vector2 a, Vector2 norm)
{
    Vector2 result = a - dot(a, norm) * norm;
    return result;
}
f32 projection(Vector2 a, Vector2 b) {
    f32 result = dot(a, b) / length(b);
    return result;
}

struct intersection_result {
    intersection_type found;
    Vector2 intersection;
};

Vector2 line_line_intersection(Vector2 r0, Vector2 r1, Vector2 s0, Vector2 s1) {
    Vector2 result;

    Vector2 r = r1 - r0;
    Vector2 s = s1 - s0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((s0.x - r0.x) * s.y - (s0.y - r0.y) * s.x);

    f32 t = nom / denom;
    result = r0 + t * r;

    return result;
}

intersection_result segment_segment_intersection(Vector2 p0, Vector2 p1, Vector2 q0, Vector2 q1) {
    intersection_result result {};

    Vector2 r = p1 - p0;
    Vector2 s = q1 - q0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((q0.x - p0.x) * s.y - (q0.y - p0.y) * s.x);

    if (absolute(denom) < EPSILON && absolute(nom) < EPSILON) {
        result = { INTERSECTION_COLLINEAR, Vector2{ NAN, NAN } };
        return result;
    }

    if (absolute(denom) < EPSILON && absolute(nom) > EPSILON) {
        result = { INTERSECTION_PARALLEL, Vector2{ NAN, NAN } };
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


} // namespace Asuka


#endif // ASUKA_COMMON_MATH_VECTOR2_HPP
