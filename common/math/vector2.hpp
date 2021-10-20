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


inline vector2i operator - (vector2i a, vector2i b) {
    return vector2i{ a.x - b.x, a.y - b.y };
}


struct vector2 {
    union {
        struct { float32  x,  y; };
        struct { float32  u,  v; };
        struct { float32 _1, _2; };
        float32 at[2];
    };

    inline float32 length_2 () { return x*x + y*y; }
    inline float32 length () { return math::sqrt(length_2()); }
    inline float32 norm () { return length(); }
    inline vector2 normalized () { auto n = norm(); return { x / n, y / n }; }

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
};


inline vector2 operator - (vector2 a) {
    return vector2{ -a.x, -a.y };
}

inline vector2 operator + (vector2 a, vector2 b) {
    return vector2{ a.x + b.x, a.y + b.y };
}

inline vector2 operator - (vector2 a, vector2 b) {
    return vector2{ a.x - b.x, a.y - b.y };
}

inline vector2 operator * (vector2 a, f32 c) {
    return vector2{ c * a.x, c * a.y };
}

inline vector2 operator * (f32 c, vector2 a) {
    return vector2{ c * a.x, c * a.y };
}

inline vector2 operator / (vector2 a, f32 c) {
    return vector2{ a.x / c, a.y / c };
}

inline bool operator == (vector2 a, vector2 b) {
    return (a.x == b.x) && (a.y == b.y);
}

inline bool operator != (vector2 a, vector2 b) {
    return !(a == b);
}

inline f32 dot (vector2 a, vector2 b) {
    return a.x * b.x + a.y * b.y;
}

inline vector2 lerp (vector2 a, vector2 b, f32 t) {
    return { lerp(a.x, b.x, t), lerp(a.y, b.y, t) };
}


inline vector2i round_to_vector2i(vector2 v) {
    return vector2i{ round_to_i32(v.x), round_to_i32(v.y) };
}


inline vector2i truncate_to_vector2i(vector2 v) {
    return vector2i{ truncate_to_int32(v.x), truncate_to_int32(v.y) };
}


inline vector2 upcast_to_vector2(vector2i v) {
    return vector2 { (float32) v.x, (float32) v.y };
}


} // math


#endif // ASUKA_COMMON_MATH_VECTOR2_HPP
