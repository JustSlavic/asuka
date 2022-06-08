#ifndef ASUKA_COMMON_MATH_COMPLEX_HPP
#define ASUKA_COMMON_MATH_COMPLEX_HPP

#include <defines.hpp>
#include "vector2.hpp"


namespace Asuka {


struct complex {
    union {
        struct { f32 x, y; };
        struct { f32 real, imaginary; };
        struct { f32 re, im; };
    };

    operator v2()
    {
        v2 result { re, im };
        return result;
    }
    static complex i;
};


[[nodiscard]] inline
complex operator + (complex a, complex b)
{
    complex result { a.re + b.re, a.im + b.im };
    return result;
}

[[nodiscard]] inline
complex operator - (complex a, complex b)
{
    complex result { a.re - b.re, a.im - b.im };
    return result;
}

[[nodiscard]] inline
complex operator * (complex a, complex b)
{
    complex result { a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re };
    return result;
}

} // namespace Asuka

#endif // ASUKA_COMMON_MATH_COMPLEX_HPP
