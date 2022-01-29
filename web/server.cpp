#include <windows.h>

#include <stdio.h>
#include <defines.hpp>
#include <math/integer.hpp>


// @note: because we are on the little-endian machine, "A" goes to the lower bits in the number, but it will go in memory at the front.
// So that 127.0.0.1 will be as number "1...0...0...127", but in the memory it will be stored as "127...0...0...1".
#define IP4(A, B, C, D) ((((uint32)A) & 0xFF) | ((((uint32)B) & 0xFF) << 8) | ((((uint32)C) & 0xFF) << 16) | (((uint32)D) & 0xFF) << 24)
#define PORT(P) (((((uint16)P) & 0x00FF) << 8) | ((((uint16)P) & 0xFF00) >> 8))


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

            uint32 ip   = IP4(127, 0, 0, 1);
            uint16 port = PORT(61079);

            sockaddr_in address {};
            address.sin_family = AF_INET;
            address.sin_addr.S_un.S_addr = ip;
            address.sin_port = port;
            printf("Try to bind a socket to the address %s:%d\n", inet_ntoa(address.sin_addr), swap_endiannes(port));

            ec = bind(s, (const sockaddr *) &address, sizeof(address));
            if (ec != SOCKET_ERROR) {
                printf("Successfully binded a socket to the address %s:%d\n", inet_ntoa(address.sin_addr), swap_endiannes(port));

                ec = listen(s, 1);
                if (ec == 0) {
                    printf("Successfully started to listen socket %llu.\n", s);

                    sockaddr_in client_address;
                    int client_address_size = sizeof(client_address);
                    SOCKET client_socket = accept(s, (sockaddr *)&client_address, &client_address_size);

                    if (client_socket != INVALID_SOCKET) {
                        printf("Successfully accepted client's connection from '%s'.\n", inet_ntoa(client_address.sin_addr));

                        char message[1024] {};
                        ec = recv(client_socket,  message, sizeof(message), 0);

                        if (ec != SOCKET_ERROR) {
                            printf("Data received successfully!!! Message: \"%s\"\n", message);
                        } else {
                            printf("Failed to receive message.\n");
                            int error = WSAGetLastError();
                            switch (error) {
                                case WSANOTINITIALISED: printf("A successful WSAStartup call must occur before using this function.\n"); break;
                                case WSAENETDOWN: printf("The network subsystem has failed.\n"); break;
                                case WSAEFAULT: printf("The buf parameter is not completely contained in a valid part of the user address space.\n"); break;
                                case WSAENOTCONN: printf("The socket is not connected.\n"); break;
                                case WSAEINTR: printf("The (blocking) call was canceled through WSACancelBlockingCall.\n"); break;
                                case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
                                case WSAENETRESET: printf("For a connection-oriented socket, this error indicates that the connection has been broken due to keep-alive activity that detected a failure while the operation was in progress. For a datagram socket, this error indicates that the time to live has expired.\n"); break;
                                case WSAENOTSOCK: printf("The descriptor is not a socket.\n"); break;
                                case WSAEOPNOTSUPP: printf("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.\n"); break;
                                case WSAESHUTDOWN: printf("The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.\n"); break;
                                case WSAEWOULDBLOCK: printf("The socket is marked as nonblocking and the receive operation would block.\n"); break;
                                case WSAEMSGSIZE: printf("The message was too large to fit into the specified buffer and was truncated.\n"); break;
                                case WSAEINVAL: printf("The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.\n"); break;
                                case WSAECONNABORTED: printf("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.\n"); break;
                                case WSAETIMEDOUT: printf("The connection has been dropped because of a network failure or because the peer system failed to respond.\n"); break;
                                case WSAECONNRESET: printf("The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UDP-datagram socket, this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message.\n"); break;
                            }
                        }
                    } else {
                        printf("Failed to accept the connection on socket %llu\n", s);
                        int error = WSAGetLastError();
                        switch (error) {
                            case WSANOTINITIALISED: printf("A successful WSAStartup call must occur before using this function.\n"); break;
                            case WSAECONNRESET: printf("An incoming connection was indicated, but was subsequently terminated by the remote peer prior to accepting the call.\n"); break;
                            case WSAEFAULT: printf("The addrlen parameter is too small or addr is not a valid part of the user address space.\n"); break;
                            case WSAEINTR: printf("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.\n"); break;
                            case WSAEINVAL: printf("The listen function was not invoked prior to accept.\n"); break;
                            case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
                            case WSAEMFILE: printf("The queue is nonempty upon entry to accept and there are no descriptors available.\n"); break;
                            case WSAENETDOWN: printf("The network subsystem has failed.\n"); break;
                            case WSAENOBUFS: printf("No buffer space is available.\n"); break;
                            case WSAENOTSOCK: printf("The descriptor is not a socket.\n"); break;
                            case WSAEOPNOTSUPP: printf("The referenced socket is not a type that supports connection-oriented service.\n"); break;
                            case WSAEWOULDBLOCK: printf("The socket is marked as nonblocking and no connections are present to be accepted.\n"); break;
                        }
                    }
                } else {
                    printf("Failed to listen socket %llu\n", s);
                    int error = WSAGetLastError();
                    switch (error) {
                        case WSANOTINITIALISED: printf("A successful WSAStartup call must occur before using this function.\n"); break;
                        case WSAENETDOWN: printf("The network subsystem has failed.\n"); break;
                        case WSAEADDRINUSE: printf("The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during execution of the bind function, but could be delayed until this function if the bind was to a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function.\n"); break;
                        case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
                        case WSAEINVAL: printf("The socket has not been bound with bind.\n"); break;
                        case WSAEISCONN: printf("The socket is already connected.\n"); break;
                        case WSAEMFILE: printf("No more socket descriptors are available.\n"); break;
                        case WSAENOBUFS: printf("No buffer space is available.\n"); break;
                        case WSAENOTSOCK: printf("The descriptor is not a socket.\n"); break;
                        case WSAEOPNOTSUPP: printf("The referenced socket is not of a type that supports the listen operation.\n"); break;
                    }
                }
            } else {
                printf("Failed to bind socket %llu\n", s);

                int error = WSAGetLastError();
                switch (error) {
                    case WSANOTINITIALISED: printf("A successful WSAStartup call must occur before using this function.\n"); break;

                    case WSAENETDOWN: printf("The network subsystem has failed.\n"); break;

                    case WSAEACCES: printf("An attempt was made to access a socket in a way forbidden by its access permissions.\n"
                                           "This error is returned if nn attempt to bind a datagram socket to the broadcast address failed because the setsockopt option SO_BROADCAST is not enabled.\n"); break;

                    case WSAEADDRINUSE: printf("Only one usage of each socket address (protocol/network address/port) is normally permitted.\n"
                                               "This error is returned if a process on the computer is already bound to the same fully qualified address and the socket has not been marked to allow address reuse with SO_REUSEADDR.\n"
                                               "For example, the IP address and port specified in the name parameter are already bound to another socket being used by another application.\n"
                                               "For more information, see the SO_REUSEADDR socket option in the SOL_SOCKET Socket Options reference, Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE, and SO_EXCLUSIVEADDRUSE.\n"); break;

                    case WSAEADDRNOTAVAIL: printf("The requested address is not valid in its context.\n"
                                                  "This error is returned if the specified address pointed to by the name parameter is not a valid local IP address on this computer.\n"); break;

                    case WSAEFAULT: printf("The system detected an invalid pointer address in attempting to use a pointer argument in a call.\n"
                                           "This error is returned if the name parameter is NULL, the name or namelen parameter is not a valid part of the user address space,\n"
                                           "the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family,\n"
                                           "or the first two bytes of the memory block specified by name do not match the address family associated with the socket descriptor s.\n"); break;

                    case WSAEINPROGRESS: printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n"); break;
                    case WSAEINVAL: printf("An invalid argument was supplied.\n"
                                           "This error is returned of the socket s is already bound to an address.\n"); break;

                    case WSAENOBUFS: printf("An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.\n"
                                            "This error is returned of not enough buffers are available or there are too many connections.\n"); break;

                    case WSAENOTSOCK: printf("An operation was attempted on something that is not a socket.\n"
                                             "This error is returned if the descriptor in the s parameter is not a socket.\n"); break;
                }
            }

            ec = closesocket(s);
            if (ec != SOCKET_ERROR) {
                printf("Socket %llu closed.\n", s);
            } else {
                printf("Failed to close socket %llu\n", s);
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
            printf("Failed to create socket\n");
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
            printf("Failed do WSACleanup\n");
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
