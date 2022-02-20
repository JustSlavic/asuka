#ifndef ASUKA_COMMON_MATH_VECTOR3_HPP
#define ASUKA_COMMON_MATH_VECTOR3_HPP

#include <defines.hpp>
#include "float.hpp"
#include "vector2.hpp"


namespace math {


struct vector3 {
    union {
        struct { float32 x, y, z; };
        struct { vector2 xy; float32 dummy_z; };
        float32 coordinates[3];
    };

    inline float32 length_2   () { return x*x + y*y + z*z; }
    inline float32 length     () { return sqrt(length_2()); }
    inline float32 norm       () { return length(); }
    inline vector3 normalized () {
        vector3 result {};

        float32 n = norm();
        if (n > 0) {
            result = vector3{ x / n, y / n, z / n };
        }

        return result;
    }

    inline vector3& operator += (vector3 other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    inline vector3& operator -= (vector3 other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    inline vector3& operator *= (float32 c) {
        x *= c;
        y *= c;
        z *= c;
        return *this;
    }

    IN_CLASS_FUNCTION
    constexpr vector3 nan() {
        vector3 result { NaN, NaN };
        return result;
    }

    IN_CLASS_FUNCTION
    constexpr vector3 zero() {
        vector3 result { 0.0f, 0.0f, 0.0f };
        return result;
    }
};


typedef vector3 vec3;
typedef vector3 v3;


inline bool32 is_valid(vector3 a) {
    bool32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z);
    return valid;
}

inline vector3 operator - (vector3 a) {
    vector3 result = vector3{ -a.x, -a.y, -a.z };
    return result;
}

inline vector3 operator + (vector3 a, vector3 b) {
    vector3 result = vector3{ a.x + b.x, a.y + b.y, a.z + b.z };
    return result;
}

inline vector3 operator - (vector3 a, vector3 b) {
    vector3 result = vector3{ a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

inline vector3 operator * (vector3 a, f32 c) {
    vector3 result = vector3{ c * a.x, c * a.y, c * a.z };
    return result;
}

inline vector3 operator * (f32 c, vector3 a) {
    vector3 result = vector3{ c * a.x, c * a.y, c * a.z };
    return result;
}

inline vector3 operator / (vector3 a, f32 c) {
    vector3 result = vector3{ a.x / c, a.y / c, a.z / c };
    return result;
}

inline bool operator == (vector3 a, vector3 b) {
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
    return result;
}

inline bool operator != (vector3 a, vector3 b) {
    bool result = !(a == b);
    return result;
}

inline f32 dot (vector3 a, vector3 b) {
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

inline vector3 clamp(vector3 a, f32 min, f32 max) {
    vector3 result{ clamp(a.x, min, max), clamp(a.y, min, max), clamp(a.z, min, max) };
    return result;
}

inline vector3 lerp (vector3 a, vector3 b, f32 t) {
    vector3 result = vector3{ lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t) };
    return result;
}


} // namespace math


using V3 = math::vector3;


V3 make_v3(F32 x, F32 y, F32 z) {
    V3 result { x, y, z };
    return result;
}


#endif // ASUKA_COMMON_MATH_VECTOR3_HPP
