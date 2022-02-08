#pragma once


namespace net {
namespace internal {


bool32 open_tcp_socket(IP4 address, Port port);


} // namespace internal
} // namespace net


#if UNITY_BUILD
#include "net.cpp"
#endif
