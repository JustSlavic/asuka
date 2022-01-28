#ifndef ASUKA_COMMON_MATH_RECTANGLE_HPP
#define ASUKA_COMMON_MATH_RECTANGLE_HPP

#include <math/vector2.hpp>


namespace math {

//
// Axis aligned rectangle.
//
struct rectangle2 {
    v2 min;
    v2 max;

    IN_CLASS_FUNCTION
    rectangle2 from_min_max(v2 min, v2 max) {
        rectangle2 result;
        result.min = min;
        result.max = max;

        return result;
    }

    IN_CLASS_FUNCTION
    rectangle2 from_center_half_dim(v2 center, v2 half_dimensions) {
        rectangle2 result;
        result.min = center - half_dimensions;
        result.max = center + half_dimensions;

        return result;
    }

    IN_CLASS_FUNCTION
    rectangle2 from_center_dim(v2 center, v2 dimensions) {
        rectangle2 result = from_center_half_dim(center, 0.5f * dimensions);

        return result;
    }
};


typedef rectangle2 rect2;


INLINE_FUNCTION
f32 get_width(rectangle2 rect) {
    f32 result = rect.max.x - rect.min.x;
    return result;
}

INLINE_FUNCTION
f32 get_height(rectangle2 rect) {
    f32 result = rect.max.y - rect.min.y;
    return result;
}

INLINE_FUNCTION
v2 get_dimensions(rectangle2 rect) {
    v2 result = rect.max - rect.min;

    return result;
}

INLINE_FUNCTION
bool32 in_rectangle(rectangle2 rect, v2 v) {
    bool32 result = ((v.x >= rect.min.x) &&
                     (v.x <  rect.max.x) &&
                     (v.y >= rect.min.y) &&
                     (v.y <  rect.max.y));

    return result;
}


} // namespace math


#endif // ASUKA_COMMON_MATH_RECTANGLE_HPP
