#ifndef ASUKA_COMMON_OS_LINUX_TIME_HPP
#define ASUKA_COMMON_OS_LINUX_TIME_HPP

#include <defines.hpp>
#include <os/time.hpp>

namespace os {
namespace internal {

uint64 get_processor_cycles();
int64 get_wall_clock_frequency();
timepoint get_wall_clock();

} // internal

} // os

#ifdef UNITY_BUILD
#include "time.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_OS_LINUX_TIME_HPP
