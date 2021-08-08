#ifndef ASUKA_COMMON_CRC_HPP
#define ASUKA_COMMON_CRC_HPP


namespace png {

//
// Return the CRC of the bytes buf[0..len-1].
//
uint32 crc(uint8 *buf, int32 len);

} // namespace png


#ifdef UNITY_BUILD
#include "crc.cpp"
#endif // UNITY_BUILD

#endif // ASUKA_COMMON_CRC_HPP
