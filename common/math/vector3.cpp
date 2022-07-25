#include "vector3.hpp"


b32 is_valid(v3 a)
{
    b32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z);
    return valid;
}

b32 is_zero(v3 a, f32 eps)
{
    b32 result = is_zero(a.x, eps) && is_zero(a.y, eps) && is_zero(a.z, eps);
    return result;
}

b32 is_equal(v3 a, v3 b, f32 eps)
{
    b32 result = is_equal(a.x, b.x, eps) && is_equal(a.y, b.y, eps) && is_equal(a.z, b.z, eps);
    return result;
}

v3 make_vector3 (v3i v)
{
    v3 result = { (f32) v.x, (f32) v.y, (f32) v.z };
    return result;
}

v3i make_vector3i (i32 x, i32 y, i32 z)
{
    v3i result = { x, y, z };
    return result;
}

v3i round_to_vector3i (v3 v)
{
    v3i result = { round_to_i32(v.x), round_to_i32(v.y), round_to_i32(v.z) };
    return result;
}

v3i truncate_to_vector3i (v3 v)
{
    v3i result = { truncate_to_i32(v.x), truncate_to_i32(v.y), truncate_to_i32(v.z) };
    return result;
}

f32 dot (v3 a, v3 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

v3 hadamard (v3 a, v3 b)
{
    v3 result = { a.x * b.x, a.y * b.y, a.z * b.z };
    return result;
}

v3 hadamard_div (v3 a, v3 b)
{
    v3 result = { a.x / b.x, a.y / b.y, a.z / b.z };
    return result;
}

v3 clamp (v3 a, f32 min, f32 max)
{
    v3 result = { clamp(a.x, min, max), clamp(a.y, min, max), clamp(a.z, min, max) };
    return result;
}

v3 lerp (v3 a, v3 b, float32 t)
{
    v3 result = { lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t) };
    return result;
}

f32 length2 (v3 a)
{
    f32 result = dot(a, a);
    return result;
}

f32 length(v3 a)
{
    f32 result = square_root(length2(a));
    return result;
}

void normalize (v3 &a)
{
    f32 n = length(a);
    if (n > 0)
    {
        a.x /= n;
        a.y /= n;
        a.z /= n;
    }
}

v3 normalized (v3 a)
{
    Vector3 result = a;
    normalize(result);
    return result;
}
