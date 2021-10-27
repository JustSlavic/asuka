#ifndef ASUKA_COMMON_PNG_HPP
#define ASUKA_COMMON_PNG_HPP

#include <defines.hpp>
#include <bitmap.hpp>


bitmap load_png_file(const char* filename);


#ifdef UNITY_BUILD
#include "png.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_PNG_HPP
