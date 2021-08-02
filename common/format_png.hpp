#ifndef ASUKA_COMMON_FORMAT_PNG_HPP
#define ASUKA_COMMON_FORMAT_PNG_HPP


void* load_png_file(const char* filename);


#ifdef UNITY_BUILD
#include "format_png.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_FORMAT_PNG_HPP
