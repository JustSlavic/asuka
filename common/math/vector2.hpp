#ifndef ASUKA_COMMON_MATH_VECTOR2_HPP
#define ASUKA_COMMON_MATH_VECTOR2_HPP

#include <defines.hpp>
#include "float.hpp"


namespace math {

struct vector2i {
    union {
        struct { int32  x, y; };
    };
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
        struct { float32 _1, _2; };
        float32 at[2];
    };

    IN_CLASS_FUNCTION INLINE_FUNCTION
    vector2 from(float32 v) {
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
    vector2i result = vector2i{ round_to_i32(v.x), round_to_i32(v.y) };
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

struct intersection_result {
    bool32 found;
    v2 intersection;
};

inline intersection_result segment_segment_intersection(vector2 p0, vector2 p1, vector2 q0, vector2 q1) {
    intersection_result result {};

    vector2 r = p1 - p0;
    vector2 s = q1 - q0;

    float32 denom = r.x * s.y - r.y * s.x;
    if (absolute(denom) < ASUKA_EPS) {
        result = { false, v2::nan() };
        return result;
    }

    float32 nom = ((q0.x - p0.x) * s.y - (q0.y - p0.y) * s.x);
    if (absolute(nom) < ASUKA_EPS) {
        result = { false, v2::nan() };
        return result;
    }

    float32 t = nom / denom;
    v2 intersection = p0 + t * r;

    if ((t < 0) || (t > 0 && intersection.length_2() > r.length_2())) {
        result = { false, intersection };
        return result;
    }

    if ((dot(intersection - q0, q1 - q0) < 0) || ((intersection - q0).length_2() > s.length_2())) {
        result = { false, intersection };
        return result;
    }

    result = { true, intersection };
    return result;
}

} // namespace math

#endif // ASUKA_COMMON_MATH_VECTOR2_HPP
