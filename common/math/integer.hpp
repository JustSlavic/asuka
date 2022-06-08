#pragma once

#include <defines.hpp>


i8 change_endiannes(i8 n) {
    return n;
}

u8 change_endiannes(u8 n) {
    return n;
}

i16 change_endiannes(i16 n) {
    i16 result = ((n & 0xFF00) >> 8) |
                 ((n & 0x00FF) << 8);
    return result;
}

u16 change_endiannes(u16 n) {
    u16 result = ((n & 0xFF00) >> 8) |
                 ((n & 0x00FF) << 8);
    return result;
}

i32 change_endiannes(i32 n) {
    i32 result = ((n & 0xFF000000) >> 24) |
                 ((n & 0x00FF0000) >>  8) |
                 ((n & 0x0000FF00) <<  8) |
                 ((n & 0x000000FF) << 24);
    return result;
}

u32 change_endiannes(u32 n) {
    u32 result = ((n & 0xFF000000) >> 24) |
                 ((n & 0x00FF0000) >>  8) |
                 ((n & 0x0000FF00) <<  8) |
                 ((n & 0x000000FF) << 24);
    return result;
}

i64 change_endiannes(i64 n) {
    i64 result = ((n & 0xFF00000000000000) >> 56) |
                 ((n & 0x00FF000000000000) >> 40) |
                 ((n & 0x0000FF0000000000) >> 24) |
                 ((n & 0x000000FF00000000) >>  8) |
                 ((n & 0x00000000FF000000) <<  8) |
                 ((n & 0x0000000000FF0000) << 24) |
                 ((n & 0x000000000000FF00) << 40) |
                 ((n & 0x00000000000000FF) << 56);
    return result;
}

u64 change_endiannes(u64 n) {
    u64 result = ((n & 0xFF00000000000000) >> 56) |
                 ((n & 0x00FF000000000000) >> 40) |
                 ((n & 0x0000FF0000000000) >> 24) |
                 ((n & 0x000000FF00000000) >>  8) |
                 ((n & 0x00000000FF000000) <<  8) |
                 ((n & 0x0000000000FF0000) << 24) |
                 ((n & 0x000000000000FF00) << 40) |
                 ((n & 0x00000000000000FF) << 56);
    return result;
}

u8 to_little_endian(u8 n) {
    return n;
}

u16 to_little_endian(u16 n) {
#if LITTLE_ENDIAN
    u16 result = n;
#endif
#if BIG_ENDIAN
    u16 result = change_endiannes(n);
#endif
    return result;
}

u32 to_little_endian(u32 n) {
#if LITTLE_ENDIAN
    u32 result = n;
#endif
#if BIG_ENDIAN
    u32 result = change_endiannes(n);
#endif
    return result;
}

u64 to_little_endian(u64 n) {
#if LITTLE_ENDIAN
    u64 result = n;
#endif
#if BIG_ENDIAN
    u64 result = change_endiannes(n);
#endif
    return result;
}


u8 to_big_endian(u8 n) {
    return n;
}

u16 to_big_endian(u16 n) {
#if LITTLE_ENDIAN
    u16 result = change_endiannes(n);
#endif
#if BIG_ENDIAN
    u16 result = n;
#endif
    return result;
}

u32 to_big_endian(u32 n) {
#if LITTLE_ENDIAN
    u32 result = change_endiannes(n);
#endif
#if BIG_ENDIAN
    u32 result = n;
#endif
    return result;
}

u64 to_big_endian(u64 n) {
#if LITTLE_ENDIAN
    u64 result = change_endiannes(n);
#endif
#if BIG_ENDIAN
    u64 result = n;
#endif
    return result;
}

