#ifndef ASUKA_COMMON_MATH_VECTOR2_HPP
#define ASUKA_COMMON_MATH_VECTOR2_HPP

#include <defines.hpp>
#include "float.hpp"


// @note: Template implementation


template <typename T>
struct Vector2
{
    union
    {
        struct { T x, y; };
        T data[2];
    };

    INLINE
    T operator [] (Int32 index)
    {
        ASSERT(index < 2);

        T result = data[index];
        return result;
    }
};

using Vec2F = Vector2<Float32>;
using Vec2I = Vector2<Int32>;
using Vec2U = Vector2<UInt32>;

// using V2 = Vec2F;
// using V2I = Vec2I;
// using V2U = Vec2U;


template <typename T> INLINE
Vector2<T> &operator += (Vector2<T> &a, Vector2<T> b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

template <typename T> INLINE
Vector2<T> &operator -= (Vector2<T> &a, Vector2<T> b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

template <typename T> INLINE
Vector2<T> &operator *= (Vector2<T> &a, T c)
{
    a.x *= c;
    a.y *= c;
    return a;
}

template <typename T> INLINE
Vector2<T> operator - (Vector2<T> a) {
    Vector2<T> result = { -a.x, -a.y };
    return result;
}

template <typename T> INLINE
Vector2<T> operator + (Vector2<T> a, Vector2<T> b) {
    Vector2<T> result = { a.x + b.x, a.y + b.y };
    return result;
}

template <typename T> INLINE
Vector2<T> operator - (Vector2<T> a, Vector2<T> b) {
    Vector2<T> result = { a.x - b.x, a.y - b.y };
    return result;
}

template <typename T> INLINE
Vector2<T> operator * (Vector2<T> a, f32 c) {
    Vector2<T> result = { c * a.x, c * a.y };
    return result;
}

template <typename T> INLINE
Vector2<T> operator * (f32 c, Vector2<T> a) {
    Vector2<T> result = { c * a.x, c * a.y };
    return result;
}

template <typename T> INLINE
Vector2<T> operator / (Vector2<T> a, f32 c) {
    Vector2<T> result = { a.x / c, a.y / c };
    return result;
}

template <typename T> INLINE
bool operator == (Vector2<T> a, Vector2<T> b) {
    bool result = (a.x == b.x) && (a.y == b.y);
    return result;
}

template <typename T> INLINE
bool operator != (Vector2<T> a, Vector2<T> b) {
    bool result = !(a == b);
    return result;
}

template <typename T> INLINE
f32 dot (Vector2<T> a, Vector2<T> b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}


template <typename T> INLINE
f32 length²(Vector2<T> a)
{
    f32 result = dot(a, a);
    return result;
}

template <typename T> INLINE
f32 length(Vector2<T> a)
{
    f32 result = math::sqrt(length²(a));
    return result;
}


// @note: Hadamard product
template <typename T> INLINE
Vector2<T> operator * (Vector2<T> a, Vector2<T> b)
{
    Vector2<T> result = { a.x * b.x, a.y * b.y };
    return result;
}

// @note: Hadamard division
template <typename T> INLINE
Vector2<T> operator / (Vector2<T> a, Vector2<T> b)
{
    Vector2<T> result = { a.x / b.x, a.y / b.y };
    return result;
}

template <typename T> INLINE
void normalize(Vector2<T>& a)
{
    f32 n = length(a);
    if (n > 0) {
        a.x /= n;
        a.y /= n;
    }
}


template <typename T> INLINE
Vector2<T> normalized(Vector2<T> a) {
    Vector2<T> result = a;
    normalize(result);
    return result;
}

template <typename T> INLINE
Vector2<T> clamp(Vector2<T> a, f32 min, f32 max) {
    Vector2<T> result{ math::clamp(a.x, min, max), math::clamp(a.y, min, max) };
    return result;
}

template <typename T> INLINE
Vector2<T> lerp (Vector2<T> a, Vector2<T> b, f32 t) {
    Vector2<T> result = Vector2<T>{ math::lerp(a.x, b.x, t), math::lerp(a.y, b.y, t) };
    return result;
}

template <typename T> INLINE
Vec2I round_to_v2i(Vector2<T> v) {
    Vec2I result = Vec2I{ math::round_to_i32(v.x), math::round_to_i32(v.y) };
    return result;
}

template <typename T> INLINE
Vec2I truncate_to_v2i(Vector2<T> v)
{
    Vec2I result = Vec2I{ math::truncate_to_i32(v.x), math::truncate_to_i32(v.y) };
    return result;
}


template <> INLINE
Vec2F cast<Vec2F, Vec2I>(Vec2I v)
{
    Vec2F result = { (Float32) v.x, (Float32) v.y };
    return result;
}



// @note: Vector2


union v2
{
    struct { f32  x,  y; };
    struct { f32  u,  v; };
    f32 data[2];

    INLINE
    f32 operator[] (i32 idx)
    {
        f32 result = data[idx];
        return result;
    }
};


INLINE
v2 &operator += (v2 &a, v2 b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

INLINE
v2 &operator -= (v2 &a, v2 b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

INLINE
v2 &operator *= (v2 &a, f32 c)
{
    a.x *= c;
    a.y *= c;
    return a;
}


[[nodiscard]] INLINE
b32 is_valid(v2 a)
{
    b32 valid = math::is_valid(a.x) && math::is_valid(a.y);
    return valid;
}

[[nodiscard]] INLINE
b32 is_zero(v2 a)
{
    b32 result = is_zero(a.x) && is_zero(a.y);
    return result;
}

[[nodiscard]] INLINE
v2 operator - (v2 a) {
    v2 result = v2{ -a.x, -a.y };
    return result;
}

[[nodiscard]] INLINE
v2 operator + (v2 a, v2 b) {
    v2 result = v2{ a.x + b.x, a.y + b.y };
    return result;
}

[[nodiscard]] INLINE
v2 operator - (v2 a, v2 b) {
    v2 result = v2{ a.x - b.x, a.y - b.y };
    return result;
}

[[nodiscard]] INLINE
v2 operator * (v2 a, f32 c) {
    v2 result = v2{ c * a.x, c * a.y };
    return result;
}

[[nodiscard]] INLINE
v2 operator * (f32 c, v2 a) {
    v2 result = v2{ c * a.x, c * a.y };
    return result;
}

[[nodiscard]] INLINE
v2 operator / (v2 a, f32 c) {
    v2 result = v2{ a.x / c, a.y / c };
    return result;
}

[[nodiscard]] INLINE
bool operator == (v2 a, v2 b) {
    bool result = (a.x == b.x) && (a.y == b.y);
    return result;
}

[[nodiscard]] INLINE
bool operator != (v2 a, v2 b) {
    bool result = !(a == b);
    return result;
}

[[nodiscard]] INLINE
f32 dot (v2 a, v2 b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}


[[nodiscard]] INLINE
f32 length²(v2 a)
{
    f32 result = dot(a, a);
    return result;
}

[[nodiscard]] INLINE
f32 length(v2 a)
{
    f32 result = math::sqrt(length²(a));
    return result;
}

[[nodiscard]] INLINE
v2 operator * (v2 a, v2 b)
{
    v2 result { a.x * b.x, a.y * b.y };
    return result;
}

[[nodiscard]] INLINE
v2 operator / (v2 a, v2 b)
{
    v2 result { a.x / b.x, a.y / b.y };
    return result;
}


INLINE
void normalize(v2& a)
{
    f32 n = length(a);
    if (n > 0) {
        a.x /= n;
        a.y /= n;
    }
}

[[nodiscard]] INLINE
v2 normalized(v2 a) {
    v2 result = a;
    normalize(result);
    return result;
}

[[nodiscard]] INLINE
v2 clamp(v2 a, f32 min, f32 max) {
    v2 result{ math::clamp(a.x, min, max), math::clamp(a.y, min, max) };
    return result;
}

[[nodiscard]] INLINE
v2 lerp (v2 a, v2 b, f32 t) {
    v2 result = v2{ math::lerp(a.x, b.x, t), math::lerp(a.y, b.y, t) };
    return result;
}

[[nodiscard]] INLINE
Vec2I round_to_v2i(v2 v) {
    Vec2I result = Vec2I{ math::round_to_i32(v.x), math::round_to_i32(v.y) };
    return result;
}

[[nodiscard]] INLINE
Vec2I truncate_to_v2i(v2 v)
{
    Vec2I result = Vec2I{ math::truncate_to_i32(v.x), math::truncate_to_i32(v.y) };
    return result;
}


[[nodiscard]] INLINE
v2 upcast_to_v2(Vec2I v) {
    v2 result = v2{ (f32) v.x, (f32) v.y };
    return result;
}


template <>
[[nodiscard]] INLINE
v2 cast<v2, Vec2I>(Vec2I v)
{
    v2 result = v2{ (f32) v.x, (f32) v.y };
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
INLINE v2 project(v2 a, v2 b) {
    v2 result = b * dot(a, b) / length²(b);
    return result;
}

//
// Projects 'a' onto line with normal 'norm'
//
INLINE v2 project_normal(v2 a, v2 norm) {
    v2 result = a - dot(a, norm) * norm;
    return result;
}


INLINE f32 projection(v2 a, v2 b) {
    f32 result = dot(a, b) / length(b);
    return result;
}

struct intersection_result {
    intersection_type found;
    v2 intersection;
};

INLINE
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

INLINE
intersection_result segment_segment_intersection(v2 p0, v2 p1, v2 q0, v2 q1) {
    intersection_result result {};

    v2 r = p1 - p0;
    v2 s = q1 - q0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((q0.x - p0.x) * s.y - (q0.y - p0.y) * s.x);

    if (math::absolute(denom) < EPSILON && math::absolute(nom) < EPSILON) {
        result = { INTERSECTION_COLLINEAR, v2{ NAN, NAN } };
        return result;
    }

    if (math::absolute(denom) < EPSILON && math::absolute(nom) > EPSILON) {
        result = { INTERSECTION_PARALLEL, v2{ NAN, NAN } };
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

Vec2I V2I(i32 x, i32 y)
{
    Vec2I result { x, y };
    return result;
}


#endif // ASUKA_COMMON_MATH_VECTOR2_HPP
