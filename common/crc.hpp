#ifndef ASUKA_COMMON_CRC_HPP
#define ASUKA_COMMON_CRC_HPP

#include "defines.hpp"


//
// Return the CRC of the bytes buf[0..len-1].
//
u32 compute_crc(u8 *buf, usize len);


#ifdef UNITY_BUILD
#include "crc.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_CRC_HPP
