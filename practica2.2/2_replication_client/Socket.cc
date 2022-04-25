#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char * address, const char * port):sd(-1) {
    //Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    // translate name to socket addresses
    addrinfo hints; 
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));
    memset(&result, 0, sizeof(addrinfo));

    hints.ai_flags    = AI_PASSIVE; // 0.0.0.0
    hints.ai_family   = AF_INET;    // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    int rc = getaddrinfo(address, port, &hints, &result);
    if (rc != 0) {
        const char* error_gai = gai_strerror(rc);
        std::string error = "Error: getaddrinfo -> " + (std::string)error_gai +  "\n"; 
        throw error;
    }

    // open socket with result content. Always 0 to TCP and UDP
    sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sd == -1) {
        std::string error = "Error: failure on socket open.\n"; 
        throw error;
    }

    //Con el resultado inicializar los miembros sd, sa y sa_len de la clase
    // associate address. Where is going to listen
    sa = (struct sockaddr)*(result->ai_addr);
    sa_len = (result->ai_addrlen);
}

int Socket::recv(Serializable &obj, Socket* &sock)
{
    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);

    if(bytes <= 0)
        return -1;

    if(sock != 0)
        sock = new Socket(&sa, sa_len);
    
    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
    //Serializar el objeto
    obj.to_bin();
    
    //Enviar el objeto binario a sock usando el socket sd
    int error_code = sendto(sd, obj.data(), obj.size(), 0, &sock.sa, sock.sa_len);

    return error_code;
}

bool operator== (const Socket &s1, const Socket &s2)
{
    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    sockaddr_in* sock1 = (sockaddr_in*) &(s1.sa);
    sockaddr_in* sock2 = (sockaddr_in*) &(s2.sa);

    //Retornar false si alguno difiere
    return sock1->sin_family == sock2->sin_family && 
        sock1->sin_addr.s_addr == sock2->sin_addr.s_addr &&
        sock1->sin_port == sock2->sin_port;
};

bool operator!= (const Socket &s1, const Socket &s2){
    return !(s1 == s2); 
};

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr*) &(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};