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
struct buffer_
{
    T *data;
    usize size;
};


template <typename T, usize Size>
struct static_array
{
    using value_t = T;

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
    using value_t = T; // For arrays
    using char_t  = T; // For strings

    T *data;
    usize size;
    usize capacity;

    constexpr T* get_data() { return data; }
    constexpr T const* get_data() const { return data; }
    constexpr usize get_size() const { return size; }
    constexpr bool is_empty() const { return (size == 0); }

    T& operator [] (usize index)
    {
        ASSERT_MSG(index + 1 < capacity, "Attempt to access array memory out of bounds.");

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

    T& push (T const& t)
    {
        ASSERT_MSG(size < capacity, "Attempt to access array memory out of bounds.");
        return data[size++] = t;
    }

    template <typename U>
    static array<T> from(array<U> s)
    {
        array<T> result;
        result.data = (T *) s.data;
        result.size = s.size * sizeof(T) / sizeof(U);
        result.capacity = s.capacity * sizeof(T) / sizeof(U);

        return result;
    }

    static array<char> from(char const* s)
    {
        static_assert(type::is_same<T, char>::value, "This from can be called only for strings.");
        array<char> result;
        result.data = (char *) s;
        result.size = cstring::size_no0(s);
        result.capacity = result.size;

        return result;
    }

    template <typename T, bool IsConst>
    struct iterator_
    {
    private:
        using ptr_t = typename type::sfinae_if<IsConst, T const*, T *>::type;
        using ref_t = typename type::sfinae_if<IsConst, T const&, T &>::type;

        ptr_t data;
        usize index;

    public:
        iterator_(ptr_t data, usize index_) : data(data), index(index_) {}
        iterator_& operator ++ () { index += 1; return *this; }
        iterator_  operator ++ (int) { iterator_ result = *this; index += 1; return result; }
        bool operator == (iterator_ other) const { return (data == other.data) && (index == other.index); }
        bool operator != (iterator_ other) const { return !(*this == other); }
        ref_t operator * () const { return data[index]; }
    };

    using iterator = iterator_<T, false>;
    using const_iterator = iterator_<T, true>;

    const_iterator cbegin() const { return const_iterator(data, 0); }
    const_iterator cend() const { return const_iterator(data, size); }
    const_iterator begin() const { return const_iterator(data, 0); }
    const_iterator end() const { return const_iterator(data, size); }
    iterator begin() { return iterator(data, 0); }
    iterator end() { return iterator(data, size); }
};

using byte_array = array<memory::byte>;
using byte_string = array<memory::byte>;
using string = array<char>;
// @todo: Utf8 support
// using utf8_string = array<utf8_char>;


template <typename T>
array<T> make_array(T *data, usize size)
{
    array<T> result = {};
    result.data = data;
    result.size = size;
    result.capacity = size;

    return result;
}

string make_string(char *data, usize size)
{
    auto result = make_array<char>(data, size);
    return result;
}

string make_string(byte_array array)
{
    string result = {};
    result.data = (char *) array.data;
    result.size = array.size;
    result.capacity = array.capacity;

    return result;
}

template <typename T>
void copy_array(array<T> *dest, array<T> source)
{
    if (dest->capacity < source.size)
    {
        return;
    }

    dest->size = source.size;
    for (usize i = 0; i < source.size; i++)
    {
        dest->data[i] = source.data[i];
    }
}

template <typename T>
byte_string to_byte_string(array<T> s) {
    byte_string result;
    result.data = (memory::byte *) s.data_;
    result.size = s.size_ * sizeof(T);
    result.capacity = s.capacity_ * sizeof(T);

    return result;
}

template <typename T>
array<T> from_byte_string(byte_string s) {
    array<T> result;
    result.data = (T *) s.data;
    result.size = s.size / sizeof(T);
    result.capacity = s.capacity / sizeof(T);

    return result;
}

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

template <typename T>
b32 operator != (array<T> lhs, array<T> rhs)
{
    b32 same = (lhs == rhs);
    return !same;
}

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

template <typename Allocator>
void deallocate_string(Allocator *allocator, string& s)
{
    deallocate_array(allocator, s);
}

template <typename T, typename Allocator>
array<T> make_copy(Allocator *allocator, array<T> source)
{
    array<T> result = allocate_array_<T>(allocator, source.size);
    copy_array(&result, source);
    return result;
}


#include "string.hpp"

#define STRING_PRINT_(STRING) (int) STRING.size, STRING.data
