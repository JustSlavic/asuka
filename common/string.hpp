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


// @note: Always be sure that c-string have null termination.
string from_cstr(char *str) {
    string result {};

    usize size = 0;
    while (str[size]) size += 1;

    result.data = (uint8 *)str;
    result.size = size;

    return result;
}


#endif // ASUKA_COMMON_STRING_HPP
