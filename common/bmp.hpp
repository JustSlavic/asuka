#ifndef ASUKA_COMMON_BMP_HPP
#define ASUKA_COMMON_BMP_HPP

#include <defines.hpp>
#include <bitmap.hpp>


bitmap load_bmp_file(const char* filename);


#ifdef UNITY_BUILD
#include "bmp.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_BMP_HPP
