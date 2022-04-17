#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500
#define BACKLOG 5

bool init_connection(const char* host, char* serv, addrinfo hints, addrinfo *result, int* socket_dc) {
    // translate name to socket addresses
    int error_code = getaddrinfo(host, serv, &hints, &result);
    if (error_code != 0) {
        std::cerr << "Error: getaddrinfo -> " << gai_strerror(error_code) << "\n";
        return -1;
    }

    // open socket with result content. Always 0 to TCP and UDP
    (*socket_dc) = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if ((*socket_dc) == -1) {
        std::cerr << "Error: failure on socket open.\n";
        return false;
    }

    // associate address. Where is going to listen
    error_code = bind((*socket_dc), (struct sockaddr *) result->ai_addr, result->ai_addrlen);
    if (error_code != 0) {
        std::cerr << "Error: bind -> " << gai_strerror(error_code) << "\n";
        return -1;
    }

    // put the server on listen state
    listen(*socket_dc, BACKLOG);

    return true;
}

int main(int argc, char *argv[]) {
    
    int error_code, socket_dc;
    
    addrinfo hints;
    addrinfo* result;

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

    // client data
    struct sockaddr client;
    socklen_t client_length = sizeof(client);
    char host[NI_MAXHOST], serv[NI_MAXSERV];

    // waits for a new connection (and so, it blocks the execution)
    // gets connection-established socket
    int client_fd = accept(socket_dc, (struct sockaddr*)&client, &client_length);
    if(client_fd == -1) {
        std::cerr << "Error: accept -> " << gai_strerror(client_fd) << "\n";
        return -1;
    }

    // info of new connection
    getnameinfo((struct sockaddr *) &client, client_length, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST| NI_NUMERICSERV);
    std::cout << "Conexión desde " << host << ":" << serv << "\n";

    // while the connection is up
    bool exit = false;
    while (!exit) {
        char buf[BUF_SIZE];

        // receive message from client
        int bytes = recv(client_fd, buf, BUF_SIZE, 0);

        exit = (bytes <= 0);
        if(exit) // if connection is closed
            continue;

        buf[bytes]='\0'; 

        // send echo message (with message received)
        error_code = send(client_fd, buf, bytes, 0);

        // in case an error occurs upon sending the echo
        exit = (error_code == -1);
    }
    
    std::cout << "Conexión terminada.\n";

    // close socket connection
    close(socket_dc);

    return 0;
}