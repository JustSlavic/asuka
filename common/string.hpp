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

usize size_no0(const char *str) {
    usize size = 0;
    while (str[size]) { size += 1; }

    return size;
}

usize size_with0(const char *s) {
    usize size = size_no0(s) + 1;
    return size;
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


b32 equals_to_cstr(string s, char *cstr) {
    for (usize i = 0; i < s.get_size(); i++)
    {
        if (*cstr == 0) { return false; }
        if (s[i] != *cstr) { return false; }

        cstr += 1;
    }

    return (*cstr == 0);
}


// @note: Always be sure that c-string have null termination.
string from_cstr(char const *str) {
    string result = {};

    result.data = (char *)str;
    result.size = cstring::size_no0(str);
    result.capacity = result.size;

    return result;
}

} // namespace Asuka

#endif // ASUKA_COMMON_STRING_HPP
