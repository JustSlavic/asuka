#ifndef ASUKA_COMMON_MATH_VECTOR3_HPP
#define ASUKA_COMMON_MATH_VECTOR3_HPP

#include <defines.hpp>
#include "float.hpp"
#include "vector2.hpp"


struct v3 {
    union {
        struct { f32 x, y, z; };
        struct { v2 xy; f32 dummy_z; };
        f32 coordinates[3];
    };

    IN_CLASS_FUNCTION v3 nan() {
        v3 result { math::NaN, math::NaN };
        return result;
    }

    IN_CLASS_FUNCTION v3 zero() {
        v3 result { 0.0f, 0.0f, 0.0f };
        return result;
    }

    IN_CLASS_FUNCTION v3 one() {
        v3 result { 1.0f, 1.0f, 1.0f };
    }

    IN_CLASS_FUNCTION v3 ex() {
        v3 result { 1.0f, 0.0f, 0.0f };
    }

    IN_CLASS_FUNCTION v3 ey() {
        v3 result { 0.0f, 1.0f, 0.0f };
    }

    IN_CLASS_FUNCTION v3 ez() {
        v3 result { 0.0f, 0.0f, 1.0f };
    }

    inline v3& operator += (v3 other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    inline v3& operator -= (v3 other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    inline v3& operator *= (f32 c) {
        x *= c;
        y *= c;
        z *= c;
        return *this;
    }
};


inline b32 is_valid(v3 a) {
    b32 valid = math::is_valid(a.x) && math::is_valid(a.y) && math::is_valid(a.z);
    return valid;
}

inline v3 operator - (v3 a) {
    v3 result = v3{ -a.x, -a.y, -a.z };
    return result;
}

inline v3 operator + (v3 a, v3 b) {
    v3 result = v3{ a.x + b.x, a.y + b.y, a.z + b.z };
    return result;
}

inline v3 operator - (v3 a, v3 b) {
    v3 result = v3{ a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

inline v3 operator * (v3 a, f32 c) {
    v3 result = v3{ c * a.x, c * a.y, c * a.z };
    return result;
}

inline v3 operator * (f32 c, v3 a) {
    v3 result = v3{ c * a.x, c * a.y, c * a.z };
    return result;
}

inline v3 operator / (v3 a, f32 c) {
    v3 result = v3{ a.x / c, a.y / c, a.z / c };
    return result;
}

inline bool operator == (v3 a, v3 b) {
    bool result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
    return result;
}

inline bool operator != (v3 a, v3 b) {
    bool result = !(a == b);
    return result;
}

inline f32 dot (v3 a, v3 b) {
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

inline v3 clamp(v3 a, f32 min, f32 max) {
    v3 result{ math::clamp(a.x, min, max), math::clamp(a.y, min, max), math::clamp(a.z, min, max) };
    return result;
}

inline v3 lerp (v3 a, v3 b, f32 t) {
    v3 result = v3{ math::lerp(a.x, b.x, t), math::lerp(a.y, b.y, t), math::lerp(a.z, b.z, t) };
    return result;
}

[[nodiscard]] inline
f32 length_squared(v3 a)
{
    f32 result = dot(a, a);
    return result;
}

[[nodiscard]] inline
f32 length(v3 a)
{
    f32 result = math::sqrt(length_squared(a));
    return result;
}

void normalize(v3 &a)
{
    f32 n = length(a);
    if (n > 0)
    {
        a.x /= n;
        a.y /= n;
        a.z /= n;
    }
}

[[nodiscard]]
v3 normalized(v3 a)
{
    v3 result = a;
    normalize(result);
    return result;
}

template <typename T>
v3 V3(T value)
{
    v3 result { (f32) value, (f32) value, (f32) value };
    return result;
}

template <typename X, typename Y, typename Z>
v3 V3(X x, Y y, Z z)
{
    v3 result { (f32) x, (f32) y, (f32) z };
    return result;
}


#endif // ASUKA_COMMON_MATH_VECTOR3_HPP
