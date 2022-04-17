#include <time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

bool init_connection(const char *host, char *serv, addrinfo hints, addrinfo *result, int *socket_dc) {
    // translate name to socket addresses
    int error_code = getaddrinfo(host, serv, &hints, &result);
    if (error_code != 0) {
        std::cerr << "Error: getaddrinfo -> " << gai_strerror(error_code) << "\n";
        return false;
    }

    // open socket with result content. Always 0 to TCP and UDP
    (*socket_dc) = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if ((*socket_dc) == -1) {
        std::cerr << "Error: failure on socket open.\n";
        return false;
    }

    // connect the client to the server
    error_code = connect((*socket_dc), result->ai_addr, result->ai_addrlen);
    if (error_code == -1) {
        std::cerr << "Error: connection failed.\n";
        shutdown((*socket_dc), SHUT_RDWR);
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    
    int socket_dc;

    addrinfo hints;
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));
    memset(&result, 0, sizeof(addrinfo));

    hints.ai_flags    = AI_PASSIVE;  // 0.0.0.0
    hints.ai_family   = AF_INET;     // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // initialize connection
    if(!init_connection(argv[1],argv[2], hints, result, &socket_dc)){
        std::cerr << "Error: failed connection init.\n";
        return -1;
    }
    freeaddrinfo(result);

    // while the connection is up
    while (true) {
        char buf[BUF_SIZE];

        // get the message to send
        fgets(buf, BUF_SIZE, stdin);

        int buf_size = strlen(buf);
        if (strlen(buf) == 2 && buf[0] == 'Q')
            break;

        // send the message
        int bytes = send(socket_dc, buf, buf_size, 0);
        if (bytes <= 0)
            break;

        char response[BUF_SIZE];
        memset(response, 0, BUF_SIZE);

        // receive echo message
        bytes = recv(socket_dc, response, BUF_SIZE, 0);
        if (bytes <= 0)
            break;
        std::cout << response;
    }

    // close socket connection
    close(socket_dc);

    return 0;
}