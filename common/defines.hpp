#ifndef ASUKA_COMMON_DEFINES_HPP
#define ASUKA_COMMON_DEFINES_HPP

#ifdef _MSC_VER
#define ASUKA_COMPILER_MICROSOFT
#elif __GNUC__
#define ASUKA_COMPILER_GNU
#elif __clang__
#define ASUKA_COMPILER_CLANG
#elif __MINGW32__
#define ASUKA_COMPILER_MINGW
#endif

#ifdef ASUKA_COMPILER_MICROSOFT

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

#define ASUKA_DEBUG_BREAK __debugbreak

#if defined(ASUKA_DLL_BUILD)
#define ASUKA_DLL_EXPORT  __declspec(dllexport)
#else
#define ASUKA_DLL_EXPORT
#endif

#endif // ASUKA_COMPILER_MICROSOFT

#ifdef ASUKA_COMPILER_GNU

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

typedef unsigned char      int8;
typedef unsigned short     int16;
typedef unsigned int       int32;
typedef unsigned long long int64;

#define ASUKA_DEBUG_BREAK __builtin_trap

#if defined(ASUKA_DLL_BUILD)
#define ASUKA_DLL_EXPORT  __attribute__((dllexport))
#else
#define ASUKA_DLL_EXPORT
#endif

#endif // ASUKA_COMPILER_GNU

#ifdef ASUKA_DEBUG
#define ASSERT(COND) \
    if (COND) {} else { ASUKA_DEBUG_BREAK(); } void(0)
#define ASSERT_MSG(COND, MSG) \
    if (COND) {} else { ASUKA_DEBUG_BREAK(); } void(0)
#define ASSERT_FAIL(MSG) \
    ASUKA_DEBUG_BREAK(); void(0)
#else // ASUKA_DEBUG
#define ASSERT(COND)
#define ASSERT_MSG(COND, MSG)
#endif // ASUKA_DEBUG

#ifdef ASUKA_OS_LINUX
#endif // ASUKA_OS_LINUX

#ifdef ASUKA_OS_MACOS
#endif // ASUKA_OS_MACOS

#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define KILOBYTES(VALUE) (((uint64)(VALUE))*1024)
#define MEGABYTES(VALUE) (KILOBYTES((uint64)(VALUE))*1024)
#define GIGABYTES(VALUE) (MEGABYTES((uint64)(VALUE))*1024)
#define TERABYTES(VALUE) (GIGABYTES((uint64)(VALUE))*1024)

#define INT8_MIN (0x80)
#define INT8_MAX (0x7F)

#define UINT8_MIN (0)
#define UINT8_MAX (0xFF)

#define INT16_MIN (0x8000)
#define INT16_MAX (0x7FFF)

#define UINT16_MIN (0)
#define UINT16_MAX (0xFFFF)

#define INT32_MIN (0x80000000)
#define INT32_MAX (0x7FFFFFFF)

#define UINT32_MIN (0)
#define UINT32_MAX (0xFFFFFFFF)

#define UINT64_MIN (0)
#define UINT64_MAX (0xFFFFFFFFFFFFFFFF)


typedef int32  bool32;
typedef float  float32;
typedef float  f32;
typedef double float64;
typedef double f64;

#endif // ASUKA_COMMON_DEFINES_HPP
