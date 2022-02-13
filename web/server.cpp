#include <stdio.h>
#include <math.hpp>
#include <os/net.hpp>
#include <os/file.hpp>

#include "parser_http.hpp"
#include "user_controllers.hpp"

#include <windows.h>



char *get_socket_send_error() {
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

    return NULL;
}


char *get_socket_recv_error() {
    return get_socket_send_error();
}


char *get_socket_listen_error() {
    int error = WSAGetLastError();
    switch (error) {
        case WSANOTINITIALISED: return "A successful WSAStartup call must occur before using this function.";
        case WSAENETDOWN: return "The network subsystem has failed.";
        case WSAEADDRINUSE: return "The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during execution of the bind function, but could be delayed until this function if the bind was to a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function.";
        case WSAEINPROGRESS: return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
        case WSAEINVAL: return "The socket has not been bound with bind.";
        case WSAEISCONN: return "The socket is already connected.";
        case WSAEMFILE: return "No more socket descriptors are available.";
        case WSAENOBUFS: return "No buffer space is available.";
        case WSAENOTSOCK: return "The descriptor is not a socket.";
        case WSAEOPNOTSUPP: return "The referenced socket is not of a type that supports the listen operation.";
    }

    return NULL;
}


char *get_socket_accept_error() {
    int error = WSAGetLastError();
    switch (error) {
        case WSANOTINITIALISED: return "A successful WSAStartup call must occur before using this function.";
        case WSAECONNRESET: return "An incoming connection was indicated, but was subsequently terminated by the remote peer prior to accepting the call.";
        case WSAEFAULT: return "The addrlen parameter is too small or addr is not a valid part of the user address space.";
        case WSAEINTR: return "A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.";
        case WSAEINVAL: return "The listen function was not invoked prior to accept.";
        case WSAEINPROGRESS: return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
        case WSAEMFILE: return "The queue is nonempty upon entry to accept and there are no descriptors available.";
        case WSAENETDOWN: return "The network subsystem has failed.";
        case WSAENOBUFS: return "No buffer space is available.";
        case WSAENOTSOCK: return "The descriptor is not a socket.";
        case WSAEOPNOTSUPP: return "The referenced socket is not a type that supports connection-oriented service.";
        case WSAEWOULDBLOCK: return "The socket is marked as nonblocking and no connections are present to be accepted.";
    }

    return NULL;
}


char *get_socket_bind_error() {
    int error = WSAGetLastError();
    switch (error) {
        case WSANOTINITIALISED: return "A successful WSAStartup call must occur before using this function.";
        case WSAENETDOWN: return "The network subsystem has failed.";
        case WSAEACCES: return "An attempt was made to access a socket in a way forbidden by its access permissions. "
                               "This error is returned if nn attempt to bind a datagram socket to the broadcast address failed because the setsockopt option SO_BROADCAST is not enabled.";
        case WSAEADDRINUSE: return "Only one usage of each socket address (protocol/network address/port) is normally permitted. "
                                   "This error is returned if a process on the computer is already bound to the same fully qualified address and the socket has not been marked to allow address reuse with SO_REUSEADDR. "
                                   "For example, the IP address and port specified in the name parameter are already bound to another socket being used by another application. "
                                   "For more information, see the SO_REUSEADDR socket option in the SOL_SOCKET Socket Options reference, Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE, and SO_EXCLUSIVEADDRUSE.";
        case WSAEADDRNOTAVAIL: return "The requested address is not valid in its context. "
                                      "This error is returned if the specified address pointed to by the name parameter is not a valid local IP address on this computer.";
        case WSAEFAULT: return "The system detected an invalid pointer address in attempting to use a pointer argument in a call. "
                               "This error is returned if the name parameter is NULL, the name or namelen parameter is not a valid part of the user address space, "
                               "the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family, "
                               "or the first two bytes of the memory block specified by name do not match the address family associated with the socket descriptor s.";
        case WSAEINPROGRESS: return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
        case WSAEINVAL: return "An invalid argument was supplied. "
                               "This error is returned of the socket s is already bound to an address.";
        case WSAENOBUFS: return "An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full. "
                                "This error is returned of not enough buffers are available or there are too many connections.";
        case WSAENOTSOCK: return "An operation was attempted on something that is not a socket. "
                                 "This error is returned if the descriptor in the s parameter is not a socket.";
    }

    return NULL;
}


