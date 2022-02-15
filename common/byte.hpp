#pragma once

#include <defines.hpp>


namespace asuka {

enum byte : uint8 {};

byte operator | (byte a, byte b) {
    byte result = (byte) (((uint8) a) | ((uint8) b));
    return result;
}

} // namespace asuka
