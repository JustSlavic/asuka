#include "array.hpp"


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


namespace cstring
{

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

bool equals(char const *s1, char const *s2)
{
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

b32 equals_to_cstr(string s, char const *cstr)
{
    for (usize i = 0; i < s.get_size(); i++)
    {
        if (*cstr == 0) { return false; }
        if (s[i] != *cstr) { return false; }

        cstr += 1;
    }

    return (*cstr == 0);
}

// @note: Always be sure that c-string have null termination.
string make_string(char const *str)
{
    string result = {};

    result.data = (char *)str;
    result.size = cstring::size_no0(str);
    result.capacity = result.size;

    return result;
}


} // namespace cstring
