#ifndef ASUKA_COMMON_DEFINES_HPP
#define ASUKA_COMMON_DEFINES_HPP

/*
    ANSI Graphics for schemes in comments: https://en.wikipedia.org/wiki/Box-drawing_character

    ← ↑ → ↓

    ─ │ ┌ ┐ ┍ ┑ ┎ ┒ ┏ ┓

    ━ ┃ └ ┘ ┕ ┙ ┖ ┚ ┗ ┛

    ╌ ╎ ├ ┤ ┝ ┥ ┞ ┦ ┟ ┧ ┠ ┨ ┡ ┩ ┢ ┪ ┣ ┫

    ╍ ╏ ┮ ┯ ┭ ┬ ┰ ┲ ┳ ┱

    ┄ ┆ ┶ ┷ ┵ ┴ ┸ ┺ ┻ ┹

    ┅ ┇ ╆ ╅ ╈ ┿ ╉ ┼ ┾ ╋

    ┈ ┊ ╄ ╃ ╇ ┽ ╊ ╀ ╁ ╂

    ┉ ┋

    ═ ║ ╒ ╕ ╓ ╖ ╔ ╗

    ╶ ╷ ╘ ╛ ╙ ╜ ╚ ╝

    ╴ ╵ ╞ ╡ ╟ ╢ ╠ ╣

    ╺ ╻ ╤ ╥ ╦

    ╸ ╹ ╧ ╨ ╩ ╪ ╫ ╬

    ╼ ╽ ╭╮

    ╾ ╿ ╯╰  ╱╲ ╳

▕
▏
▎
▍
▋
▊
▉
█▇▆▅▄▃▂▁
▁▂
▔    ▄■▀
▛▜▗▖
▌▐▝▘
▙▟ ▞▚
░▒▓

    ¢¥£®¤øØ×º°§¯¬¦|Ï·¨‗±«»
    ¹²³ªƒßµ
    ½¼¾

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

#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN    4321

typedef __int8  i8;
typedef __int16 i16;
typedef __int32 i32;
typedef __int64 i64;

typedef __int8  int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

typedef float  f32;
typedef double f64;

typedef float  float32;
typedef double float64;

#define ASUKA_DEBUG_BREAK __debugbreak
#define FORCE_INLINE __forceinline

#if ASUKA_DLL_BUILD == 1
#define ASUKA_DLL_EXPORT  __declspec(dllexport)
#else
#define ASUKA_DLL_EXPORT
#endif // ASUKA_DLL_BUILD

#define READ_BARRIER       do { _ReadBarrier(); _mm_lfence(); } while(0)
#define WRITE_BARRIER      do { _WriteBarrier(); _mm_sfence(); } while(0)
#define READ_WRITE_BARRIER do { _ReadWriteBarrier(); _mm_mfence(); } while(0)

#define INTERLOCKED_COMPARE_EXCHANGE InterlockedCompareExchange

#endif // ASUKA_COMPILER_MICROSOFT

#ifdef ASUKA_COMPILER_GNU

typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;

typedef signed char        int8;
typedef signed short       int16;
typedef signed int         int32;
typedef signed long long   int64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

typedef float  f32;
typedef double f64;

typedef float  float32;
typedef double float64;

#define ASUKA_DEBUG_BREAK __builtin_trap
#define FORCE_INLINE __attribute__((always_inline))

#if ASUKA_DLL_BUILD
#define ASUKA_DLL_EXPORT __attribute__((dllexport))
#else
#define ASUKA_DLL_EXPORT
#endif // ASUKA_DLL_BUILD

#endif // ASUKA_COMPILER_GNU

#if ASUKA_DEBUG
#define ASSERT(COND)           if (COND) {} else { ASUKA_DEBUG_BREAK(); } void(0)
#define ASSERT_MSG(COND, ...)  if (COND) {} else { ASUKA_DEBUG_BREAK(); } void(0)
#else // ASUKA_DEBUG
#define ASSERT(COND)           void(0)
#define ASSERT_MSG(COND, ...)  void(0)
#endif // ASUKA_DEBUG

#define ASSERT_FAIL(...)        ASSERT_MSG(NULL, __VA_ARGS__)

#define INVALID_CODE_PATH(...)  ASSERT_FAIL("%s:%d (%s) Invalid code path!", __FILE__, __LINE__, __FUNCTION__);
#define NOT_IMPLEMENTED(...)    ASSERT_FAIL("%s:%d (%s) Not implemented yet!", __FILE__, __LINE__, __FUNCTION__);

#define STATIC_ASSERT(COND)           static_assert(COND)
#define STATIC_ASSERT_MSG(COND, MSG)  static_assert(COND, MSG)

#define ASUKA_PLAYBACK_LOOP ASUKA_DEBUG

#ifdef ASUKA_OS_WINDOWS

#define osOutputDebugString(MSG, ...) \
{  \
    char OutputBuffer_##__LINE__[256]; \
    sprintf(OutputBuffer_##__LINE__, MSG, __VA_ARGS__); \
    OutputDebugStringA(OutputBuffer_##__LINE__); \
} void(0)

#endif // ASUKA_OS_WINDOWS

#ifdef ASUKA_OS_LINUX

#define osOutputDebugString(MSG, ...) \
{  \
    fprintf(stdout, MSG, ##__VA_ARGS__); \
} void(0)

#endif // ASUKA_OS_LINUX

#ifdef ASUKA_OS_MACOS
#endif // ASUKA_OS_MACOS

#define STRINGIFY_(X)    #X
#define STRINGIFY(X)     STRINGIFY_(X)

#define CONCAT_(A, B)    A##B
#define CONCAT(A, B)     CONCAT_(A, B)
#define CONCAT2(A, B)    CONCAT(A, B)
#define CONCAT3(A, B, C) CONCAT2(CONCAT2(A, B), C)

#define OFFSET_OF(STRUCT, MEMBER) ((size_t)&(((STRUCT *)0)->MEMBER(STRUCT, MEMBER)))

#define CONSTANT         const
#define STATIC           static
#define INTERNAL         static
#define PERSIST          static
#define GLOBAL           static
#define INLINE           inline

#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define KILOBYTES(VALUE) (((usize)(VALUE))*1024)
#define MEGABYTES(VALUE) (KILOBYTES((usize)(VALUE))*1024)
#define GIGABYTES(VALUE) (MEGABYTES((usize)(VALUE))*1024)
#define TERABYTES(VALUE) (GIGABYTES((usize)(VALUE))*1024)

#define EPSILON  (1e-5f)
#define EPSILON2 (EPSILON*EPSILON)

#define INT8_MIN   (0x80)
#define INT16_MIN  (0x8000)
#define INT32_MIN  (0x80000000)
#define INT64_MIN  (0x8000000000000000)

#define INT8_MAX   (0x7F)
#define INT16_MAX  (0x7FFF)
#define INT32_MAX  (0x7FFFFFFF)
#define INT64_MAX  (0x7FFFFFFFFFFFFFFF)

#define UINT8_MIN  (0)
#define UINT16_MIN (0)
#define UINT32_MIN (0)
#define UINT64_MIN (0)

#define UINT8_MAX  (0xFF)
#define UINT16_MAX (0xFFFF)
#define UINT32_MAX (0xFFFFFFFF)
#define UINT64_MAX (0xFFFFFFFFFFFFFFFF)

#define SUCCESS    (1)
#define FAILURE    (0)

// Sound sample is 16-bit number
// Sound frame is 2 samples: [left, right]
// Sound period is a number of frames
// Sound buffer is a bunch of periods?
typedef i16 sound_sample_t;

typedef u64 hash_t;

typedef u32 b32;
typedef u32 bool32;

typedef u64 usize;
typedef i64 isize;
typedef u64 uintptr;
typedef i64 intptr;
typedef i64 ptrdiff;

#define loop while(true)
#define when loop if

template <typename Callback>
struct Defer {
    Callback cb;
    Defer(Callback const& f) :cb(f) {}
    ~Defer() { cb(); }
};
#define defer Defer CONCAT2(defer__, __LINE__) = [&]()

#define TOGGLE(X) { (X) = !(X); } void(0)

#define SWAP(x, y) do { auto CONCAT2(SWAP,__LINE__) = (x); (x) = (y); (y) = CONCAT2(SWAP,__LINE__); } while(0)

#endif // ASUKA_COMMON_DEFINES_HPP
