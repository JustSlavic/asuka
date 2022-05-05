#ifndef ASUKA_COMMON_OS_TIME_HPP
#define ASUKA_COMMON_OS_TIME_HPP

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

INLINE
duration operator - (timepoint t1, timepoint t2)
{
    ASSERT(t2.us < t1.us);
    duration result = { t1.us - t2.us };
    return result;
}


INLINE
timepoint operator + (timepoint t, duration d)
{
    timepoint result = { t.us + d.us };
    return result;
}


INLINE
duration operator + (duration d1, duration d2)
{
    duration result = { d1.us + d2.us };
    return result;
}


INLINE
duration operator - (duration d1, duration d2)
{
    duration result = { d1.us - d2.us };
    return result;
}


INLINE
b32 operator < (duration d1, duration d2)
{
    b32 result = (d1.us < d2.us);
    return result;
}

} // os


#if ASUKA_OS_WINDOWS
#include "windows/time.hpp"
#endif

#endif // ASUKA_COMMON_OS_TIME_HPP
