#pragma once

#include <defines.hpp>
#include <byte.hpp>
#include <type.hpp>
#include <allocator.hpp>


//
// @note: Arrays are non-owners of data. They are just spans of values (when copied, data shares between instances).
// You should free it manually via your allocator, which allocated buffer of the array.
//

//
//                Arrays
//
// Arrays are lorem ipsum sir dor amet
// Lorem ipsum Lorem ipsum
//
// array = allocate_array_<T>(allocator, capacity);
// array = allocate_array<T>(allocator, capacity);
// reallocate_array(allocator, array, new_count);
// deallocate_array(allocator, array);
//


template <typename T>
struct array
{
    T *data;
    usize size;
    usize capacity;

    constexpr T* get_data() { return data; }
    constexpr T const* get_data() const { return data; }
    constexpr usize get_size() const { return size; }
    constexpr bool is_empty() const { return (size == 0); }
    constexpr bool is_valid() const { return (data != 0) && (size != 0); }

    T& operator [] (usize index)
    {
        ASSERT_MSG(index < capacity, "Attempt to access array memory out of bounds.");

        if (size < index + 1)
        {
            size = index + 1;
        }

        return data[index];
    }

    T const& operator [] (usize index) const
    {
        ASSERT_MSG(index < size, "Attempt to access array memory out of bounds.");
        return data[index];
    }
};

using byte_array = array<memory::byte>;

// ========================== ARRAY ==================================

template <typename T, typename Allocator>
array<T> allocate_array_(Allocator *allocator, usize count)
{
    T *buffer = ALLOCATE_BUFFER_(allocator, T, count);
    ASSERT(buffer);

    array<T> result = {};

    result.data = buffer;
    result.size = 0;
    result.capacity = count;

    return result;
}

template <typename T, typename Allocator>
array<T> allocate_array(Allocator *allocator, usize count)
{
    T *buffer = ALLOCATE_BUFFER(allocator, T, count);
    ASSERT(buffer);

    array<T> result = {};

    result.data = buffer;
    result.size = 0;
    result.capacity = count;

    return result;
}

template <typename T, typename Allocator>
void deallocate_array(Allocator *allocator, array<T>& a)
{
    DEALLOCATE(allocator, a.data);
    a.data = NULL;
    a.size = 0;
    a.capacity = 0;
}

template <typename T, typename Allocator>
array<T> copy_array(Allocator *allocator, array<T> source)
{
    array<T> result = allocate_array_<T>(allocator, source.capacity);
    memory::copy(result.data, source.data, source.size * sizeof(T));
    result.size = source.size;
    result.capacity = source.capacity;
    return result;
}

// ============================= STRINGS ===============================

namespace cstring {

usize size_no0(const char *str)
{
    usize size = 0;
    while (str[size]) { size += 1; }

    return size;
}

usize size_with0(const char *s)
{
    usize size = size_no0(s) + 1;
    return size;
}

} // namespacec cstring

// @todo: Utf8 support
struct string
{
    char *data;
    usize size;

    bool is_empty() const { return (size == 0); }
    bool is_valid() const { return (data != 0) && (size != 0); }

    template <typename Allocator> STATIC
    string copy_from(Allocator *allocator, char const *s)
    {
        string result;
        result.size = cstring::size_with0(s); // @todo: or without 0 ?
        result.data = ALLOCATE_BUFFER_(allocator, char, result.size);
        memory::copy(result.data, s, result.size);

        return result;
    }
};

template <typename Allocator>
void deallocate_string(Allocator *allocator, string& s)
{
    deallocate_array(allocator, s);
}

template <typename Allocator>
string copy_string(Allocator *allocator, string source)
{
    string result = allocate_string_(allocator, source.capacity);
    memory::copy(result.data, source.data, source.size);
    result.size = source.size;
    result.capacity = source.capacity;
    return result;
}

struct string_view
{
    char const *data;
    usize size;

    string_view(string const& s)
    {
        data = s.data;
        size = s.size;
    }

    string_view(char const *s)
    {
        data = s;
        size = cstring::size_no0(s);
    }

    bool is_empty() const { return (size == 0); }
    bool is_valid() const { return (data != 0) && (size != 0); }
};

bool operator == (string_view left, string_view right)
{
    if (left.size != right.size)
    {
        return false;
    }
    for (usize index = 0; index < left.size; index++)
    {
        if (left.data[index] != right.data[index]) return false;
    }
    return true;
}

// ============================================================

template <typename Allocator>
string to_string(Allocator *allocator, int32 n)
{
    string result = allocate_string(allocator, 12);

    if (n < 0)
    {
        result.push('-');
        n = -n;
    }

    int32 d = 1000000000;
    while (d > 0)
    {
        char c = (char) (n / d);
        if (c > 0)
        {
            result.push('0' + c);
        }
        n %= d;
        d /= 10;
    }

    return result;
}

// ============================================================

template <typename T>
b32 operator == (array<T> lhs, array<T> rhs)
{
    b32 same = (lhs.get_size() == rhs.get_size());
    for (usize i = 0; same && (i < lhs.get_size()); i++)
    {
        if (lhs[i] != rhs[i]) same = false;
    }

    return same;
}

bool operator == (char const *left, string right)
{
    return (right == left);
}

bool operator != (string left, char const *right)
{
    return !(left == right);
}

bool operator != (char const *left, string right)
{
    return !(right == left);
}

template <typename T>
b32 operator != (array<T> lhs, array<T> rhs)
{
    b32 same = (lhs == rhs);
    return !same;
}

template <typename T, typename Allocator>
array<T> make_copy(Allocator *allocator, array<T> source)
{
    array<T> result = copy_array(allocator, source);
    return result;
}

#define STRING_PRINT_(STRING) (int) STRING.size, STRING.data
