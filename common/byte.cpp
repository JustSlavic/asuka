#include "byte.hpp"

namespace memory
{


byte operator | (byte a, byte b)
{
    byte result = (byte) (((u8) a) | ((u8) b));
    return result;
}


}
