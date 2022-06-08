#pragma once

#include <defines.hpp>
#include <byte.hpp>


namespace Asuka {

//
// @note: Arrays are non-owners of data. They are just spans of values (copyable, shares data between instances).
// You should free in manually via your allocator, which allocated buffer of the array.
//

template <typename T>
struct array {
    T *data;
    usize size;

    STATIC
    array<T> from_cstr(const char *s) {
        static_assert(type::is_same<T, char>::value);

        array<T> result;
        result.data = (char *) s;
        result.size = cstring::size_no0(s);

        return result;
    }
};

using byte_array = array<memory::byte>;

template <typename T>
b32 operator == (array<T> lhs, array<T> rhs) {
    b32 same = lhs.size == rhs.size;
    for (usize i = 0; same && (i < lhs.size); i++) {
        if (lhs.data[i] != rhs.data[i]) same = false;
    }

    return same;
}

template <typename T>
b32 operator != (array<T> lhs, array<T> rhs) {
    b32 same = (lhs == rhs);
    return !same;
}


template <typename T, typename Allocator>
array<T> allocate(Allocator allocator, usize size) {
    array<T> result {};
    result.data = (T *) memory::allocate(allocator, size, alignof(T));
    result.size = size;

    return result;
}

} // namespace Asuka
