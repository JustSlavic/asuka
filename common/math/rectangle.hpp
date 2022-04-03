#ifndef ASUKA_COMMON_MATH_RECTANGLE_HPP
#define ASUKA_COMMON_MATH_RECTANGLE_HPP

#include <math/vector2.hpp>

//
// Axis aligned rectangle.
//
struct Rectangle2 {
    v2 min;
    v2 max;

    IN_CLASS_FUNCTION
    Rectangle2 from_min_max(v2 min, v2 max) {
        Rectangle2 result;
        result.min = min;
        result.max = max;

        return result;
    }

    IN_CLASS_FUNCTION
    Rectangle2 from_center_half_dim(v2 center, v2 half_dimensions) {
        Rectangle2 result;
        result.min = center - half_dimensions;
        result.max = center + half_dimensions;

        return result;
    }

    IN_CLASS_FUNCTION
    Rectangle2 from_center_dim(v2 center, v2 dimensions) {
        Rectangle2 result = from_center_half_dim(center, 0.5f * dimensions);

        return result;
    }
};


typedef Rectangle2 rect2;


INLINE
f32 get_width(Rectangle2 rect) {
    f32 result = rect.max.x - rect.min.x;
    return result;
}

INLINE
f32 get_height(Rectangle2 rect) {
    f32 result = rect.max.y - rect.min.y;
    return result;
}

INLINE
v2 get_center(Rectangle2 rect) {
    v2 result = 0.5f * (rect.min + rect.max);
    return result;
}

INLINE
v2 get_dimensions(Rectangle2 rect) {
    v2 result = rect.max - rect.min;

    return result;
}

INLINE
b32 in_rectangle(Rectangle2 rect, v2 v) {
    b32 result = ((v.x >= rect.min.x) &&
                  (v.x <= rect.max.x) &&
                  (v.y >= rect.min.y) &&
                  (v.y <= rect.max.y));

    return result;
}


#endif // ASUKA_COMMON_MATH_RECTANGLE_HPP
