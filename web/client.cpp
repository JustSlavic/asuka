#include <stdio.h>
#include <defines.hpp>
#include <math.hpp>
#include <os/net.hpp>

#include <windows.h>


char *get_socket_transmission_error() {
    int error = WSAGetLastError();
    switch (error) {
        case WSANOTINITIALISED: return "A successful WSAStartup call must occur before using this function.";
        case WSAENETDOWN: return "The network subsystem has failed.";
        case WSAEFAULT: return "The buf parameter is not completely contained in a valid part of the user address space.";
        case WSAENOTCONN: return "The socket is not connected.";
        case WSAEINTR: return "The (blocking) call was canceled through WSACancelBlockingCall.";
        case WSAEINPROGRESS: return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
        case WSAENETRESET: return "For a connection-oriented socket, this error indicates that the connection has been broken due to keep-alive activity that detected a failure while the operation was in progress. For a datagram socket, this error indicates that the time to live has expired.";
        case WSAENOTSOCK: return "The descriptor is not a socket.";
        case WSAEOPNOTSUPP: return "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.";
        case WSAESHUTDOWN: return "The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.";
        case WSAEWOULDBLOCK: return "The socket is marked as nonblocking and the receive operation would block.";
        case WSAEMSGSIZE: return "The message was too large to fit into the specified buffer and was truncated.";
        case WSAEINVAL: return "The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.";
        case WSAECONNABORTED: return "The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.";
        case WSAETIMEDOUT: return "The connection has been dropped because of a network failure or because the peer system failed to respond.";
        case WSAECONNRESET: return "The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UDP-datagram socket, this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message.";
    }

    return 0;
}


struct Message {
    math::vector2 p;
    math::vector2 v;

    f32 m;
};


