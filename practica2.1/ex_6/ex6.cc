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
#define NUM_THREADS 3

class ConnectionThread
{
public:
    ConnectionThread(int sockdesc_) {
        socket_dc = sockdesc_;
        time(&rawtime);
    }

    ~ConnectionThread() {
        std::cout << "Thread " << std::this_thread::get_id() << " closed.\n";
    }

    void proccess_connection() {

        std::cout << "Thread " << std::this_thread::get_id() << " connected.\n";

        int error_code;

        struct sockaddr client;
        socklen_t client_length = sizeof(client);
        char host[NI_MAXHOST], serv[NI_MAXSERV];

        bool exit = false;
        while (!exit)  {
            char buf[BUF_SIZE];

            // receive message
            int bytes = recvfrom(socket_dc, buf, BUF_SIZE, 0, &client, &client_length);

            if (bytes == -1) // error receiving message
                std::cerr << "Error: Thread " << std::this_thread::get_id() << ": -> recvfrom.\n"; 
            else if(!bytes) // no message
                continue;      

            buf[bytes]='\0'; 

            // artificial wait between receptions
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // get client information
            error_code = getnameinfo(&client, client_length, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST| NI_NUMERICSERV);
            if (error_code != 0) {
                std::cerr << "Error: Thread " << std::this_thread::get_id() <<  " -> getnameinfo -> " << gai_strerror(error_code) << "\n";
                return;
            }

            std::cout << "== Thread [" << std::this_thread::get_id() << "]: ==\n";
            std::cout << bytes << " bytes de " << host << ":" << serv << "\n";

            // process message
            bytes = 0;
            switch (buf[0]) {
                case 't':
                    timeinfo = localtime(&rawtime);
                    bytes = strftime(buf, BUF_SIZE, "%I:%M:%S %p\n", timeinfo);
                    break;
                case 'd':
                    timeinfo = localtime(&rawtime);
                    bytes = strftime(buf, BUF_SIZE, "%F\n", timeinfo);
                    break;
                default:
                    std::cout << "Thread: " << std::this_thread::get_id() << ".\n";
                    std::cout << "Comando no soportado " << buf[0] << "\n";
            }

            // send returning message if 
            if(bytes != 0)
                sendto(socket_dc, buf, bytes, 0,&client, client_length);
        }
    }

private:
    int socket_dc;
    time_t rawtime;
    struct tm *timeinfo;
};

bool init_connection(const char* host, char* serv, addrinfo hints, addrinfo *result, int* socket_dc) {
    // translate name to socket addresses
    int rc = getaddrinfo(host, serv, &hints, &result);
    if (rc != 0) {
        std::cerr << "Error: getaddrinfo -> " << gai_strerror(rc) << "\n";
        return false;
    }

    // open socket with result content. Always 0 to TCP and UDP
    (*socket_dc) = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if ((*socket_dc) == -1) {
        std::cerr << "Error: failure on socket open.\n";
        return false;
    }

    // associate address. Where is going to listen
    rc = bind((*socket_dc), result->ai_addr, result->ai_addrlen);
    if (rc != 0) {
        std::cerr << "Error: bind -> " << gai_strerror(rc) << "\n";
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

    hints.ai_flags    = AI_PASSIVE; // 0.0.0.0
    hints.ai_family   = AF_INET;    // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    // initialize connection
    if(!init_connection(argv[1], argv[2], hints, result, &socket_dc)) {
        std::cerr << "Error: failed connection init.\n";
        return -1;
    }
    freeaddrinfo(result);

    // open all threads
    int i = 0;
    while(i < NUM_THREADS) {
        ConnectionThread *ct = new ConnectionThread(socket_dc);
        std::thread([&ct](){
            ct->proccess_connection();
            delete ct;
        }).detach();

        i++;
    }

    // wait for 'q' to end 
    char buf[BUF_SIZE];
    do {
        memset(buf, 0, BUF_SIZE);
        fgets(buf, BUF_SIZE, stdin);
    } while (buf[0] != 'q');
    
    // close all threads
    close(socket_dc);
    std::cout << "All threads closed.\n";

    return 0;
}