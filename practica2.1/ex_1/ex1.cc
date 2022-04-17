#include <cstring>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define NUM_SOCKET_TYPES 3

int socket_types[NUM_SOCKET_TYPES] = { SOCK_STREAM, SOCK_DGRAM, SOCK_RAW };

bool print_socket_data(const char *addr, char *serv, addrinfo hints, addrinfo *result) {
    for (size_t index = 0; index < NUM_SOCKET_TYPES; index++)
    {
        // assign the corresponding socket type
        hints.ai_socktype = socket_types[index];

        // get the address (with its data)
        int error_code = getaddrinfo(addr, serv, &hints, &result);
        if (error_code != 0) {
            std::cerr << "Error: getaddrinfo -> " << gai_strerror(error_code) << "\n";
            return false;
        }

        char hostName[NI_MAXHOST], serviceName[NI_MAXSERV];

        // get the numeric value of the addrs
        error_code = getnameinfo(result->ai_addr, result->ai_addrlen, hostName, sizeof(hostName),
            serviceName, sizeof(serviceName), NI_NUMERICHOST | NI_NUMERICSERV);
        if (error_code != 0) {
            std::cerr << "Error: getnameinfo -> " << gai_strerror(error_code) << "\n";
            return false;
        }

        // print address data
        std::cout << "+ " << hostName << '\t' << result->ai_family << '\t' << result->ai_socktype << "\n";
    }

    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2)
        std::cerr << "Error: not enough arguments.\n";

    addrinfo hints;
    addrinfo *result;

    // reserve space for the structs
    memset(&hints, 0, sizeof(addrinfo));
    memset(&result, 0, sizeof(addrinfo));

    // first, we print the IPv4 adresses
    hints.ai_family = AF_INET;
    bool success = print_socket_data(argv[1], argv[2], hints, result);
    if(!success) return -1;

    // second, we print the IPv6 adresses
    hints.ai_family = AF_INET6;
    print_socket_data(argv[1], argv[2], hints, result);
    if(!success) return -1;

    freeaddrinfo(result);

    return 0;
}