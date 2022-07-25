#pragma once

#include <defines.hpp>


namespace os {

struct timepoint {
    union {
        u64 us;
        u64 microseconds_from_epoch;
    };
};

struct duration {
    union {
        u64 us;
        u64 microseconds;
    };
};


u64 get_processor_cycles();
timepoint get_wall_clock();
f32 get_seconds(duration d);

duration operator - (timepoint t1, timepoint t2);
timepoint operator + (timepoint t, duration d);
duration operator + (duration d1, duration d2);
duration operator - (duration d1, duration d2);
b32 operator < (duration d1, duration d2);

} // os

#if UNITY_BUILD
#include "time.cpp"
#endif
