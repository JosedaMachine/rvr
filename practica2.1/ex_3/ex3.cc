#include <time.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

bool createSocket(const char* host, char* serv, addrinfo hints, addrinfo *result, int* socketDesc){
    //Transalate name to socket addresses
    int rc = getaddrinfo(host, serv, &hints, &result);

    if (rc != 0) {
        std::cerr << "Error: getaddrinfo -> " << gai_strerror(rc) << "\n";
        return false;
    }

    //Open socket with result content. Always 0 to TCP and UDP
    (*socketDesc) = socket(result->ai_family, result->ai_socktype, 0);

    return true;
}

int main(int argc, char *argv[]){
    if(argv[3][0] != 't' && argv[3][0] != 'd' && argv[3][0] != 'q'){
        std::cerr << "Comando no soportado.\n";
        return -1;
    }

    int socketDesc;

    addrinfo hints;
    addrinfo* result;

    memset(&hints, 0, sizeof(addrinfo));
    memset(&result, 0, sizeof(addrinfo));

    hints.ai_family   = AF_INET;    // IPv4
    hints.ai_socktype = SOCK_DGRAM;

    //Initialize connection and listening
    if(!createSocket(argv[1], argv[2], hints, result, &socketDesc)){
        std::cerr << "Error: Initialization\n";
        return -1;
    }

    char buffer[BUF_SIZE];

	sockaddr_in servidor;
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(atoi(argv[2]));
	servidor.sin_addr.s_addr = inet_addr(argv[1]);

    sendto(socketDesc, argv[3], 2, 0, (struct sockaddr *)&servidor,
				 sizeof(servidor));

    if (argv[3][0] != 'q'){
	    socklen_t servidor_length;
	    int bytes = recvfrom(socketDesc, buffer, (BUF_SIZE - 1), 0,
	    					(struct sockaddr *)&servidor, &servidor_length);

	    if (bytes == -1) {
	    	std::cerr << "Comando no soportado.\n";
	    	return -1;
	    }

	    std::cout << buffer << '\n';
    }
    else
	    std::cout << "Servidor cerrado con exito\n";

    close(socketDesc);
    freeaddrinfo(result);
    return 0;
}