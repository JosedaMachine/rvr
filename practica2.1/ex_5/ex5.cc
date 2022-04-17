#include <time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 80

bool createSocket(const char *host, char *serv, addrinfo hints, addrinfo *result, int *socketDesc)
{
    //Transalate name to socket addresses
    int rc = getaddrinfo(host, serv, &hints, &result);

    if (rc != 0)
    {
        std::cerr << "Error: getaddrinfo -> " << gai_strerror(rc) << "\n";
        return false;
    }

    //Open socket with result content. Always 0 to TCP and UDP
    (*socketDesc) = socket(result->ai_family, result->ai_socktype, 0);

    int error_code = connect((*socketDesc), result->ai_addr, result->ai_addrlen);
    if (error_code == -1)
    {
        std::cerr << "Error conexión servidor.\n";
        shutdown((*socketDesc), SHUT_RDWR);
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    int socketDesc;

    addrinfo hints;
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));
    memset(&result, 0, sizeof(addrinfo));

    hints.ai_flags    = AI_PASSIVE; //Devolver 0.0.0.0
    hints.ai_family   = AF_INET;    // IPv4
    hints.ai_socktype = SOCK_STREAM;

    //Initialize connection and listening
    if (!createSocket(argv[1], argv[2], hints, result, &socketDesc)) {
        std::cerr << "Error: Initialization\n";
        return -1;
    }

    while (true) {
        char buf[BUF_SIZE];

        fgets(buf, BUF_SIZE, stdin);
        int buf_size = strlen(buf);
        if (strlen(buf) == 2 && buf[0] == 'Q')
            break;

        int bytes = send(socketDesc, buf, buf_size, 0);
        if (bytes <= 0)
            break;

        char response[BUF_SIZE];
        memset(response, 0, BUF_SIZE);
        bytes = recv(socketDesc, response, BUF_SIZE, 0);

        if (bytes <= 0)
            break;

        std::cout << response;
    }

    close(socketDesc);
    freeaddrinfo(result);
    return 0;
}