char *get_socket_create_error() {
    int error = WSAGetLastError();
    switch (error) {
        case WSANOTINITIALISED: return "A successful WSAStartup call must occur before using this function.";
        case WSAENETDOWN: return "The network subsystem or the associated service provider has failed.";
        case WSAEAFNOSUPPORT: return "The specified address family is not supported. For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapter and device driver is not installed on the local computer.";
        case WSAEINPROGRESS: return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
        case WSAEMFILE: return "No more socket descriptors are available.";
        case WSAEINVAL: return "An invalid argument was supplied. This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol parameter are unspecified.";
        case WSAEINVALIDPROVIDER: return "The service provider returned a version other than 2.2.";
        case WSAEINVALIDPROCTABLE: return "The service provider returned an invalid or incomplete procedure table to the WSPStartup.";
        case WSAENOBUFS: return "No buffer space is available. The socket cannot be created.";
        case WSAEPROTONOSUPPORT: return "The specified protocol is not supported.";
        case WSAEPROTOTYPE: return "The specified protocol is the wrong type for this socket.";
        case WSAEPROVIDERFAILEDINIT: return "The service provider failed to initialize. This error is returned if a layered service provider (LSP) or namespace provider was improperly installed or the provider fails to operate correctly.";
        case WSAESOCKTNOSUPPORT: return "The specified socket type is not supported in this address family.";
    }

    return NULL;
}


char *get_socket_close_error() {
    int error = WSAGetLastError();
    switch (error) {
        case WSANOTINITIALISED: return "A successful WSAStartup call must occur before using this function.";
        case WSAENETDOWN: return "The network subsystem has failed.";
        case WSAENOTSOCK: return "The descriptor is not a socket.";
        case WSAEINPROGRESS: return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
        case WSAEINTR: return "The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.";
        case WSAEWOULDBLOCK: return "The socket is marked as nonblocking, but the l_onoff member of the linger structure is set to nonzero and the l_linger member of the linger structure is set to a nonzero timeout value.";
    }

    return NULL;
}


char *get_wsa_startup_error() {
    int error = WSAGetLastError();
    switch (error) {
        case WSASYSNOTREADY: return "The underlying network subsystem is not ready for network communication.";
        case WSAVERNOTSUPPORTED: return "The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.";
        case WSAEINPROGRESS: return "A blocking Windows Sockets 1.1 operation is in progress.";
        case WSAEPROCLIM: return "A limit on the number of tasks supported by the Windows Sockets implementation has been reached.";
        case WSAEFAULT: return "The lpWSAData parameter is not a valid pointer.";
    }

    return NULL;
}

char *get_wsa_cleanup_error() {
    int error = WSAGetLastError();
    switch (error) {
        case WSANOTINITIALISED: return "A successful WSAStartup call must occur before using this function.";
        case WSAENETDOWN: return "The network subsystem has failed.";
        case WSAEINPROGRESS: return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
    }

    return NULL;
}


char http_200_response[] =
"HTTP/1.1 200 OK\n"
"Connection: close\n"
"\n"
"<head>\n"
"</head>\n"
"<body>\n"
"<img src=\"200_face.png\">\n"
"<h1>Hello World!</h1>\n"
"</body>\n"
"\n"
;


char http_404_response[] =
"HTTP/1.1 404 Not found\n"
"Connection: close\n"
"\n"
"<head>\n"
"</head>\n"
"<body>\n"
"<h1>Code 404! Could not find page!</h1>\n"
"</body>\n"
"\n"
;


char http_500_response[] =
"HTTP/1.1 500 Internal Server Error\n"
"Connection: close\n"
"\n"
"<head>\n"
"</head>\n"
"<body>\n"
"<img src=\"400_face.png\">\n"
"<h1>Code 505! Internal Server Error!</h1>\n"
"</body>\n"
"\n"
;


