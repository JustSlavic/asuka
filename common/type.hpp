#pragma once


namespace asuka {
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

} // namespace type
} // namespace asuka
