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
struct array
{
    T *data;
    usize size;

    T& operator [] (int32 index)
    {
        ASSERT_MSG(index < size, "Attempt to access memory out of bounds.");

        T & result = data[index];
        return result;
    }

    T const& operator [] (int32 index) const
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


template <typename T, typename Allocator>
struct dynamic_array
{
    T *data;
    usize size;
    usize capacity;
    Allocator *allocator;

    T& operator [] (int32 index)
    {
        ASSERT_MSG(index < size, "Attempt to access memory out of bounds.");

        T & result = data[index];
        return result;
    }

    void push(T value) // @todo: check if T&& is going to eliminate excessive copies
    {
        ASSERT(size <= capacity); // This should always be true;

        if (size == capacity)
        {
            ensure_capacity((capacity + 1) * 2);
        }

        data[size++] = value;
    }

    void reserve(usize new_capacity)
    {
        if (new_capacity > capacity)
        {
            ensure_capacity(new_capacity);
        }
    }

    void ensure_capacity(usize new_capacity)
    {
        T *new_buffer = memory::allocate_buffer_<T>(allocator, new_capacity);

        for (int32 i = 0; i < size; i++)
        {
            new_buffer[i] = data[i];
        }

        if (data)
        {
            deallocate_buffer(allocator, data);
        }

        data = new_buffer;
        capacity = new_capacity;
    }
};


template <typename T, typename Allocator>
dynamic_array<T, Allocator> make_dynamic_array(Allocator *alloc)
{
    dynamic_array<T, Allocator> result = {};
    result.allocator = alloc;

    return result;
}


template <typename T, typename Allocator>
dynamic_array<T, Allocator> make_dynamic_array(Allocator *alloc, usize size)
{
    dynamic_array<T, Allocator> result = {};
    result.allocator = alloc;

    result.data = memory::allocate_buffer_<T>(alloc, size);
    result.size = size;
    result.capacity = size;

    return result;
}


template <typename T, typename Allocator>
array<T> make_array(dynamic_array<T, Allocator> dyn_array)
{
    array<T> result = {};
    result.data = dyn_array.data;
    result.size = dyn_array.size;
    // result.capacity = dyn_array.capacity;

    return result;
}


template <typename T>
void copy_array(array<T> source, array<T> dest)
{
    if (dest.size < source.size)
    {
        return;
    }

    for (usize i = 0; i < source.size; i++)
    {
        dest[i] = source[i];
    }
}


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
    result.size = count;

    return result;
}

template <typename T, typename Allocator>
array<T> allocate_array(Allocator *allocator, usize count)
{
    array<T> result = {};
    result.data = (T *) memory::allocate(allocator, sizeof(T)*count, alignof(T));
    result.size = count;
    
    return result;
}

template <typename Allocator>
string allocate_string_(Allocator allocator, usize count)
{
    static_assert(type::is_same<array<char>, string>::value);
    string result = allocate_array_<char>(allocator, count);
    return result;
}

template <typename Allocator>
string allocate_string(Allocator allocator, usize count)
{
    static_assert(type::is_same<array<char>, string>::value);
    string result = allocate_array<char>(allocator, count);
    return result;
}

#include "string.hpp"
