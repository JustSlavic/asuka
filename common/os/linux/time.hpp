#ifndef ASUKA_COMMON_OS_LINUX_TIME_HPP
#define ASUKA_COMMON_OS_LINUX_TIME_HPP

namespace os {
namespace internal {

uint64 get_processor_cycles();
int64 get_wall_clock_frequency();
timepoint get_wall_clock();

} // internal

inline uint64 get_processor_cycles() { return internal::get_processor_cycles(); }
inline int64  get_wall_clock_frequency() { return internal::get_wall_clock_frequency(); }
inline timepoint get_wall_clock() { return internal::get_wall_clock(); }

} // os

#ifdef UNITY_BUILD
#include "time.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_OS_LINUX_TIME_HPP
