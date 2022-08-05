#pragma once

#include <defines.hpp>


namespace type {

template <bool Condition, typename IfTrue, typename IfFalse>
struct sfinae_if
{};

template <typename IfTrue, typename IfFalse>
struct sfinae_if<true, IfTrue, IfFalse>
{
    static const b32 value = true;
    using type = IfTrue;
};

template <typename IfTrue, typename IfFalse>
struct sfinae_if<false, IfTrue, IfFalse>
{
    static const b32 value = false;
    using type = IfTrue;
};

template <typename LHS, typename RHS>
struct is_same {
    static const b32 value = false;
};

template <typename T>
struct is_same<T, T> {
    using type = T;
    static const b32 value = true;
};

template <typename T>
struct is_integral
{
    using type = T;
    static const bool value = false;
};

template <>
struct is_integral<int8>
{
    using type = int8;
    static const bool value = true;
};

template <>
struct is_integral<int16>
{
    using type = int16;
    static const bool value = true;
};

template <>
struct is_integral<int32>
{
    using type = int32;
    static const bool value = true;
};

template <>
struct is_integral<int64>
{
    using type = int64;
    static const bool value = true;
};

template <>
struct is_integral<uint8>
{
    using type = uint8;
    static const bool value = true;
};

template <>
struct is_integral<uint16>
{
    using type = uint16;
    static const bool value = true;
};

template <>
struct is_integral<uint32>
{
    using type = uint32;
    static const bool value = true;
};

template <>
struct is_integral<uint64>
{
    using type = uint64;
    static const bool value = true;
};

template <typename T>
struct is_floating
{
    using type = T;
    static const bool value = false;
};

template <>
struct is_floating<float32>
{
    using type = float32;
    static const bool value = true;
};

template <>
struct is_floating<float64>
{
    using type = float64;
    static const bool value = true;
};

template <typename T>
struct is_lvalue_reference
{
    using type = T;
    static constexpr bool value = false;
};

template <typename T>
struct is_lvalue_reference<T&>
{
    using type = T&;
    static constexpr bool value = true;
};

template <typename T>
struct is_rvalue_reference
{
    using type = T;
    static constexpr bool value = false;
};

template <typename T>
struct is_rvalue_reference<T&&>
{
    using type = T&&;
    static constexpr bool value = true;
};

template <typename T>
struct remove_reference
{
    using type = T;
};

template <typename T>
struct remove_reference<T&>
{
    using type = T;
};

template <typename T>
struct remove_reference<T&&>
{
    using type = T;
};

template <typename T>
typename remove_reference<T>::type&& move(T&& t)
{
    return static_cast<typename remove_reference<T>::type&&>(t);
}

template <typename T>
T&& forward(typename remove_reference<T>::type& t)
{
    return static_cast<T&&>(t);
}

template <typename T>
T&& forward(typename remove_reference<T>::type&& t)
{
    static_assert(!is_lvalue_reference<T>::value, "forward must not be used to convert an rvalue to an lvalue");
    return static_cast<T&&>(t);
}

} // namespace type
