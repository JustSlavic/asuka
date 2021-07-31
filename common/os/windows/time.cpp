#include "time.hpp"
#include <windows.h>

namespace os {
namespace windows {


uint64 get_processor_cycles() {
    return __rdtsc();
}

int64 get_performance_counter_frequency() {
    static int64 cache;

    if (cache == 0) {
        LARGE_INTEGER PerformanceCounterFrequency;
        QueryPerformanceFrequency(&PerformanceCounterFrequency); // Note: this always succeeds on WinXP and higher
        cache = PerformanceCounterFrequency.QuadPart;
    }

    return cache;
}

int64 get_performance_counter() {
    LARGE_INTEGER PerformanceCounter;
    QueryPerformanceCounter(&PerformanceCounter); // Note: this always succeeds on WinXP and higher

    return PerformanceCounter.QuadPart;
}


} // windows
} // os
