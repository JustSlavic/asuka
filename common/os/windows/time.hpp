#ifndef ASUKA_COMMON_OS_WINDOWS_TIME_HPP
#define ASUKA_COMMON_OS_WINDOWS_TIME_HPP


namespace os {
namespace windows {

uint64 get_processor_cycles();
int64 get_wall_clock_frequency();
int64 get_wall_clock();

} // windows

uint64 get_processor_cycles() { return windows::get_processor_cycles(); }
int64 get_wall_clock_frequency() { return windows::get_wall_clock_frequency(); }
int64 get_wall_clock() { return windows::get_wall_clock(); }

} // os


#ifdef UNITY_BUILD
#include "time.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_OS_WINDOWS_TIME_HPP
