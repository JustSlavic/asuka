#ifndef ASUKA_COMMON_DEBUG_CASTS_HPP
#define ASUKA_COMMON_DEBUG_CASTS_HPP


inline u8  truncate_cast_to_uint8  (u32 value) { return (u8)  value; }
inline i16 truncate_cast_to_int16  (i32 value) { return (i16) value; }
inline i16 truncate_cast_to_int16  (f32 value) { return (i16) value; }
inline u16 truncate_cast_to_uint16 (u32 value) { return (u16) value; }
inline u16 truncate_cast_to_uint16 (f32 value) { return (u16) value; }
inline i32 truncate_cast_to_int32  (f32 value) { return (i32) value; }
inline u32 truncate_cast_to_uint32 (f32 value) { return (u32) value; }


#endif // ASUKA_COMMON_DEBUG_CASTS_HPP
