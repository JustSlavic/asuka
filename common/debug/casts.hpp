#ifndef ASUKA_COMMON_DEBUG_CASTS_HPP
#define ASUKA_COMMON_DEBUG_CASTS_HPP


inline uint8  truncate_cast_to_uint8  (uint32 value)  { return (uint8)  value; }
inline  int16 truncate_cast_to_int16  (int32 value)   { return (int16)  value; }
inline  int16 truncate_cast_to_int16  (float32 value) { return (int16)  value; }
inline uint16 truncate_cast_to_uint16 (uint32 value)  { return (uint16) value; }
inline uint16 truncate_cast_to_uint16 (float32 value) { return (uint16) value; }
inline  int32 truncate_cast_to_int32  (float32 value) { return (int32)  value; }
inline uint32 truncate_cast_to_uint32 (float32 value) { return (uint32) value; }


#endif // ASUKA_COMMON_DEBUG_CASTS_HPP
