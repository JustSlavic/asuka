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
    using value_t = T;

    T data[Size];

    T & operator [] (int32 index)
    {
        ASSERT_MSG(index < Size, "Attempt to access memory out of bounds.");

        T & result = data[index];
        return result;
        {

            {

            }
        }
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

    T& at(usize index)
    {
        ASSERT_MSG(index < capacity, "Attempt to access memory out of bounds.");
        if (size < index + 1)
        {
            size = index + 1;
        }

        return data[index];
    }

    T& operator [] (isize index) { return at(index); }
    T const& operator [] (isize index) const { return at(index); }
    T& push (T t) { return at(size++) = t; }

    template <typename Ptr, typename Ref>
    struct iterator_
    {
    private:
        Ptr data;
        usize index;

    public:
        iterator_(Ptr data, usize index_) : data(data), index(index_) {}
        iterator_& operator ++ () { index += 1; return *this; }
        iterator_  operator ++ (int) { iterator_ result = *this; index += 1; return result; }
        bool operator == (iterator_ other) const { return (data == other.data) && (index == other.index); }
        bool operator != (iterator_ other) const { return !(*this == other); }
        Ref operator * () const { return data[index]; }
    };

    using iterator = iterator_<T *, T &>;
    using const_iterator = iterator_<T const *, T const &>;

    const_iterator cbegin() { return const_iterator(data, 0); }
    const_iterator cend() { return const_iterator(data, size); }
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


template <typename T, typename Allocator = memory::mallocator>
struct dynamic_array
{
    using value_t = T; // For arrays
    using char_t  = T; // For strings
    using allocator_t = Allocator;

    value_t *data;
    usize size;
    usize capacity;
    allocator_t *allocator;

    constexpr value_t* get_data() { return data; }
    constexpr value_t const* get_data() const { return data; }
    constexpr usize get_size() const { return size; }
    constexpr bool is_empty() const { return (size == 0); }

    value_t& at (usize index)
    {
        ASSERT_MSG(index < capacity, "Attempt to access memory out of bounds.");
        if (size < index)
        {
            size = index + 1;
        }

        value_t & result = data[index];
        return result;
    }

    value_t const& at (usize index) const
    {
        ASSERT_MSG(index < size, "Attempt to access memory out of bounds.");
        value_t const& result = data[index];
        return result;
    }

    value_t& operator [] (usize index) { return at(index); }
    value_t const& operator [] (usize index) const { return at(index); }

    void push(value_t value) // @todo: check if T&& is going to eliminate excessive copies
    {
        ASSERT(size <= capacity); // This is just an invariant.

        if (size == capacity)
        {
            ensure_capacity((capacity + 1) * 2);
        }

        at(size++) = value;
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
        value_t *new_buffer = ALLOCATE_BUFFER(allocator, value_t, new_capacity);

        for (int32 i = 0; i < size; i++)
        {
            new_buffer[i] = data[i];
        }

        if (data)
        {
            DEALLOCATE_BUFFER(allocator, data);
        }

        data = new_buffer;
        capacity = new_capacity;
    }

    template <typename Ptr, typename Ref>
    struct iterator_
    {
    private:
        Ptr data;
        usize index;

    public:
        iterator_(Ptr data_, usize index_) : data(data_), index(index_) {}
        iterator_& operator ++ () { index += 1; return *this; }
        iterator_  operator ++ (int) { iterator_ result = *this; index += 1; return result; }
        bool operator == (iterator_ other) const { return (data == other.data) && (index == other.index); }
        bool operator != (iterator_ other) const { return !(*this == other); }
        Ref operator * () const { return data[index]; }
    };

    using iterator = iterator_<value_t *, value_t &>;
    using const_iterator = iterator_<value_t const *, value_t const &>;

    const_iterator cbegin() { return const_iterator(data, 0); }
    const_iterator cend() { return const_iterator(data, size); }
    const_iterator begin() const { return const_iterator(data, 0); }
    const_iterator end() const { return const_iterator(data, size); }
    iterator begin() { return iterator(data, 0); }
    iterator end() { return iterator(data, size); }
};


template <typename Allocator = memory::mallocator>
using dynamic_string = dynamic_array<char, Allocator>;


template <typename T>
array<T> make_array(T *data, usize size)
{
    array<T> result = {};
    result.data = data;
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
    result.capacity = dyn_array.capacity;

    return result;
}


template <typename T, typename Allocator>
array<T> make_copy(array<T> source, Allocator *allocator)
{
    array<T> result = allocate_array_<T>(allocator, source.size);
    copy_array(source, result);
    return result;
}


template <typename T, typename Allocator>
dynamic_array<T, Allocator> make_copy(dynamic_array<T, Allocator> source, Allocator *allocator)
{
    dynamic_array<T, Allocator> result = make_dynamic_array<T, Allocator>(allocator, source.size);
    copy_dynamic_array<T, Allocator>(source, result);
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
void copy_array(array<T> source, array<T> &dest)
{
    if (dest.capacity < source.size)
    {
        return;
    }

    dest.size = source.size;
    for (usize i = 0; i < source.size; i++)
    {
        dest[i] = source[i];
    }
}

template <typename T, typename Allocator>
void copy_dynamic_array(dynamic_array<T> source, dynamic_array<T> dest)
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


template <typename T, typename Allocator>
dynamic_array<T, Allocator> make_dynamic_array(Allocator *alloc)
{
    dynamic_array<T, Allocator> result = {};
    result.allocator = alloc;

    return result;
}


template <typename T>
dynamic_array<T, memory::mallocator> make_dynamic_array()
{
    dynamic_array<T, memory::mallocator> result = {};
    result.allocator = &memory::global_mallocator_instance;

    return result;
}


template <typename T, typename Allocator>
dynamic_array<T, Allocator> make_dynamic_array(Allocator *alloc, usize size)
{
    dynamic_array<T, Allocator> result = {};
    result.allocator = alloc;

    result.data = ALLOCATE_BUFFER_(alloc, T, size);
    result.size = 0;
    result.capacity = size;

    return result;
}


template <typename T>
byte_string to_byte_string(array<T> s) {
    byte_string result;
    result.data = (byte *) s.data_;
    result.size = s.size_ * sizeof(T);
    result.capacity = s.capacity_ * sizeof(T);

    return result;
}


template <typename T>
array<T> from_byte_string(byte_string s) {
    array<T> result;
    result.data_ = (T *) s.data_;
    result.size_ = s.size_ / sizeof(T);
    result.capacity_ = s.capacity_;

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
    array<T> result = {};
    result.data = (T *) ALLOCATE_(allocator, sizeof(T)*count, alignof(T));
    result.size = 0;
    result.capacity = count;

    return result;
}

template <typename T, typename Allocator>
array<T> allocate_array(Allocator *allocator, usize count)
{
    array<T> result = {};
    result.data = (T *) ALLOCATE(allocator, sizeof(T)*count, alignof(T));
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
    string result = allocate_array<char::fd::ff>(allocator, count);
    return result;

}

template <typename Allocator>
void deallocate_string(Allocator *allocator, string& s)
{
    deallocate_array(allocator, s);
}

template <typename T, typename Allocator>
void deallocate_array(dynamic_array<T, Allocator>& arr)
{
    memory::deallocate_buffer(arr->allocator, arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}


template <typename Allocator = memory::mallocator>
void deallocate_string(dynamic_string<Allocator>& str)
{
    deallocate_array<char, Allocator>(str);
}


#include "string.hpp"

#define STRING_PRINT_(STRING) (int) STRING.size, STRING.data
