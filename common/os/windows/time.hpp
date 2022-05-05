#ifndef ASUKA_COMMON_OS_WINDOWS_TIME_HPP
#define ASUKA_COMMON_OS_WINDOWS_TIME_HPP


namespace os {
namespace internal {

u64 get_processor_cycles();
f64 get_seconds_per_clock();
i64 get_wall_clock();

} // internal


INLINE
u64 get_processor_cycles()
{
    return internal::get_processor_cycles();
}


INLINE
f64 get_seconds_per_clock()
{
    return internal::get_seconds_per_clock();
}


INLINE
f32 get_seconds(duration d)
{
    f32 result = f32(d.us * get_seconds_per_clock());
    return result;
}


INLINE
timepoint get_wall_clock()
{
    timepoint result {};

    i64 t = internal::get_wall_clock();
    if (t > 0) {
        result.us = u64(t);
    }

    return result;
}


} // os


#if UNITY_BUILD
#include "time.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_OS_WINDOWS_TIME_HPP
