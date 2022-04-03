#ifndef ASUKA_COMMON_MATH_VECTOR2_HPP
#define ASUKA_COMMON_MATH_VECTOR2_HPP

#include <defines.hpp>
#include "float.hpp"


struct v2i {
    union {
        struct { i32  x, y; };
        i32 coordinates[2];
    };

    inline i32 operator[] (i32 idx) {
        i32 result = coordinates[idx];
        return result;
    }
};


inline v2i operator - (v2i a, v2i b) {
    v2i result = v2i{ a.x - b.x, a.y - b.y };
    return result;
}

inline v2i operator + (v2i a, v2i b) {
    v2i result = v2i{ a.x + b.x, a.y + b.y};
    return result;
}


struct v2 {
    union {
        struct { f32  x,  y; };
        struct { f32  u,  v; };
        f32 coordinates[2];
    };

    inline v2& operator += (v2 other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    inline v2& operator -= (v2 other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    inline v2& operator *= (f32 c) {
        x *= c;
        y *= c;
        return *this;
    }

    IN_CLASS_FUNCTION
    constexpr v2 nan() {
        v2 result { math::NaN, math::NaN };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr v2 zero() {
        v2 result { 0, 0 };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr v2 one() {
        v2 result { 1, 1 };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr v2 ex() {
        v2 result { 1, 0 };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr v2 ey() {
        v2 result { 0, 1 };
        return result;
    }
};


inline b32 is_valid(v2 a) {
    b32 valid = math::is_valid(a.x) && math::is_valid(a.y);
    return valid;
}

inline v2 operator - (v2 a) {
    v2 result = v2{ -a.x, -a.y };
    return result;
}

inline v2 operator + (v2 a, v2 b) {
    v2 result = v2{ a.x + b.x, a.y + b.y };
    return result;
}

inline v2 operator - (v2 a, v2 b) {
    v2 result = v2{ a.x - b.x, a.y - b.y };
    return result;
}

inline v2 operator * (v2 a, f32 c) {
    v2 result = v2{ c * a.x, c * a.y };
    return result;
}

inline v2 operator * (f32 c, v2 a) {
    v2 result = v2{ c * a.x, c * a.y };
    return result;
}

inline v2 operator / (v2 a, f32 c) {
    v2 result = v2{ a.x / c, a.y / c };
    return result;
}

inline bool operator == (v2 a, v2 b) {
    bool result = (a.x == b.x) && (a.y == b.y);
    return result;
}

inline bool operator != (v2 a, v2 b) {
    bool result = !(a == b);
    return result;
}

inline f32 dot (v2 a, v2 b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

[[nodiscard]] inline
f32 length_squared(v2 a)
{
    f32 result = dot(a, a);
    return result;
}

[[nodiscard]] inline
f32 length(v2 a)
{
    f32 result = math::sqrt(length_squared(a));
    return result;
}

inline
void normalize(v2& a)
{
    f32 n = length(a);
    if (n > 0) {
        a.x /= n;
        a.y /= n;
    }
}

inline v2 normalized(v2 a) {
    v2 result = a;
    normalize(result);
    return result;
}

inline v2 clamp(v2 a, f32 min, f32 max) {
    v2 result{ math::clamp(a.x, min, max), math::clamp(a.y, min, max) };
    return result;
}

inline v2 lerp (v2 a, v2 b, f32 t) {
    v2 result = v2{ math::lerp(a.x, b.x, t), math::lerp(a.y, b.y, t) };
    return result;
}

inline v2i round_to_v2i(v2 v) {
    v2i result = v2i{ math::round_to_int32(v.x), math::round_to_int32(v.y) };
    return result;
}

inline v2i truncate_to_v2i(v2 v)
{
    v2i result = v2i{ math::truncate_to_int32(v.x), math::truncate_to_int32(v.y) };
    return result;
}

inline v2 upcast_to_v2(v2i v) {
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
inline v2 project(v2 a, v2 b) {
    v2 result = b * dot(a, b) / length_squared(b);
    return result;
}

//
// Projects 'a' onto line with normal 'norm'
//
inline v2 project_normal(v2 a, v2 norm) {
    v2 result = a - dot(a, norm) * norm;
    return result;
}


inline f32 projection(v2 a, v2 b) {
    f32 result = dot(a, b) / length(b);
    return result;
}

struct intersection_result {
    intersection_type found;
    v2 intersection;
};

inline
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

inline
intersection_result segment_segment_intersection(v2 p0, v2 p1, v2 q0, v2 q1) {
    intersection_result result {};

    v2 r = p1 - p0;
    v2 s = q1 - q0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((q0.x - p0.x) * s.y - (q0.y - p0.y) * s.x);

    if (math::absolute(denom) < EPSILON && math::absolute(nom) < EPSILON) {
        result = { INTERSECTION_COLLINEAR, v2::nan() };
        return result;
    }

    if (math::absolute(denom) < EPSILON && math::absolute(nom) > EPSILON) {
        result = { INTERSECTION_PARALLEL, v2::nan() };
        return result;
    }

    f32 t = nom / denom;
    v2 intersection = p0 + t * r;

    // t < 0 checks for  X<-p0-->p1 situation
    // t > 0 requires us to check whether it's p0-->p1->X situation
    if ((t + EPSILON < 0) || (t > 0 && length_squared(intersection - p0) - EPSILON > length_squared(r))) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    // dot(intersection - q0, q1 - q0) < 0           checks for X<-q0-->q1 situation
    f32 vector_alignment = dot(intersection - q0, q1 - q0);
    // (intersection - q0).length_2() > s.length_2() checks for q0-->q1->X situation
    f32 vector_iq0_length = length_squared(intersection - q0);
    f32 vector_s_length = length_squared(s);
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

v2i V2I(i32 x, i32 y)
{
    v2i result { x, y };
    return result;
}


#endif // ASUKA_COMMON_MATH_VECTOR2_HPP
