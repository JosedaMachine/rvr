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

bool createSocket(const char* host, char* serv, addrinfo hints, addrinfo *result, int* socketDesc){
    //Transalate name to socket addresses
    int rc = getaddrinfo(host, serv, &hints, &result);

    if (rc != 0) {
        std::cerr << "Error: getaddrinfo -> " << gai_strerror(rc) << "\n";
        return -1;
    }

    //Open socket with result content. Always 0 to TCP and UDP
    (*socketDesc) = socket(result->ai_family, result->ai_socktype, 0);

    //Associate address. Where is going to listen
    rc = bind((*socketDesc), (struct sockaddr *) result->ai_addr, result->ai_addrlen);
    if (rc != 0) {
        std::cerr << "Error: bind -> " << gai_strerror(rc) << "\n";
        return -1;
    }

    listen(*socketDesc, BACKLOG);

    return true;
}

int main(int argc, char *argv[]){
    
    int error_code, socketDesc;
    
    addrinfo hints;
    addrinfo* result;

    memset(&hints, 0, sizeof(addrinfo));
    memset(&result, 0, sizeof(addrinfo));
    hints.ai_flags    = AI_PASSIVE; //Devolver 0.0.0.0
    hints.ai_family   = AF_INET;    // IPv4
    hints.ai_socktype = SOCK_STREAM;

    //Initialize connection and listening
    if(!createSocket(argv[1],argv[2], hints, result, &socketDesc)){
        std::cerr << "Error: Initiliazation\n";
        return -1;
    }

    struct sockaddr cliente;
    socklen_t cliente_len = sizeof(cliente);
    char buf[BUF_SIZE];
    char host[NI_MAXHOST], serv[NI_MAXSERV];
    bool exit = false;

    //Waits for a new connection
    //Gets connection-established socket. Block Call
    int clientFileDesc = accept(socketDesc, (struct sockaddr*)&cliente, &cliente_len);

    if(clientFileDesc == -1){
        std::cerr << "Error: accept -> " << gai_strerror(clientFileDesc) << "\n";
        return -1;
    }

    //Information of the new connection
    getnameinfo((struct sockaddr *) &cliente, cliente_len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST| NI_NUMERICSERV);
    printf("Conexión desde %s%s\n",host, serv);

    while (!exit) {
        int bytes = recv(clientFileDesc, buf, BUF_SIZE, 0);

        if(bytes <= 0) {
            exit = true;
            continue;
        }

        error_code = send(clientFileDesc, buf, bytes, 0);
        if(error_code == -1){
            exit = true;
            continue;
        }
    }
    printf("Conexión terminada.\n");
    close(socketDesc);
    freeaddrinfo(result);
    return 0;
}