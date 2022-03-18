#ifndef ASUKA_COMMON_OS_TIME_HPP
#define ASUKA_COMMON_OS_TIME_HPP

#include <defines.hpp>


namespace os {

u64 get_processor_cycles();
i64 get_wall_clock_frequency();
i64 get_wall_clock();

} // os


#if ASUKA_OS_WINDOWS
#include "windows/time.hpp"
#endif

#endif // ASUKA_COMMON_OS_TIME_HPP
