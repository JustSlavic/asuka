#ifndef ASUKA_COMMON_MATH_RECTANGLE_HPP
#define ASUKA_COMMON_MATH_RECTANGLE_HPP

#include <defines.hpp>
#include <math/vector2.hpp>


//
// Axis aligned rectangle.
//
struct rectangle2
{
    v2 min;
    v2 max;

    STATIC
    rectangle2 from_min_max(v2 min, v2 max)
    {
        rectangle2 result;
        result.min = min;
        result.max = max;
        return result;
    }

    STATIC
    rectangle2 from_center_half_dim(v2 center, v2 half_dimensions)
    {
        rectangle2 result;
        result.min = center - half_dimensions;
        result.max = center + half_dimensions;
        return result;
    }

    STATIC
    rectangle2 from_center_dim(v2 center, v2 dimensions)
    {
        rectangle2 result = from_center_half_dim(center, 0.5f * dimensions);
        return result;
    }
};


using rect2 = rectangle2;


INLINE
f32 get_width(rectangle2 rect)
{
    f32 result = rect.max.x - rect.min.x;
    return result;
}

INLINE
f32 get_height(rectangle2 rect)
{
    f32 result = rect.max.y - rect.min.y;
    return result;
}

INLINE
v2 get_center(rectangle2 rect)
{
    v2 result = 0.5f * (rect.min + rect.max);
    return result;
}

INLINE
v2 get_dimensions(rectangle2 rect)
{
    v2 result = rect.max - rect.min;
    return result;
}

INLINE
b32 in_rectangle(rectangle2 rect, v2 v)
{
    b32 result = ((v.x >= rect.min.x) &&
                  (v.x <= rect.max.x) &&
                  (v.y >= rect.min.y) &&
                  (v.y <= rect.max.y));
    return result;
}


//
// Axis aligned parallelepiped
//
struct rectangle3
{
    v3 min;
    v3 max;

    STATIC
    rectangle3 from_min_max(v3 min, v3 max)
    {
        rectangle3 result;
        result.min = min;
        result.max = max;
        return result;
    }

    STATIC
    rectangle3 from_center_half_dim(v3 center, v3 half_dimensions)
    {
        rectangle3 result;
        result.min = center - half_dimensions;
        result.max = center + half_dimensions;
        return result;
    }

    STATIC
    rectangle3 from_center_dim(v3 center, v3 dimensions)
    {
        rectangle3 result = from_center_half_dim(center, 0.5f * dimensions);
        return result;
    }
};

typedef rectangle3 rect3;


INLINE
f32 get_side_x(rectangle3 rect)
{
    f32 result = rect.max.x - rect.min.x;
    return result;
}

INLINE
f32 get_side_y(rectangle3 rect)
{
    f32 result = rect.max.y - rect.min.y;
    return result;
}

INLINE
f32 get_side_z(rectangle3 rect)
{
    f32 result = rect.max.z - rect.min.z;
    return result;
}

INLINE
v3 get_center(rectangle3 rect)
{
    v3 result = 0.5f * (rect.min + rect.max);
    return result;
}

INLINE
v3 get_dimensions(rectangle3 rect)
{
    v3 result = rect.max - rect.min;
    return result;
}

INLINE
b32 in_rectangle(rectangle3 rect, v3 v)
{
    b32 result = ((v.x >= rect.min.x) &&
                  (v.x <= rect.max.x) &&
                  (v.y >= rect.min.y) &&
                  (v.y <= rect.max.y) &&
                  (v.z >= rect.min.z) &&
                  (v.z <= rect.max.z));
    return result;
}


#endif // ASUKA_COMMON_MATH_RECTANGLE_HPP
