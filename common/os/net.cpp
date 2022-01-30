#include "net.hpp"


#if ASUKA_OS_WINDOWS
#include "windows/net.hpp"
#endif

#if ASUKA_OS_LINUX
#include "linux/net.hpp"
#endif


namespace net {


bool32 open_tcp_socket(IP4 address, Port port) {
    return internal::open_tcp_socket(address, port);
}


} // namespace net
