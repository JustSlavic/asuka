#include "time.hpp"
#include <sys/time.h>
#include <x86intrin.h>

namespace os {
namespace internal {

uint64 get_processor_cycles() {
    return __rdtsc();
}

int64 get_wall_clock_frequency() {
    return 1'000'000;
}

// Returns number of microseconds since Epoch
timepoint get_wall_clock() {
    struct timeval time;
    if (gettimeofday(&time, NULL)) {
        return timepoint {};
    }

    return timepoint{ (uint64)time.tv_sec * 1'000'000llu + (uint64)time.tv_usec };
}

} // internal
} // os
