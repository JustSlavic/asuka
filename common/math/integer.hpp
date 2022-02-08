#pragma once

#include <defines.hpp>


uint8 change_endiannes(uint8 n) {
    return n;
}

uint16 change_endiannes(uint16 n) {
    uint16 result = ((n & 0xFF00) >> 8) |
                    ((n & 0x00FF) << 8);
    return result;
}

uint32 change_endiannes(uint32 n) {
    uint32 result = ((n & 0xFF000000) >> 24) |
                    ((n & 0x00FF0000) >>  8) |
                    ((n & 0x0000FF00) <<  8) |
                    ((n & 0x000000FF) << 24);
    return result;
}

uint64 change_endiannes(uint64 n) {
    uint64 result = ((n & 0xFF00000000000000) >> 56) |
                    ((n & 0x00FF000000000000) >> 40) |
                    ((n & 0x0000FF0000000000) >> 24) |
                    ((n & 0x000000FF00000000) >>  8) |
                    ((n & 0x00000000FF000000) <<  8) |
                    ((n & 0x0000000000FF0000) << 24) |
                    ((n & 0x000000000000FF00) << 40) |
                    ((n & 0x00000000000000FF) << 56);
    return result;
}

uint8 to_little_endian(uint8 n) {
    return n;
}

uint16 to_little_endian(uint16 n) {
#if LITTLE_ENDIAN
    uint16 result = n;
#endif
#if BIG_ENDIAN
    uint16 result = change_endiannes(n);
#endif
    return result;
}

uint32 to_little_endian(uint32 n) {
#if LITTLE_ENDIAN
    uint32 result = n;
#endif
#if BIG_ENDIAN
    uint32 result = change_endiannes(n);
#endif
    return result;
}

uint64 to_little_endian(uint64 n) {
#if LITTLE_ENDIAN
    uint64 result = n;
#endif
#if BIG_ENDIAN
    uint64 result = change_endiannes(n);
#endif
    return result;
}


uint8 to_big_endian(uint8 n) {
    uint8 result = n;
    return result;
}

uint16 to_big_endian(uint16 n) {
#if LITTLE_ENDIAN
    uint16 result = change_endiannes(n);
#endif
#if BIG_ENDIAN
    uint16 result = n;
#endif
    return result;
}

uint32 to_big_endian(uint32 n) {
#if LITTLE_ENDIAN
    uint32 result = change_endiannes(n);
#endif
#if BIG_ENDIAN
    uint32 result = n;
#endif
    return result;
}

uint64 to_big_endian(uint64 n) {
#if LITTLE_ENDIAN
    uint64 result = change_endiannes(n);
#endif
#if BIG_ENDIAN
    uint64 result = n;
#endif
    return result;
}

