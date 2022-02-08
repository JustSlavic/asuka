#ifndef ASUKA_COMMON_MATH_VECTOR2_HPP
#define ASUKA_COMMON_MATH_VECTOR2_HPP

#include <defines.hpp>
#include "float.hpp"


namespace math {

struct vector2i {
    union {
        struct { int32  x, y; };
        int32 coordinates[2];
    };

    inline int32 operator[] (int32 idx) {
        int32 result = coordinates[idx];
        return result;
    }
};


typedef vector2i vec2i;
typedef vector2i v2i;


inline vector2i operator - (vector2i a, vector2i b) {
    vector2i result = vector2i{ a.x - b.x, a.y - b.y };
    return result;
}


struct vector2 {
    union {
        struct { float32  x,  y; };
        struct { float32  u,  v; };
        float32 coordinates[2];
    };

    IN_CLASS_FUNCTION
    vector2 make(float32 v) {
        vector2 result { v, v };
        return result;
    }

    inline float32 length_2 () { return x*x + y*y; }
    inline float32 length () { return sqrt(length_2()); }
    inline float32 norm () { return length(); }
    inline vector2 normalized () {
        vector2 result {};

        float32 n = norm();
        if (n > 0) {
            result = vector2{ x / n, y / n };
        }

        return result;
    }

    inline vector2& operator += (vector2 other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    inline vector2& operator -= (vector2 other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    inline vector2& operator *= (float32 c) {
        x *= c;
        y *= c;
        return *this;
    }

    IN_CLASS_FUNCTION
    constexpr vector2 nan() {
        vector2 result { NaN, NaN };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr vector2 zero() {
        vector2 result { 0, 0 };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr vector2 one() {
        vector2 result { 1, 1 };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr vector2 ex() {
        vector2 result { 1, 0 };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr vector2 ey() {
        vector2 result { 0, 1 };
        return result;
    }
};


typedef vector2 vec2;
typedef vector2 v2;


inline bool32 is_valid(vector2 a) {
    bool32 valid = is_valid(a.x) && is_valid(a.y);
    return valid;
}

inline vector2 operator - (vector2 a) {
    vector2 result = vector2{ -a.x, -a.y };
    return result;
}

inline vector2 operator + (vector2 a, vector2 b) {
    vector2 result = vector2{ a.x + b.x, a.y + b.y };
    return result;
}

inline vector2 operator - (vector2 a, vector2 b) {
    vector2 result = vector2{ a.x - b.x, a.y - b.y };
    return result;
}

inline vector2 operator * (vector2 a, f32 c) {
    vector2 result = vector2{ c * a.x, c * a.y };
    return result;
}

inline vector2 operator * (f32 c, vector2 a) {
    vector2 result = vector2{ c * a.x, c * a.y };
    return result;
}

inline vector2 operator / (vector2 a, f32 c) {
    vector2 result = vector2{ a.x / c, a.y / c };
    return result;
}

inline bool operator == (vector2 a, vector2 b) {
    bool result = (a.x == b.x) && (a.y == b.y);
    return result;
}

inline bool operator != (vector2 a, vector2 b) {
    bool result = !(a == b);
    return result;
}

inline f32 dot (vector2 a, vector2 b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

inline vector2 clamp(vector2 a, f32 min, f32 max) {
    vector2 result{ clamp(a.x, min, max), clamp(a.y, min, max) };
    return result;
}

inline vector2 lerp (vector2 a, vector2 b, f32 t) {
    vector2 result = vector2{ lerp(a.x, b.x, t), lerp(a.y, b.y, t) };
    return result;
}

inline vector2i round_to_vector2i(vector2 v) {
    vector2i result = vector2i{ round_to_int32(v.x), round_to_int32(v.y) };
    return result;
}

inline vector2i truncate_to_vector2i(vector2 v) {
    vector2i result = vector2i{ truncate_to_int32(v.x), truncate_to_int32(v.y) };
    return result;
}

inline vector2 upcast_to_vector2(vector2i v) {
    vector2 result = vector2{ (float32) v.x, (float32) v.y };
    return result;
}

inline vector2 mul_per_axis(vector2 a, vector2 b) {
    vector2 result = vector2{ a.x * b.x, a.y * b.y };
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
inline vector2 project(vector2 a, vector2 b) {
    vector2 result = b * math::dot(a, b) / b.length_2();
    return result;
}

//
// Projects 'a' onto line with normal 'norm'
//
inline vector2 project_normal(vector2 a, vector2 norm) {
    vector2 result = a - dot(a, norm) * norm;
    return result;
}


inline float32 projection(vector2 a, vector2 b) {
    float32 result = math::dot(a, b) / b.length();
    return result;
}

struct intersection_result {
    intersection_type found;
    v2 intersection;
};

inline
vector2 line_line_intersection(vector2 r0, vector2 r1, vector2 s0, vector2 s1) {
    vector2 result;

    vector2 r = r1 - r0;
    vector2 s = s1 - s0;

    float32 denom = r.x * s.y - r.y * s.x;
    float32 nom = ((s0.x - r0.x) * s.y - (s0.y - r0.y) * s.x);

    float32 t = nom / denom;
    result = r0 + t * r;

    return result;
}

inline
intersection_result segment_segment_intersection(vector2 p0, vector2 p1, vector2 q0, vector2 q1) {
    intersection_result result {};

    vector2 r = p1 - p0;
    vector2 s = q1 - q0;

    float32 denom = r.x * s.y - r.y * s.x;
    float32 nom = ((q0.x - p0.x) * s.y - (q0.y - p0.y) * s.x);

    if (absolute(denom) < EPSILON && absolute(nom) < EPSILON) {
        result = { INTERSECTION_COLLINEAR, v2::nan() };
        return result;
    }

    if (absolute(denom) < EPSILON && absolute(nom) > EPSILON) {
        result = { INTERSECTION_PARALLEL, v2::nan() };
        return result;
    }

    float32 t = nom / denom;
    v2 intersection = p0 + t * r;

    // t < 0 checks for  X<-p0-->p1 situation
    // t > 0 requires us to check whether it's p0-->p1->X situation
    if ((t + EPSILON < 0) || (t > 0 && (intersection - p0).length_2() - EPSILON > r.length_2())) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    // dot(intersection - q0, q1 - q0) < 0           checks for X<-q0-->q1 situation
    float32 vector_alignment = dot(intersection - q0, q1 - q0);
    // (intersection - q0).length_2() > s.length_2() checks for q0-->q1->X situation
    float32 vector_iq0_length = (intersection - q0).length_2();
    float32 vector_s_length = s.length_2();
    if ((vector_alignment + EPSILON < 0) || (vector_iq0_length - EPSILON > vector_s_length)) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    result = { INTERSECTION_FOUND, intersection };
    return result;
}

} // namespace math

#endif // ASUKA_COMMON_MATH_VECTOR2_HPP
