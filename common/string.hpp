#ifndef ASUKA_COMMON_STRING_HPP
#define ASUKA_COMMON_STRING_HPP


//
// @note: String acts like string_view (copyable, shares data between instances).
// You should free in manually via your allocator.
//
// @note: Size does not include null termination.
// @todo: Shoud I include it to be honest with the user???
//

struct string {
    uint8 *data;
    usize size;
};


bool32 equals_to_cstr(string s, char *cstr) {
    bool32 equal = true;
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
template <typename Allocator>
string allocate_string_of_size(Allocator allocator, usize size) {
    string result {};
    result.data = memory::allocate(allocator, size);
    result.size = size;

    return result;
}


// ???
template <typename Allocator>
void free_string(Allocator allocator, string s) {
    memory::free(allocator, s.data);
}


// @note: Always be sure that c-string have null termination.
string from_cstr(char *str) {
    string result {};

    usize size = 0;
    while (str[size]) size += 1;

    result.data = (uint8 *)str;
    result.size = size;

    return result;
}


namespace cstring {


bool equals(char *s1, char *s2) {
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


#endif // ASUKA_COMMON_STRING_HPP
