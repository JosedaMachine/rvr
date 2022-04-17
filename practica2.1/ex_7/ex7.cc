#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <thread>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500
#define BACKLOG 16

class ConnectionThread
{
public:
    ConnectionThread(int sockdesc_, char* host_, char* serv_)  {
        socket_dc = sockdesc_;
        host = host_; serv = serv_;
    }

    ~ConnectionThread() {
        close(socket_dc);
        std::cout << "Thread: " << std::this_thread::get_id() << " closed.\n";
    }

    void proccess_connection()  {
        bool exit = false;
        while (!exit) {
            char buffer[80];
            
            // receive echo
            int bytes = recv(socket_dc, buffer, sizeof(char) * BUF_SIZE, 0);

            std::cout << "== Thread [" << std::this_thread::get_id() << "]: ==\n";
            std::cout << bytes << " bytes de " << host << ":" << serv << "\n";

            // send echo back
            if ((exit = bytes <= 0 || send(socket_dc, buffer, bytes, 0) == -1))
                continue;
        }
    }

private:
    int socket_dc;
    char* host, *serv;
};

bool init_connection(const char* host, char* serv, addrinfo hints, addrinfo *result, int* socket_dc) {
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

    // associate address. Where is going to listen
    error_code = bind((*socket_dc), (struct sockaddr *) result->ai_addr, result->ai_addrlen);
    if (error_code != 0) {
        std::cerr << "Error: bind -> " << gai_strerror(error_code) << "\n";
        return false;
    }

    if(listen(*socket_dc, BACKLOG) == -1) {
        std::cerr << "Error: listen\n";
		return false;
    }

    return true;
}

int main(int argc, char *argv[]) {

    int socket_dc;

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

    struct sockaddr client;
    socklen_t client_length = sizeof(client);
    char host[NI_MAXHOST], serv[NI_MAXSERV];

    bool exit = false;
    while(!exit){
        int sock_dc = accept(socket_dc, &client, &client_length);

        getnameinfo(&client, client_length, host, NI_MAXHOST, serv,
					NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        std::cout << "Connected from " << host << " " << serv << '\n';

        ConnectionThread *mt = new ConnectionThread(sock_dc, host, serv);
        std::thread([&mt](){
            mt->proccess_connection();
            delete mt;
        }).detach();
    }
    
    //close all threads
    close(socket_dc);
    std::cout << "Connection closed.\n";

    return 0;
}