int main() {
    int ec;

    string favicon_data = os::load_entire_file("favicon.ico");
    string response_200_image = os::load_entire_file("200_face.png");
    string response_400_image = os::load_entire_file("200_face.png");

    uint32 wsa_version_requested = (2 << 8) | 2;

    WSADATA wsa_data;
    ec = WSAStartup((WORD)wsa_version_requested, &wsa_data);
    if (ec != 0) {
        return 1;
    }

    defer { WSACleanup(); };

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s != INVALID_SOCKET) {
        defer { closesocket(s); };

        uint32 ip   = net::make_ip4(127, 0, 0, 1);
        uint16 port = net::make_port(6969);

        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.S_un.S_addr = ip;
        address.sin_port = port;


        ec = bind(s, (const sockaddr *) &address, sizeof(address));
        if (ec != SOCKET_ERROR) {
            ec = listen(s, 1);
            if (ec == 0) {
                while (true) {
                    sockaddr_in client_address;
                    int client_address_size = sizeof(client_address);
                    SOCKET client_socket = accept(s, (sockaddr *)&client_address, &client_address_size);

                    if (client_socket != INVALID_SOCKET) {
                        defer { closesocket(client_socket); };

                        printf("Successfully accepted client's connection from '%s'.\n", inet_ntoa(client_address.sin_addr));

                        char message_data[2048] {};
                        int32 received_bytes = recv(client_socket, message_data, sizeof(message_data), 0);

                        if (received_bytes != SOCKET_ERROR) {
                            if (received_bytes > 0) {
                                string request_data;
                                request_data.data = (uint8 *)message_data;
                                request_data.size = (usize)received_bytes;

                                http::Request request = http::parse_request(request_data);
                                if (request.method == http::HTTP_REQUEST_GET) {
                                    if (equals_to_cstr(request.path, "/favicon.ico")) {
                                        auto response = serve_favicon(request);
                                        string bytes = serialize_response(response);

                                        static const char favicon_header[] =
                                            "HTTP/1.1 200 OK\n"
                                            "Content-Length: 1150\n"
                                            "Content-Type: image/x-icon\n"
                                            "Connection: close\n"
                                            "\n";

                                        int32 sent_bytes = send(client_socket, favicon_header, sizeof(favicon_header) - 1, 0);
                                        if (sent_bytes != SOCKET_ERROR) {
                                            sent_bytes = send(client_socket, (const char *)bytes.data, (int)bytes.size, 0);

                                            if (sent_bytes != SOCKET_ERROR) {
                                                printf("Favicon successfully sent!\n");
                                            } else {
                                                printf("Could not send favicon binary data!\n");
                                                printf("%s\n", get_socket_send_error());
                                            }
                                        } else {
                                            printf("Could not send favicon haeder. How pathetic!\n");
                                            printf("%s\n", get_socket_send_error());
                                        }
                                    } else if (equals_to_cstr(request.path, "/200_face.png")) {
                                        char image_header[1024];
                                        int image_header_size = sprintf(image_header,
                                            "HTTP/1.1 200 OK\n"
                                            "Content-Length: %zu\n"
                                            "Content-Type: image/png\n"
                                            "Connection: close\n"
                                            "\n",
                                            response_200_image.size);

                                        int32 sent_bytes = send(client_socket, image_header, image_header_size - 1, 0);
                                        sent_bytes = send(client_socket, (const char *)response_200_image.data, (int)response_200_image.size, 0);
                                    } else {
                                        int32 sent_bytes = send(client_socket, http_200_response, sizeof(http_200_response), 0);
                                        if (sent_bytes != SOCKET_ERROR) {
                                        } else {
                                            printf("Failed to send response.\n");
                                            printf("%s\n", get_socket_send_error());
                                        }
                                    }
                                } else {
                                    int32 sent_bytes = send(client_socket, http_404_response, sizeof(http_404_response), 0);
                                    if (sent_bytes != SOCKET_ERROR) {
                                    } else {
                                        printf("Failed to send response.\n");
                                        printf("%s\n", get_socket_send_error());
                                    }
                                }
                            }
                        } else {
                            printf("Failed to receive message.\n");
                            printf("%s\n", get_socket_recv_error());
                        }
                    } else {
                        printf("Failed to accept the connection on socket %llu\n", s);
                        printf("%s\n", get_socket_accept_error());
                    }
                }
            } else {
                printf("Failed to listen socket %llu\n", s);
                printf("%s\n", get_socket_listen_error());
            }
        } else {
            printf("Failed to bind socket %llu\n", s);
            printf("%s\n", get_socket_bind_error());
        }
    } else {
        printf("Failed to create socket\n");
        printf("%s\n", get_socket_create_error());
    }

    return 0;
}

