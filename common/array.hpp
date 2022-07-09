#pragma once

#include <defines.hpp>
#include <byte.hpp>
#include <type.hpp>

//
// @note: Arrays are non-owners of data. They are just spans of values (when copied, data shares between instances).
// You should free it manually via your allocator, which allocated buffer of the array.
//


template <typename T, usize Size>
struct static_array
{
    T data[Size];

    T & operator [] (int32 index)
    {
        ASSERT_MSG(index < Size, "Attempt to access memory out of bounds.");

        T & result = data[index];
        return result;
    }
};


template <typename T>
struct array {
    T *data;
    usize size;

    T & operator [] (int32 index)
    {
        ASSERT_MSG(index < size, "Attempt to access memory out of bounds.");

        T & result = data[index];
        return result;
    }

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
using byte_string = array<memory::byte>;
using string = array<char>;

// @todo: Utf8 support
// using utf8_string = array<utf8_char>;


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


string make_string(byte_array array)
{
    string result = {};
    result.data = (char *) array.data;
    result.size = array.size;

    return result;
}


template <typename T>
b32 operator == (array<T> lhs, array<T> rhs)
{
    b32 same = (lhs.size == rhs.size);
    for (usize i = 0; same && (i < lhs.size); i++)
    {
        if (lhs.data[i] != rhs.data[i]) same = false;
    }

    return same;
}

template <typename T>
b32 operator != (array<T> lhs, array<T> rhs)
{
    b32 same = (lhs == rhs);
    return !same;
}


template <typename T, typename Allocator>
array<T> allocate_array_(Allocator *allocator, usize count)
{
    array<T> result = {};
    result.data = (T *) memory::allocate_(allocator, sizeof(T)*count, alignof(T));
    result.size = sizeof(T)*count;

    return result;
}

template <typename T, typename Allocator>
array<T> allocate_array(Allocator *allocator, usize count)
{
    array<T> result = {};
    result.data = memory::allocate(allocator, sizeof(T)*count, alignof(T));
    result.size = sizeof(T)*count;
    
    return result;
}

template <typename Allocator>
string allocate_string__(Allocator allocator, usize count)
{
    static_assert(type::is_same<array<char>, string>::value);
    string result = allocate_array__<char>(allocator, count);
    return result;
}

#include "string.hpp"
