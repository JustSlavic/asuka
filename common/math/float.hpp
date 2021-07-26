#ifndef ASUKA_COMMON_MATH_FLOAT_HPP
#define ASUKA_COMMON_MATH_FLOAT_HPP


#include <defines.hpp>


namespace math {


template <typename T>
struct consts {};


template <> struct consts<f32> {
    using self = f32;

    static constexpr self pi () noexcept { return 3.14159265358979323846f; }
};


} // math


#endif // ASUKA_COMMON_MATH_FLOAT_HPP
