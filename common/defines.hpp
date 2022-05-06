#ifndef ASUKA_COMMON_DEFINES_HPP
#define ASUKA_COMMON_DEFINES_HPP

/*

ASUKA_DEBUG=1 - compile with:
    + with debug symbols
    + with debug utilities
    - without optimizations

ASUKA_PROFILING=1
    + include profiling stuff
    - without slow code except profiling utilities

*/

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

typedef __int8  i8;
typedef __int16 i16;
typedef __int32 i32;
typedef __int64 i64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef __int8  Int8;
typedef __int16 Int16;
typedef __int32 Int32;
typedef __int64 Int64;

typedef unsigned __int8  UInt8;
typedef unsigned __int16 UInt16;
typedef unsigned __int32 UInt32;
typedef unsigned __int64 UInt64;

typedef float  Float32;
typedef double Float64;

#define ASUKA_DEBUG_BREAK __debugbreak
#define FORCE_INLINE __forceinline

#if defined(ASUKA_DLL_BUILD)
#define ASUKA_DLL_EXPORT  __declspec(dllexport)
#else
#define ASUKA_DLL_EXPORT
#endif // ASUKA_DLL_BUILD

#define WRITE_BARRIER _WriteBarrier()
#define READ_BARRIER _ReadBarrier()
#define READ_WRITE_BARRIER _ReadWriteBarrier()

#endif // ASUKA_COMPILER_MICROSOFT

#ifdef ASUKA_COMPILER_GNU

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;

#define ASUKA_DEBUG_BREAK __builtin_trap
#define FORCE_INLINE __attribute__((always_inline))

#if defined(ASUKA_DLL_BUILD)
#define ASUKA_DLL_EXPORT  __attribute__((dllexport))
#else
#define ASUKA_DLL_EXPORT
#endif // ASUKA_DLL_BUILD

#endif // ASUKA_COMPILER_GNU

#ifdef ASUKA_DEBUG
#define ASSERT(COND)  if (COND) {} else { ASUKA_DEBUG_BREAK(); } void(0)
#define ASSERT_MSG(COND, MSG)  if (COND) {} else { ASUKA_DEBUG_BREAK(); } void(0)
#else // ASUKA_DEBUG
#define ASSERT(COND)  void(0)
#define ASSERT_MSG(COND, MSG)  void(0)
#endif // ASUKA_DEBUG

#define ASSERT_FAIL(MSG)  ASSERT_MSG(NULL, MSG)
#define INVALID_CODE_PATH(...)  ASSERT_FAIL("Invalid code path")

#define STATIC_ASSERT(COND)  static_assert(COND, "")
#define STATIC_ASSERT_MSG(COND, MSG)  static_assert(COND, MSG)

#define ASUKA_PLAYBACK_LOOP ASUKA_DEBUG

#ifdef ASUKA_OS_WINDOWS

#define osOutputDebugString(MSG, ...) \
{ \
    char OutputBuffer_##__LINE__[256]; \
    sprintf(OutputBuffer_##__LINE__, MSG, __VA_ARGS__); \
    OutputDebugStringA(OutputBuffer_##__LINE__); \
} void(0)

#endif // ASUKA_OS_WINDOWS

#ifdef ASUKA_OS_LINUX
#endif // ASUKA_OS_LINUX

#ifdef ASUKA_OS_MACOS
#endif // ASUKA_OS_MACOS

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define CONCAT_(A, B) A##B
#define CONCAT(A, B) CONCAT_(A, B)
#define CONCAT2(A, B) CONCAT(A, B)
#define CONCAT3(A, B, C) CONCAT2(CONCAT2(A, B), C)

#define OFFSET_OF(STRUCT, MEMBER) ((size_t)&(((STRUCT *)0)->MEMBER(STRUCT, MEMBER)))

#define STATIC            static
#define INTERNAL          static
#define PERSIST           static
#define GLOBAL            static
#define INLINE            inline

#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define KILOBYTES(VALUE) (((u64)(VALUE))*1024)
#define MEGABYTES(VALUE) (KILOBYTES((u64)(VALUE))*1024)
#define GIGABYTES(VALUE) (MEGABYTES((u64)(VALUE))*1024)
#define TERABYTES(VALUE) (GIGABYTES((u64)(VALUE))*1024)

#define EPSILON  (1e-5f)
#define EPSILON² (EPSILON*EPSILON)

#define INT8_MIN   (0x80)
#define INT16_MIN  (0x8000)
#define INT32_MIN  (0x8000'0000)
#define INT64_MIN  (0x8000'0000'0000'0000)

#define INT8_MAX   (0x7F)
#define INT16_MAX  (0x7FFF)
#define INT32_MAX  (0x7FFF'FFFF)
#define INT64_MAX  (0x7FFF'FFFF'FFFF'FFFF)

#define UINT8_MIN  (0)
#define UINT16_MIN (0)
#define UINT32_MIN (0)
#define UINT64_MIN (0)

#define UINT8_MAX  (0xFF)
#define UINT16_MAX (0xFFFF)
#define UINT32_MAX (0xFFFF'FFFF)
#define UINT64_MAX (0xFFFF'FFFF'FFFF'FFFF)

#define SUCCESS    (1)
#define FAILURE    (0)

typedef i16 sound_sample_t;
typedef u64 hash_t;

typedef i32  b32;
typedef size_t usize;
typedef u64 uintptr;
typedef i64  intptr;
typedef i64  ptrdiff;

typedef float  f32;
typedef double f64;

#define loop while(true)

template <typename Callback>
struct Defer {
    Callback cb;
    Defer(Callback const& f) :cb(f) {}
    ~Defer() { cb(); }
};
#define defer Defer CONCAT2(defer__, __LINE__) = [&]()

template <typename ResultType, typename ArgumentType>
[[nodiscard]] constexpr
ResultType cast(ArgumentType argument)
{
    ResultType result = (ResultType) argument;
    return result;
}

#endif // ASUKA_COMMON_DEFINES_HPP
