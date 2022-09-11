#pragma once

#include <defines.hpp>
#include <byte.hpp>
#include <type.hpp>
#include <allocator.hpp>


template <typename T, typename Allocator = memory::mallocator>
struct array
{
    T *data;
    usize size;
    usize capacity;
    Allocator *allocator;

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

    T& push(T const& t);
};

using byte_array = array<memory::byte>;

template <typename T, typename Allocator>
array<T> allocate_array_(Allocator *allocator, usize count)
{
    T *buffer = ALLOCATE_BUFFER_(allocator, T, count);
    ASSERT(buffer);

    array<T> result = {};

    result.data = buffer;
    result.size = 0;
    result.capacity = count;
    result.allocator = allocator;

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
    result.allocator = allocator;

    return result;
}

template <typename T>
void deallocate_array(array<T>& a)
{
    DEALLOCATE(a.allocator, a.data);
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

template <typename T, typename Allocator>
array<T> make_copy(Allocator *allocator, array<T> source)
{
    array<T> result = copy_array(allocator, source);
    return result;
}

template <typename T, typename Allocator>
T& array<T, Allocator>::push(T const& t)
{
    ASSERT(size <= capacity);

    if (size == capacity)
    {
        usize new_capacity = capacity * 2;
        T *new_data = REALLOCATE_BUFFER(allocator, data, new_capacity);
        if (new_data)
        {
            data = new_data;
            capacity = new_capacity;
        }
    }

    return data[size++] = t; // @todo: forward
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

struct string_view;
struct string_id;

// @todo: Utf8 support
struct string
{
    char *data;
    usize size;

    string() = default;
    string(string const&) = delete;
    string(string&& other);

    string& operator =(string const&) = delete;
    string& operator =(string&& other);

    bool is_empty() const { return (size == 0); }
    bool is_valid() const { return (data != 0) && (size != 0); }

    template <typename Allocator> STATIC
    string copy_from(Allocator *allocator, char const *s);

    template <typename Allocator> STATIC
    string copy_from(Allocator *allocator, string s);

    template <typename Allocator> STATIC
    string copy_from(Allocator *allocator, string_view s);

    STATIC
    string from(byte_array ba);

    char & operator [] (usize index)
    {
        ASSERT_MSG(index < size, "Attempt to access array memory out of bounds.");
        return data[index];
    }

    char const& operator [] (usize index) const
    {
        ASSERT_MSG(index < size, "Attempt to access array memory out of bounds.");
        return data[index];
    }
};

string::string(string&& other)
{
    data = other.data;
    size = other.size;
    other.data = 0;
    other.size = 0;
}

string& string::operator = (string&& other)
{
    data = other.data;
    size = other.size;
    other.data = 0;
    other.size = 0;
    return *this;
}

string string::from(byte_array ba)
{
    string result;
    result.data = (char *) ba.data;
    result.size = ba.size;
    return result;
}

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

// ============================================================

struct string_view
{
    char const *data;
    usize size;

    string_view();
    string_view(char const *s);
    string_view(string const& s);
    string_view(string_id const& s);

    bool is_empty() const { return (size == 0); }
    bool is_valid() const { return (data != 0) && (size != 0); }

    char operator[] (int32 index);
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

bool operator != (string_view left, string_view right)
{
    return !(left == right);
}

// ============================================================

// @todo: make something which will hold string_id buffer, and
// hash table to it, which I could access easily and fast.
// 'string_id' stores only index in hashtable (or offset in buffer?).
// Strings in the buffer always null-terminated.
// The table always grows, throught the lifetime of the program, so
// you just supposed to statically increase the table size value.

struct string_id_entry
{
    int32 offset;
    usize size;
};

struct string_id_table
{
    char *buffer;
    usize buffer_size;

    string_id_entry hash_table[256];
};

string_id_entry get_string_id_entry()
{
    // @todo
    NOT_IMPLEMENTED();
    string_id_entry entry = {};
    return entry;
}

struct string_id
{
    int32 index_in_hash;

    string_id& operator = (string_view const& s)
    {
        // @todo: search in the hash table, and if not found -- make new entry
        NOT_IMPLEMENTED();
        index_in_hash = -1;
    }
};

bool operator == (string_id left, string_id right)
{
    NOT_IMPLEMENTED();
    return (left.index_in_hash == right.index_in_hash);
}

// ============================================================

template <typename Allocator>
string string::copy_from(Allocator *allocator, char const *s)
{
    string result;
    result.size = cstring::size_with0(s);
    result.data = ALLOCATE_BUFFER_(allocator, char, result.size);
    memory::copy(result.data, s, result.size);

    return result;
}

template <typename Allocator>
string string::copy_from(Allocator *allocator, string s)
{
    string result;
    result.size = s.size;
    result.data = ALLOCATE_BUFFER_(allocator, char, result.size);
    memory::copy(result.data, s.data, result.size);

    return result;
}

template <typename Allocator>
string string::copy_from(Allocator *allocator, string_view s)
{
    string result;
    result.size = s.size + 1; // +1 for null terminator
    result.data = ALLOCATE_BUFFER_(allocator, char, result.size);
    memory::copy(result.data, s.data, result.size);
    result.data[result.size - 1] = 0; // null terminator

    return result;
}

// ============================================================

string_view::string_view()
{
    data = NULL;
    size = 0;
}

string_view::string_view(char const *s)
{
    data = s;
    size = cstring::size_no0(s);
}

string_view::string_view(string const& s)
{
    data = s.data;
    size = s.size;
}

string_view::string_view(string_id const& s)
{
    // @todo
    NOT_IMPLEMENTED();
}

char string_view::operator [] (int32 index)
{
    char c = 0;
    if (index < size)
    {
        c = data[index];
    }
    return c;
}

// ============================================================

#define STRING_PRINT_(STRING) (int) STRING.size, STRING.data
