#ifndef ASUKA_COMMON_DEFINES_HPP
#define ASUKA_COMMON_DEFINES_HPP


#ifdef ASUKA_OS_WINDOWS
typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

#ifdef ASUKA_DEBUG
#define ASSERT(COND) \
    if (COND) {} else { __debugbreak(); } void(0)
#define ASSERT_MSG(COND, MSG) \
    if (COND) {} else { __debugbreak(); } void(0)
#else
#define ASSERT(COND)
#define ASSERT_MSG(COND, MSG)
#endif

#endif

#ifdef ASUKA_OS_LINUX
#endif

#ifdef ASUKA_OS_MACOS
#endif

typedef float float32;
typedef double float64;

typedef float f32;
typedef double f64;


#endif // ASUKA_COMMON_DEFINES_HPP