int main() {
    int ec = 0;

    uint32 wsa_version_requested = (2 << 8) | 2;

    WSADATA wsa_data;
    ec = WSAStartup((WORD)wsa_version_requested, &wsa_data);

    printf("Requested wsa version %d\n", wsa_version_requested);
    printf("Returned wsa version: %d\n", wsa_data.wVersion);

    if (ec == 0) {
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s != INVALID_SOCKET) {
            printf("Successfully created a socket %llu\n", s);

            uint32 ip   = net::make_ip4(127, 0, 0, 1);
            uint16 port = net::make_port(6969);

            sockaddr_in address {};
            address.sin_family = AF_INET;
            address.sin_addr.S_un.S_addr = ip;
            address.sin_port = port;
            printf("Try to connect to a socket at the address %s:%d\n", inet_ntoa(address.sin_addr), change_endiannes(port));

            ec = connect(s, (const sockaddr *)&address, sizeof(address));
            if (ec == 0) {
                printf("Connection successful!!! YAY!!!\n");

                char message[2048] {};
                sprintf(message,
                    "GET / HTTP/1.1\n"
                    "Host: %s:%d\n"
                    "Accept: text/html\n"
                    ,
                    "localhost", change_endiannes(port)
                );

                int32 bytes_sent = send(s, message, sizeof(message), 0);
                if (bytes_sent != SOCKET_ERROR) {
                    if (bytes_sent > 0) {
                        printf("Data sent successfully!!!\n");

                        char response[2048] {};
                        int32 bytes_received = recv(s, response, sizeof(response), 0);
                        if (bytes_received != SOCKET_ERROR) {
                            if (bytes_received > 0) {
                                printf("Response received successfully!!!\n");
                                printf("Response: \"%s\"\n", response);
                            }
                        } else {
                            printf("Failed to receive response.\n");
                            printf("%s\n", get_socket_transmission_error());
                        }
                    }
                } else {
                    printf("Failed to send message.\n");
                    printf("%s\n", get_socket_transmission_error());
                }
            } else {
                int error = WSAGetLastError();
                switch (error) {
                    case WSANOTINITIALISED: printf("A successful WSAStartup call must occur before using this function.\n"); break;
                    case WSAENETDOWN: printf("The network subsystem has failed.\n"); break;
                    case WSAEADDRINUSE: printf("The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs when executing bind, but could be delayed until the connect function if the bind was to a wildcard address (INADDR_ANY or in6addr_any) for the local IP address. A specific address needs to be implicitly bound by the connect function.\n"); break;
                    case WSAEINTR: printf("The blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.\n"); break;
                    case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
                    case WSAEALREADY: printf("A nonblocking connect call is in progress on the specified socket. Note: In order to preserve backward compatibility, this error is reported as WSAEINVAL to Windows Sockets 1.1 applications that link to either Winsock.dll or Wsock32.dll.\n"); break;
                    case WSAEADDRNOTAVAIL: printf("The remote address is not a valid address (such as INADDR_ANY or in6addr_any) .\n"); break;
                    case WSAEAFNOSUPPORT: printf("Addresses in the specified family cannot be used with this socket.\n"); break;
                    case WSAECONNREFUSED: printf("The attempt to connect was forcefully rejected.\n"); break;
                    case WSAEFAULT: printf("The sockaddr structure pointed to by the name contains incorrect address format for the associated address family or the namelen parameter is too small. This error is also returned if the sockaddr structure pointed to by the name parameter with a length specified in the namelen parameter is not in a valid part of the user address space.\n"); break;
                    case WSAEINVAL: printf("The parameter s is a listening socket.\n"); break;
                    case WSAEISCONN: printf("The socket is already connected (connection-oriented sockets only).\n"); break;
                    case WSAENETUNREACH: printf("The network cannot be reached from this host at this time.\n"); break;
                    case WSAEHOSTUNREACH: printf("A socket operation was attempted to an unreachable host.\n"); break;
                    case WSAENOBUFS: printf("No buffer space is available. The socket cannot be connected.\n"); break;
                    case WSAENOTSOCK: printf("The descriptor specified in the s parameter is not a socket.\n"); break;
                    case WSAETIMEDOUT: printf("An attempt to connect timed out without establishing a connection.\n"); break;
                    case WSAEWOULDBLOCK: printf("The socket is marked as nonblocking and the connection cannot be completed immediately.\n"); break;
                    case WSAEACCES: printf("An attempt to connect a datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled.\n"); break;
                }
            }

            ec = closesocket(s);
            if (ec != SOCKET_ERROR) {
                printf("Socket %llu closed.\n", s);
            } else {
                int error = WSAGetLastError();
                switch (error) {
                    case WSANOTINITIALISED: printf("A successful WSAStartup call must occur before using this function.\n"); break;
                    case WSAENETDOWN: printf("The network subsystem has failed.\n"); break;
                    case WSAENOTSOCK: printf("The descriptor is not a socket.\n"); break;
                    case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
                    case WSAEINTR: printf("The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.\n"); break;
                    case WSAEWOULDBLOCK: printf("The socket is marked as nonblocking, but the l_onoff member of the linger structure is set to nonzero and the l_linger member of the linger structure is set to a nonzero timeout value.\n"); break;
                }
            }
        } else {
            printf("Could not create socket\n");
            ec = WSAGetLastError();

            switch (ec) {
                case WSANOTINITIALISED: printf("A successful WSAStartup call must occur before using this function.\n"); break;
                case WSAENETDOWN: printf("The network subsystem or the associated service provider has failed.\n"); break;
                case WSAEAFNOSUPPORT: printf("The specified address family is not supported. For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapter and device driver is not installed on the local computer.\n"); break;
                case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
                case WSAEMFILE: printf("No more socket descriptors are available.\n"); break;
                case WSAEINVAL: printf("An invalid argument was supplied. This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol parameter are unspecified.\n"); break;
                case WSAEINVALIDPROVIDER: printf("The service provider returned a version other than 2.2.\n"); break;
                case WSAEINVALIDPROCTABLE: printf("The service provider returned an invalid or incomplete procedure table to the WSPStartup.\n"); break;
                case WSAENOBUFS: printf("No buffer space is available. The socket cannot be created.\n"); break;
                case WSAEPROTONOSUPPORT: printf("The specified protocol is not supported.\n"); break;
                case WSAEPROTOTYPE: printf("The specified protocol is the wrong type for this socket.\n"); break;
                case WSAEPROVIDERFAILEDINIT: printf("The service provider failed to initialize. This error is returned if a layered service provider (LSP) or namespace provider was improperly installed or the provider fails to operate correctly.\n"); break;
                case WSAESOCKTNOSUPPORT: printf("The specified socket type is not supported in this address family.\n"); break;
            }
        }

        ec = WSACleanup();
        if (ec == 0) {
            printf("Successful WSACleanup.\n");
        } else {
            int error = WSAGetLastError();
            switch (error) {
                case WSANOTINITIALISED: printf("A successful WSAStartup call must occur before using this function.\n"); break;
                case WSAENETDOWN: printf("The network subsystem has failed.\n"); break;
                case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
            }
        }
    } else {
        switch (ec) {
            case WSASYSNOTREADY: printf("The underlying network subsystem is not ready for network communication.\n"); break;
            case WSAVERNOTSUPPORTED: printf("The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.\n"); break;
            case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 operation is in progress.\n"); break;
            case WSAEPROCLIM: printf("A limit on the number of tasks supported by the Windows Sockets implementation has been reached.\n"); break;
            case WSAEFAULT: printf("The lpWSAData parameter is not a valid pointer.\n"); break;
        }
    }

    return 0;
}
