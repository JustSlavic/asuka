#ifndef ASUKA_COMMON_OS_TIME_HPP
#define ASUKA_COMMON_OS_TIME_HPP

#include <defines.hpp>


namespace os {

struct timepoint {
    union {
        uint64 us;
        uint64 microseconds_from_epoch;
    };
};

struct duration {
    union {
        uint64 us;
        uint64 microseconds;
    };
};

uint64 get_processor_cycles();
int64  get_wall_clock_frequency(); // @note: tick per seconds?
timepoint  get_wall_clock();

duration operator - (timepoint t1, timepoint t2) {
    return duration { t1.us - t2.us };
}

timepoint operator + (timepoint t, duration d) {
    return timepoint { t.us + d.us };
}

duration operator + (duration d1, duration d2) {
    return duration { d1.us + d2.us };
}

duration operator - (duration d1, duration d2) {
    return duration { d1.us - d2.us };
}

bool operator < (duration d1, duration d2) {
    return d1.us < d2.us;
}

} // os

#if ASUKA_OS_WINDOWS
#include "windows/time.hpp"
#endif

#if ASUKA_OS_LINUX
#include "linux/time.hpp"
#endif

#endif // ASUKA_COMMON_OS_TIME_HPP
