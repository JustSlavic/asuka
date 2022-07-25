#pragma once

#include <defines.hpp>


i8  change_endiannes(i8 n);
i16 change_endiannes(i16 n);
i32 change_endiannes(i32 n);
i64 change_endiannes(i64 n);

u8  change_endiannes(u8 n);
u16 change_endiannes(u16 n);
u32 change_endiannes(u32 n);
u64 change_endiannes(u64 n);

u8  to_little_endian(u8 n);
u16 to_little_endian(u16 n);
u32 to_little_endian(u32 n);
u64 to_little_endian(u64 n);

u8  to_big_endian(u8 n);
u16 to_big_endian(u16 n);
u32 to_big_endian(u32 n);
u64 to_big_endian(u64 n);
