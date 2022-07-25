#include "time.hpp"

#if ASUKA_OS_WINDOWS
#include "windows/time.hpp"
#endif

#if ASUKA_OS_LINUX
#include "linux/time.hpp"
#endif

namespace os
{


uint64 get_processor_cycles()
{
    return internal::get_processor_cycles();
}

int64  get_wall_clock_frequency()
{
    return internal::get_wall_clock_frequency();
}

timepoint get_wall_clock()
{
    return internal::get_wall_clock();
}


duration operator - (timepoint t1, timepoint t2)
{
    ASSERT(t2.us < t1.us);
    duration result = { t1.us - t2.us };
    return result;
}

timepoint operator + (timepoint t, duration d)
{
    timepoint result = { t.us + d.us };
    return result;
}

duration operator + (duration d1, duration d2)
{
    duration result = { d1.us + d2.us };
    return result;
}

duration operator - (duration d1, duration d2)
{
    duration result = { d1.us - d2.us };
    return result;
}

b32 operator < (duration d1, duration d2)
{
    b32 result = (d1.us < d2.us);
    return result;
}

} // namespace os
