/*

Reference: http://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html

*/

#include "crc.hpp"


//
// Table of CRCs of all 8-bit messages.
//
u32 crc_table[256];

// Flag: has the table been computed? Initially false.
i32 crc_table_computed = 0;

//
// Make the table for a fast CRC.
//
void make_crc_table() {
    for (i32 n = 0; n < 256; n++) {
        u32 c = (u32) n;
        for (i32 k = 0; k < 8; k++) {
            if (c & 1) {
                // polynomial: 1+x+x^2+x^4+x^5+x^7+x^8+x^10+x^11+x^12+x^16+x^22+x^23+x26+x^32
                // 1110'1101'1011'1000'1000'0011'0010'0000
                // Note: x^32 is not written explicitly, it is implied; To account this last 1 bit
                //       we take 1's compliment of the result.
                u32 polynomial = 0xedb88320; // Hex representation of this polynomial
                c = polynomial ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}

//
// Update a running CRC with the bytes buf[0..len-1] -- the CRC
// should be initialized to all 1's, and the transmitted value
// is the 1's complement of the final running CRC (see the crc() routine below).
//
u32 update_crc(u32 init, u8 *buf, usize len) {
    u32 crc = init;

    if (!crc_table_computed) {
        make_crc_table();
    }

    for (usize n = 0; n < len; n++) {
        crc = crc_table[(crc ^ buf[n]) & 0xff] ^ (crc >> 8);
        //              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        //                   What is going on here???
    }

    return crc;
}

//
// Return the CRC of the bytes buf[0..len-1].
//
u32 compute_crc(u8 *buf, usize len) {
    return update_crc(0xffff'ffff, buf, len) ^ 0xffff'ffff;
    //                ^^^^^^^^^^^            ^^^^^^^^^^^^^
    //                Initialize all 1's     Take 1's complement of the result
}
