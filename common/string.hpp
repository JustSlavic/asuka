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

namespace cstring
{

usize size_no0(const char *str);
usize size_with0(const char *s);
bool equals(char const *s1, char const *s2);
b32 equals_to_cstr(string s, char const *cstr);
// @note: Always be sure that c-string have null termination.
string make_string(char const *str);

} // namespace cstring

#endif // ASUKA_COMMON_STRING_HPP
