#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>

#include "MessageThread.cc"

#define BUF_SIZE 80
#define BACKLOG 16

bool createSocket(const char* host, char* serv, addrinfo hints, addrinfo *result, int* socketDesc){
    //Transalate name to socket addresses
    int rc = getaddrinfo(host, serv, &hints, &result);

    if (rc != 0) {
        std::cerr << "Error: getaddrinfo -> " << gai_strerror(rc) << "\n";
        return false;
    }

    //Open socket with result content. Always 0 to TCP and UDP
    (*socketDesc) = socket(result->ai_family, result->ai_socktype, 0);

    //Associate address. Where is going to listen
    rc = bind((*socketDesc), (struct sockaddr *) result->ai_addr, result->ai_addrlen);
    if (rc != 0) {
        std::cerr << "Error: bind -> " << gai_strerror(rc) << "\n";
        return false;
    }

    if(listen(*socketDesc, BACKLOG) == -1){
        std::cerr << "Error: listen\n";
		return false;
    }

    return true;
}

int main(int argc, char *argv[]){
    int socketDesc;

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
    char host[NI_MAXHOST], serv[NI_MAXSERV];
    bool exit = false;

    while(!exit){
        int sockDescClient = accept(socketDesc, &cliente, &cliente_len);
        std::cout << sockDescClient << "\n";
        if(sockDescClient == -1) continue;

        getnameinfo(&cliente, cliente_len, host, NI_MAXHOST, serv,
					NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        std::cout << "Connected from " << host << " " << serv << '\n';

        MessageThread *mt = new MessageThread(sockDescClient, host, serv);
        std::thread([&mt](){
            mt->do_message();
            delete mt;
        }).detach();
    }
    
    //close all threads
    close(socketDesc);
    freeaddrinfo(result);
    std::cout << "Connection closed.\n";
    return 0;
}