#include <time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

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

    return true;
}

int main(int argc, char *argv[]) {
    if(argv[3][0] != 't' && argv[3][0] != 'd' && argv[3][0] != 'q'){
        std::cerr << "Comando no soportado.\n";
        return -1;
    }

    int socket_dc;

    addrinfo hints;
    addrinfo* result;

    memset(&hints, 0, sizeof(addrinfo));
    memset(&result, 0, sizeof(addrinfo));

    hints.ai_family   = AF_INET;    // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    // initialize connection
    if(!init_connection(argv[1], argv[2], hints, result, &socket_dc)) {
        std::cerr << "Error: failed connection init.\n";
        return -1;
    }
    freeaddrinfo(result);

    char buffer[BUF_SIZE];

    // server data
	sockaddr_in server;
	socklen_t server_length = sizeof(server);

	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);

    // send client instruction request
    sendto(socket_dc, argv[3], 2, 0, (struct sockaddr *)&server, server_length);

    // if requested 't' or 'd'
    if (argv[3][0] != 'q') {
        // receive the server message
	    int bytes = recvfrom(socket_dc, buffer, (BUF_SIZE - 1), 0, (struct sockaddr*)&server, &server_length);

	    if (bytes == -1) {
	    	std::cerr << "Comando no soportado.\n";
	    	return -1;
	    }

        // print the message
	    std::cout << buffer;
    }
    else // upon closure
	    std::cout << "Servidor cerrado con exito.\n";

    // close socket connection
    close(socket_dc);

    return 0;
}