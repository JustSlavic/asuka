#include "vector4.hpp"


b32 is_valid (vector4 a)
{
    b32 valid = is_valid(a.x) && is_valid(a.y) && is_valid(a.z) && is_valid(a.w);
    return valid;
}

b32 is_zero (vector4 a, f32 eps)
{
    b32 result = is_zero(a.x, eps) && is_zero(a.y, eps) && is_zero(a.z, eps) && is_zero(a.w, eps);
    return result;
}

b32 is_equal (vector4 a, vector4 b, f32 eps)
{
    b32 result = is_equal(a.x, b.x, eps) && is_equal(a.y, b.y, eps) && is_equal(a.z, b.z, eps) && is_equal(a.w, b.w, eps);
    return result;
}

vector4 make_vector4 (v4i v)
{
    vector4 result = { (f32) v.x, (f32) v.y, (f32) v.z, (f32) v.w };
    return result;
}

v4i make_vector4i (i32 x, i32 y, i32 z, i32 w)
{
    v4i result = { x, y, z, w };
    return result;
}

v4i round_to_vector4i (vector4 v)
{
    v4i result = { round_to_i32(v.x), round_to_i32(v.y), round_to_i32(v.z), round_to_i32(v.w) };
    return result;
}

v4i truncate_to_vector4i (vector4 v)
{
    v4i result = { truncate_to_i32(v.x), truncate_to_i32(v.y), truncate_to_i32(v.z), truncate_to_i32(v.w) };
    return result;
}

f32 dot (vector4 a, vector4 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}

vector4 hadamard (vector4 a, vector4 b)
{
    vector4 result = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
    return result;
}

vector4 hadamard_div (vector4 a, vector4 b)
{
    vector4 result = { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    return result;
}

vector4 clamp (vector4 a, f32 min, f32 max)
{
    vector4 result = { clamp(a.x, min, max), clamp(a.y, min, max), clamp(a.z, min, max), clamp(a.w, min, max) };
    return result;
}

vector4 lerp (vector4 a, vector4 b, float32 t)
{
    vector4 result = { lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t) };
    return result;
}

f32 length2 (vector4 a)
{
    f32 result = dot(a, a);
    return result;
}

f32 length (vector4 a)
{
    f32 result = square_root(length2(a));
    return result;
}

void normalize (vector4 &a)
{
    f32 n = length(a);
    if (n > 0)
    {
        a.x /= n;
        a.y /= n;
        a.z /= n;
        a.w /= n;
    }
}

vector4 normalized (vector4 a)
{
    Vector4 result = a;
    normalize(result);
    return result;
}

