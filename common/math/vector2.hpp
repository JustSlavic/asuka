#pragma once

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


b32 is_valid(vector2 a);
b32 is_zero(vector2 a, f32 eps = EPSILON);
b32 is_equal(vector2 a, vector2 b, f32 eps = EPSILON);

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

vector2 make_vector2 (vector2i v);
vector2i make_vector2i (i32 x, i32 y);
vector2i round_to_v2i (vector2 v);
vector2i truncate_to_v2i (vector2 v);
f32 dot (vector2 a, vector2 b);
f32 length2 (vector2 a);
f32 length (vector2 a);

// @note: Hadamard product
// @todo: Should I make it operator* ?
vector2 hadamard (vector2 a, vector2 b);
// @note: Hadamard division
vector2 hadamard_div (vector2 a, vector2 b);
void normalize (vector2 &a);
vector2 normalized (vector2 a);
vector2 clamp (vector2 a, f32 min, f32 max);
vector2 lerp (vector2 a, vector2 b, f32 t);

//
// Projects 'a' onto 'b'
//
v2 project(v2 a, v2 b);

//
// Projects 'a' onto line with normal 'norm'
//
v2 project_normal(v2 a, v2 norm);

//
// Gets length of the projection of 'a' onto 'b'
//
f32 projection(v2 a, v2 b);

v2 line_line_intersection(v2 r0, v2 r1, v2 s0, v2 s1);

enum intersection_type {
    INTERSECTION_FOUND,      //
    INTERSECTION_PARALLEL,   //
    INTERSECTION_COLLINEAR,  //
    INTERSECTION_SKEW,       //
    INTERSECTION_OUT_BOUNDS, //
};

struct intersection_result {
    intersection_type found;
    v2 intersection;
};

intersection_result segment_segment_intersection(v2 p0, v2 p1, v2 q0, v2 q1);

#if UNITY_BUILD
#include "vector2.cpp"
#endif // UNITY_BUILD
