#pragma once


namespace type {

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
