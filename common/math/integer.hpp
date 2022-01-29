#pragma once

#include <defines.hpp>


uint8 swap_endiannes(uint8 n) {
    return n;
}

uint16 swap_endiannes(uint16 n) {
    uint16 result = ((n & 0xFF00) >> 8) |
                    ((n & 0x00FF) << 8);
    return result;
}

uint32 swap_endiannes(uint32 n) {
    uint32 result = ((n & 0xFF000000) >> 24) |
                    ((n & 0x00FF0000) >>  8) |
                    ((n & 0x0000FF00) <<  8) |
                    ((n & 0x000000FF) << 24);
    return result;
}

uint64 swap_endiannes(uint64 n) {
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

