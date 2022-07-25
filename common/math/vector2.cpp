#include "vector2.hpp"


b32 is_valid(vector2 a)
{
    b32 valid = is_valid(a.x) && is_valid(a.y);
    return valid;
}

b32 is_zero(vector2 a, f32 eps)
{
    b32 result = is_zero(a.x, eps) && is_zero(a.y, eps);
    return result;
}

b32 is_equal(vector2 a, vector2 b, f32 eps)
{
    b32 result = is_equal(a.x, b.x, eps) && is_equal(a.y, b.y, eps);
    return result;
}

vector2 make_vector2 (vector2i v)
{
    vector2 result = { (f32) v.x, (f32) v.y };
    return result;
}

vector2i make_vector2i (i32 x, i32 y)
{
    vector2i result = { x, y };
    return result;
}

vector2i round_to_v2i (vector2 v)
{
    vector2i result = { round_to_i32(v.x), round_to_i32(v.y) };
    return result;
}

vector2i truncate_to_v2i (vector2 v)
{
    vector2i result = { truncate_to_i32(v.x), truncate_to_i32(v.y) };
    return result;
}

f32 dot (vector2 a, vector2 b)
{
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

f32 length2 (vector2 a)
{
    f32 result = dot(a, a);
    return result;
}

f32 length (vector2 a)
{
    f32 result = square_root(length2(a));
    return result;
}

vector2 hadamard (vector2 a, vector2 b)
{
    vector2 result = { a.x * b.x, a.y * b.y };
    return result;
}

vector2 hadamard_div (vector2 a, vector2 b)
{
    vector2 result = { a.x / b.x, a.y / b.y };
    return result;
}

void normalize (vector2 &a)
{
    f32 n = length(a);
    if (n > 0) {
        a.x /= n;
        a.y /= n;
    }
}

vector2 normalized (vector2 a) {
    vector2 result = a;
    normalize(result);
    return result;
}

vector2 clamp (vector2 a, f32 min, f32 max)
{
    vector2 result = { clamp(a.x, min, max), clamp(a.y, min, max) };
    return result;
}

vector2 lerp (vector2 a, vector2 b, f32 t)
{
    vector2 result = { lerp(a.x, b.x, t), lerp(a.y, b.y, t) };
    return result;
}

v2 project(v2 a, v2 b)
{
    v2 result = b * dot(a, b) / length2(b);
    return result;
}

v2 project_normal(v2 a, v2 norm)
{
    v2 result = a - dot(a, norm) * norm;
    return result;
}

f32 projection(v2 a, v2 b)
{
    f32 result = dot(a, b) / length(b);
    return result;
}

v2 line_line_intersection(v2 r0, v2 r1, v2 s0, v2 s1)
{
    v2 result;

    v2 r = r1 - r0;
    v2 s = s1 - s0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((s0.x - r0.x) * s.y - (s0.y - r0.y) * s.x);

    f32 t = nom / denom;
    result = r0 + t * r;

    return result;
}

intersection_result segment_segment_intersection(v2 p0, v2 p1, v2 q0, v2 q1)
{
    intersection_result result {};

    v2 r = p1 - p0;
    v2 s = q1 - q0;

    f32 denom = r.x * s.y - r.y * s.x;
    f32 nom = ((q0.x - p0.x) * s.y - (q0.y - p0.y) * s.x);

    if (absolute(denom) < EPSILON && absolute(nom) < EPSILON) {
        result = { INTERSECTION_COLLINEAR, v2{ nan(), nan() } };
        return result;
    }

    if (absolute(denom) < EPSILON && absolute(nom) > EPSILON) {
        result = { INTERSECTION_PARALLEL, v2{ nan(), nan() } };
        return result;
    }

    f32 t = nom / denom;
    Vector2 intersection = p0 + t * r;

    // t < 0 checks for  X<-p0-->p1 situation
    // t > 0 requires us to check whether it's p0-->p1->X situation
    if ((t + EPSILON < 0) || (t > 0 && length2(intersection - p0) - EPSILON > length2(r))) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    // dot(intersection - q0, q1 - q0) < 0           checks for X<-q0-->q1 situation
    f32 vector_alignment = dot(intersection - q0, q1 - q0);
    // (intersection - q0).length_2() > s.length_2() checks for q0-->q1->X situation
    f32 vector_iq0_length = length2(intersection - q0);
    f32 vector_s_length = length2(s);
    if ((vector_alignment + EPSILON < 0) || (vector_iq0_length - EPSILON > vector_s_length)) {
        result = { INTERSECTION_OUT_BOUNDS, intersection };
        return result;
    }

    result = { INTERSECTION_FOUND, intersection };
    return result;
}
