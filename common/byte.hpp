#pragma once

#include <defines.hpp>

namespace Asuka {
namespace memory {


enum byte : u8 {};

byte operator | (byte a, byte b) {
    byte result = (byte) (((u8) a) | ((u8) b));
    return result;
}


} // namespace memory
} // namespace Asuka
