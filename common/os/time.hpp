#ifndef ASUKA_COMMON_OS_TIME_HPP
#define ASUKA_COMMON_OS_TIME_HPP


namespace os {

uint64 get_processor_cycles();
int64  get_wall_clock_frequency();
int64  get_wall_clock();

} // os


#if ASUKA_OS_WINDOWS
#include "windows/time.hpp"
#endif

#endif // ASUKA_COMMON_OS_TIME_HPP
