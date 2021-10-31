#include "float.hpp"


namespace math {

INTERNAL_FUNCTION
uint32 uNaN = 0x7FC00000;

f32 pi = 3.14159265358979323846f;
f32 NaN = *(float32*)(&uNaN);

} // math
