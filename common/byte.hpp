#pragma once

#include <defines.hpp>


namespace asuka {

enum byte : u8 {};

byte operator | (byte a, byte b) {
    byte result = (byte) (((u8) a) | ((u8) b));
    return result;
}

} // namespace asuka
