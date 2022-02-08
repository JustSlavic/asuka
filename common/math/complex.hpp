#ifndef ASUKA_COMMON_MATH_COMPLEX_HPP
#define ASUKA_COMMON_MATH_COMPLEX_HPP

#include <defines.hpp>


namespace math {

struct complex {
    union {
        struct { float32 real, imaginary; };
        struct { float32 re, im; };
    };

    IN_CLASS_FUNCTION INLINE_FUNCTION
    complex from_v2(v2 v) {
        complex result { v.x, v.y };
        return result;
    }

    extern complex i;
};


} // namespace math


#endif // ASUKA_COMMON_MATH_COMPLEX_HPP
