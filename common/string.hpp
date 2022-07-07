#ifndef ASUKA_COMMON_STRING_HPP
#define ASUKA_COMMON_STRING_HPP

#include <defines.hpp>
#include <type.hpp>
#include "array.hpp"
#include "byte.hpp"

//
// @note: Strings act like string_view (copyable, shares data between instances).
// You should free in manually via your allocator.
//

namespace Asuka {

namespace cstring {

usize size_no0(const char *s) {
    usize result = 0;
    while (*s) {
        result += 1;
        s += 1;
    }

    return result;
}

usize size_with0(const char *s) {
    usize result = size_no0(s) + 1;
    return result;
}

bool equals(char const *s1, char const *s2) {
    bool equal = true;

    char c1 = *s1++;
    char c2 = *s2++;

    while ((c1 != 0) && (c2 != 0)) {
        if (c1 != c2) {
            equal = false;
            break;
        }

        c1 = *s1++;
        c2 = *s2++;
    }

    if (equal &&
        (((*s1 != 0) && (*s2 == 0)) ||
         ((*s1 == 0) && (*s2 != 0))))
    {
        equal = false;
    }

    return equal;
}

} // namespace cstring


using byte_string = array<byte>;
// using string = array<char>;
// using utf8_string = bast_string<>; // @todo: utf8 string


template <typename T>
byte_string to_byte_string(array<T> s) {
    byte_string result;
    result.data = (byte *) s.data;
    result.size = s.size * sizeof(T);

    return result;
}


template <typename T>
array<T> from_byte_string(byte_string s) {
    array<T> result;
    result.data = (T *) s.data;
    result.size = s.size / sizeof(T);

    return result;
}


b32 is_empty(string s) {
    return (s.data == 0) || (s.size == 0);
}


b32 equals_to_cstr(string s, char *cstr) {
    b32 equal = true;
    while ((s.size > 0) && (*cstr != 0)) {
        if (*s.data != *cstr) {
            equal = false;
            break;
        }

        s.data += 1;
        s.size -= 1;
        cstr += 1;
    }

    if (equal && (s.size > 0 || *cstr != 0)) {
        equal = false;
    }

    return equal;
}


// What did I do?
template <typename Allocator, typename T>
array<T> allocate_string_of_size(Allocator allocator, usize size) {
    string result {};
    result.data = memory::allocate(allocator, size, alignof(T));
    result.size = size;

    return result;
}


// ???
template <typename Allocator, typename T>
void free_string(Allocator allocator, array<T> s) {
    memory::free(allocator, s.data);
}


// @note: Always be sure that c-string have null termination.
string from_cstr(char *str) {
    string result {};

    usize size = 0;
    while (str[size]) size += 1;

    result.data = (char *)str;
    result.size = size;

    return result;
}

} // namespace Asuka

#endif // ASUKA_COMMON_STRING_HPP
