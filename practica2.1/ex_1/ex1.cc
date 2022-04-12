#include <cstring>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define SOCK_TYPES 3

int socket_types[SOCK_TYPES] = { SOCK_STREAM, SOCK_DGRAM, SOCK_RAW };

bool printSocketInfo(const char *addr, char *serv, addrinfo hints, addrinfo *result)
{
    for (size_t index = 0; index < SOCK_TYPES; index++)
    {
        hints.ai_socktype = socket_types[index];

        int error_code = getaddrinfo(addr, serv, &hints, &result);
        if (error_code != 0) {
            std::cerr << "Error: getaddrinfo -> " << gai_strerror(error_code) << "\n";
            return false;
        }

        char hostName[NI_MAXHOST], serviceName[NI_MAXSERV];

        error_code = getnameinfo(result->ai_addr, result->ai_addrlen, hostName, sizeof(hostName),
            serviceName, sizeof(serviceName), NI_NUMERICHOST | NI_NUMERICSERV);
        if (error_code != 0) {
            std::cerr << "Error: getnameinfo -> " << gai_strerror(error_code) << "\n";
            return false;
        }

        std::cout << "+ " << hostName << '\t' << result->ai_family << '\t' << result->ai_socktype << "\n";
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        std::cerr << "Error: not enough arguments\n";

    addrinfo hints;
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));
    memset(&result, 0, sizeof(addrinfo));

    hints.ai_family = AF_INET;
    bool success = printSocketInfo(argv[1], argv[2], hints, result);
    if(!success) return -1;

    hints.ai_family = AF_INET6;
    printSocketInfo(argv[1], argv[2], hints, result);
    if(!success) return -1;

    freeaddrinfo(result);

    return 0;
}