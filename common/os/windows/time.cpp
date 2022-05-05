#include "time.hpp"
#include <windows.h>

namespace os {
namespace internal {


u64 get_processor_cycles()
{
    return __rdtsc();
}


[[nodiscard]] INTERNAL
i64 get_wall_clock_frequency()
{
    PERSIST i64 cache;

    if (cache == 0)
    {
        LARGE_INTEGER PerformanceCounterFrequency;
        QueryPerformanceFrequency(&PerformanceCounterFrequency); // Note: this always succeeds on WinXP and higher
        cache = PerformanceCounterFrequency.QuadPart;
    }

    return cache;
}


f64 get_seconds_per_clock()
{
    PERSIST f64 cache;

    if (cache == 0.0)
    {
        cache = 1.0 / get_wall_clock_frequency();
    }

    return cache;
}


i64 get_wall_clock()
{
    LARGE_INTEGER PerformanceCounter;
    QueryPerformanceCounter(&PerformanceCounter); // Note: this always succeeds on WinXP and higher

    return PerformanceCounter.QuadPart;
}


} // internal
} // os
