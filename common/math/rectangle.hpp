#ifndef ASUKA_COMMON_MATH_RECTANGLE_HPP
#define ASUKA_COMMON_MATH_RECTANGLE_HPP

#include <defines.hpp>
#include <math/vector2.hpp>


namespace Asuka {


//
// Axis aligned rectangle.
//
struct Rectangle2
{
    Vector2 min;
    Vector2 max;

    STATIC
    Rectangle2 from_min_max(Vector2 min, Vector2 max)
    {
        Rectangle2 result;
        result.min = min;
        result.max = max;
        return result;
    }

    STATIC
    Rectangle2 from_center_half_dim(Vector2 center, Vector2 half_dimensions)
    {
        Rectangle2 result;
        result.min = center - half_dimensions;
        result.max = center + half_dimensions;
        return result;
    }

    STATIC
    Rectangle2 from_center_dim(Vector2 center, Vector2 dimensions)
    {
        Rectangle2 result = from_center_half_dim(center, 0.5f * dimensions);
        return result;
    }
};


using rect2 = Rectangle2;
using Rect2 = Rectangle2;


INLINE
f32 get_width(Rectangle2 rect)
{
    f32 result = rect.max.x - rect.min.x;
    return result;
}

INLINE
f32 get_height(Rectangle2 rect)
{
    f32 result = rect.max.y - rect.min.y;
    return result;
}

INLINE
Vector2 get_center(Rectangle2 rect)
{
    Vector2 result = 0.5f * (rect.min + rect.max);
    return result;
}

INLINE
Vector2 get_dimensions(Rectangle2 rect)
{
    Vector2 result = rect.max - rect.min;
    return result;
}

INLINE
b32 in_rectangle(Rectangle2 rect, Vector2 v)
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
struct Rectangle3
{
    Vec3F min;
    Vec3F max;

    STATIC
    Rectangle3 from_min_max(Vector3 min, Vector3 max)
    {
        Rectangle3 result;
        result.min = min;
        result.max = max;
        return result;
    }

    STATIC
    Rectangle3 from_center_half_dim(Vector3 center, Vector3 half_dimensions)
    {
        Rectangle3 result;
        result.min = center - half_dimensions;
        result.max = center + half_dimensions;
        return result;
    }

    STATIC
    Rectangle3 from_center_dim(Vector3 center, Vector3 dimensions)
    {
        Rectangle3 result = from_center_half_dim(center, 0.5f * dimensions);
        return result;
    }
};

typedef Rectangle3 rect3;


INLINE
f32 get_side_x(Rectangle3 rect)
{
    f32 result = rect.max.x - rect.min.x;
    return result;
}

INLINE
f32 get_side_y(Rectangle3 rect)
{
    f32 result = rect.max.y - rect.min.y;
    return result;
}

INLINE
f32 get_side_z(Rectangle3 rect)
{
    f32 result = rect.max.z - rect.min.z;
    return result;
}

INLINE
Vector3 get_center(Rectangle3 rect)
{
    Vector3 result = 0.5f * (rect.min + rect.max);
    return result;
}

INLINE
Vector3 get_dimensions(Rectangle3 rect)
{
    Vector3 result = rect.max - rect.min;
    return result;
}

INLINE
b32 in_rectangle(Rectangle3 rect, Vector3 v)
{
    b32 result = ((v.x >= rect.min.x) &&
                  (v.x <= rect.max.x) &&
                  (v.y >= rect.min.y) &&
                  (v.y <= rect.max.y) &&
                  (v.z >= rect.min.z) &&
                  (v.z <= rect.max.z));
    return result;
}


} // namespace Asuka


#endif // ASUKA_COMMON_MATH_RECTANGLE_HPP
