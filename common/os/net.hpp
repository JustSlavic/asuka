#pragma once

#include <defines.hpp>
#include <math/integer.hpp>


namespace net {


struct IP4 {
    uint32 address;
};


struct Port {
    uint16 port;
};


struct Socket {
    IP4 address;
    Port port;

    // Platform specific data.
    uint8 data[8];
};


INLINE
uint32 make_ip4(uint8 a, uint8 b, uint8 c, uint8 d) {
#if LITTLE_ENDIAN
    uint32 result = (((uint32)d) << 24) |
                    (((uint32)c) << 16) |
                    (((uint32)b) << 8)  |
                    (((uint32)a));
#endif

#if BIG_ENDIAN
    uint32 result = (((uint32)a) << 24) |
                    (((uint32)b) << 16) |
                    (((uint32)c) << 8)  |
                    (((uint32)d));
#endif

    return result;
}

INLINE
uint16 make_port(uint16 n) {
    uint16 result = to_big_endian(n);
    return result;
}


bool32 open_tcp_socket(IP4 address, Port port);


#if UNITY_BUILD
#include "net.cpp"
#endif

} // namespace net
