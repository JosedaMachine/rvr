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
#define NUM_THREADS 3

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
    rc = bind((*socketDesc), result->ai_addr, result->ai_addrlen);
    if (rc != 0) {
        std::cerr << "Error: bind -> " << gai_strerror(rc) << "\n";
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
    hints.ai_socktype = SOCK_DGRAM;

    if(!createSocket(argv[1],argv[2], hints, result, &socketDesc)){
        std::cerr << "Error: Initiliazation\n";
        return -1;
    }

    int i = 0;
    while(i < NUM_THREADS){
        MessageThread *mt = new MessageThread(socketDesc);
        std::thread([&mt](){
            mt->do_message();
            delete mt;
        }).detach();

        i++;
    }

    char buf[BUF_SIZE];
    do{
        memset(buf, 0, BUF_SIZE);
        fgets(buf, BUF_SIZE, stdin);
    } while (buf[0] != 'q');
    
    //close all threads
    close(socketDesc);
    freeaddrinfo(result);
    std::cout << "All threads closed.\n";
    return 0;
}