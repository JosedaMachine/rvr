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
#define MAX_RESPONSE_LEN 15

/*
  argv[0] ---> nombre del programa
  argv[1] ---> primer argumento (char *)
  ./time_server_multi 0.0.0.0 2222
    argv[0] = "./time_server_multi"
    argv[1] = "0.0.0.0"
    argv[2] = "2222"
      |
      |
      V
    res->ai_addr ---> (socket + bind)
*/

class Connection
{
	int fd_;

public:
	Connection(int fd) : fd_(fd) {}

	void run() const noexcept
	{
		// ---------------------------------------------------------------------- //
		// RECEPCIÓN MENSAJE DE CLIENTE //
		// ---------------------------------------------------------------------- //
		time_t time_;
		struct tm *tm_;

		char buffer[80];
		char host[NI_MAXHOST];
		char service[NI_MAXSERV];

		struct sockaddr client_addr;
		socklen_t client_len = sizeof(struct sockaddr);

		// Escribe algo de información sobre este thread:
		std::cout << "Thread [" << std::this_thread::get_id() << "]: Activo." << std::endl;

		bool exit = false;
		char response[MAX_RESPONSE_LEN];
		while (!exit)
		{
			// Recive un mensaje de un cliente:
			ssize_t bytes = recvfrom(fd_, buffer, 79 * sizeof(char), 0, &client_addr,
									 &client_len);

			// Si se ha producido un error, por ejemplo cuando el servidor se cierra,
			// bytes será asignado como -1, en ese caso cerraremos el loop ya que el
			// socket es potencialmente inválido:
			if (bytes == -1)
			{
				std::cerr << "Thread [" << std::this_thread::get_id() << "]: "
						  << "recvfrom: " << std::endl;
				break;
			}

			// Añadimos una pequeña espera:
			std::this_thread::sleep_for(std::chrono::seconds(1));

			// Escribe algo de información del cliente:
			getnameinfo(&client_addr, client_len, host, NI_MAXHOST, service,
						NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

			std::cout << "Thread [" << std::this_thread::get_id() << "]: "
					  << bytes << " bytes de " << host << ":" << service << std::endl;

			// Si no se mandaron suficientes bytes, continua:
			if (bytes == 0)
			{
				continue;
			}

			switch (buffer[0])
			{
			case 't':
				time(&time_);
				tm_ = localtime(&time_);
				bytes = strftime(response, MAX_RESPONSE_LEN, "%H:%M:%S %p", tm_);
				sendto(fd_, response, bytes, 0, &client_addr, client_len);
				break;
			case 'd':
				time(&time_);
				tm_ = localtime(&time_);
				bytes = strftime(response, MAX_RESPONSE_LEN, "%Y-%m-%d", tm_);
				sendto(fd_, response, bytes, 0, &client_addr, client_len);
				break;
			default:
				std::cout << "Thread [" << std::this_thread::get_id() << "]: "
						  << "Comando no soportado " << buffer[0] << std::endl;
			}
		}

		std::cout << "Thread [" << std::this_thread::get_id() << "]: Inactivo." << std::endl;
	}
};

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Debes proveer la dirección. Por ejemplo: 0.0.0.0"
				  << std::endl;
		return EXIT_FAILURE;
	}

	if (argc < 3)
	{
		std::cerr << "Debes proveer el puerto. Por ejemplo: 2222"
				  << std::endl;
		return EXIT_FAILURE;
	}

	struct addrinfo hints;
	struct addrinfo *res;

	// ---------------------------------------------------------------------- //
	// INICIALIZACIÓN SOCKET & BIND //
	// ---------------------------------------------------------------------- //

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	// res contiene la representación como sockaddr de dirección + puerto
	int rc = getaddrinfo(argv[1], argv[2], &hints, &res);

	if (rc != 0)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(rc) << std::endl;
		return EXIT_FAILURE;
	}

	int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (bind(sd, res->ai_addr, res->ai_addrlen) != 0)
	{
		std::cerr << "bind: " << std::endl;
		return EXIT_FAILURE;
	}

	// Libera la información de la dirección una vez ya hemos usado sus datos:
	freeaddrinfo(res);

	// Crea tantos threads como se hayan especificado, o 3 threads en caso de
	// no haberse escrito el argumento:
	int n_threads = argc >= 4 ? atoi(argv[3]) : 3;
	for (int i = 0; i < n_threads; ++i)
	{
		// Crea una conexión, el thread se encarga de liberar la memoria tan
		// pronto como haya terminado de trabajar:
		const auto *conn = new Connection(sd);
		std::thread([conn]() {
			conn->run();
			delete conn;
		}).detach();

		// Espera un segundo antes de crear el siguiente thread para evitar
		// posibles errores al llamar recvfrom de manera simultánea:
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::cout << "Creados con éxito " << n_threads << " threads." << std::endl;

	// Lee el input de la consola, si el usuario escribe 'q' en la consola, el
	// servidor debe cerrar:
	std::string v;
	do
	{
		std::cin >> v;
	} while (v != "q");

	// Cierra el socket:
	close(sd);

	return EXIT_SUCCESS;
}

class MessageThread {
public:
    MessageThread(int sockdesc_){
      sockDesc = sockdesc_;
      time(&rawtime);
    }

    ~MessageThread(){
        std::cout << "Thread: " << std::this_thread::get_id() << "closed.\n";
    }

    void do_message(){
      struct sockaddr cliente;
      socklen_t cliente_len = sizeof(cliente);
      char host[NI_MAXHOST], serv[NI_MAXSERV];
      bool exit = false;
      int error_code;

      while (!exit) {
        char buf[BUF_SIZE];

        //Receive message
        int bytes = recvfrom(sockDesc, buf, BUF_SIZE, 0, &cliente, &cliente_len);
        
        if (bytes == -1) 
            std::cerr << "Error: Thread " << std::this_thread::get_id() << ": -> recvfrom.\n"; 
    
        buf[bytes]='\0'; 
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        //Who's is sending message
        error_code = getnameinfo(&cliente, cliente_len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST| NI_NUMERICSERV);
        if (error_code != 0) {
            std::cerr << "Error: Thread :" << std::this_thread::get_id() <<  " -> getnameinfo -> " << gai_strerror(error_code) << "\n";
            return;
        }

        //No Mesage
        if(!bytes) 
            continue;      

        std::cout << "== Thread [" << std::this_thread::get_id() << "]: ==\n";
        printf("%d bytes de %s:%s\n", bytes, host, serv);
        
        bytes = 0;
        //Process message
        switch (buf[0]) {
            case 't':
                timeinfo = localtime(&rawtime);
                bytes = strftime (buf,BUF_SIZE,"%I:%M:%S %p.",timeinfo);
                break;
            case 'd':
                timeinfo = localtime(&rawtime);
                bytes = strftime (buf,BUF_SIZE,"%F",timeinfo);
                break;
            default:
                std::cout << "Thread: " << std::this_thread::get_id() << ".";
                std::cout << "Comando no soportado " << buf[0] << "\n";
        }

        if(bytes != 0)
            sendto(sockDesc, buf, bytes, 0,&cliente, cliente_len);
      }
    }

private:
  int sockDesc;
  time_t rawtime;
  struct tm * timeinfo;

};

class ConnectionThread
{
public:
    ConnectionThread(int sockdesc_) {
        socket_dc = sockdesc_;
        time(&rawtime);
    }

    ~ConnectionThread() {
        std::cout << "Thread: " << std::this_thread::get_id() << "closed.\n";
    }

    void proccess_connection() {
        int error_code;

        struct sockaddr cliente;
        socklen_t cliente_len = sizeof(cliente);
        char host[NI_MAXHOST], serv[NI_MAXSERV];

        bool exit = false;
        while (!exit)  {
            char buf[BUF_SIZE];

            // receive message from client
            int bytes = recvfrom(socket_dc, buf, BUF_SIZE, 0, &cliente, &cliente_len);

            if (bytes == -1)
                std::cerr << "Error: Thread " << std::this_thread::get_id() << ": -> recvfrom.\n";

            buf[bytes] = '\0';

            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Who's is sending message
            error_code = getnameinfo(&cliente, cliente_len, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
            if (error_code != 0)
            {
                std::cerr << "Error: Thread :" << std::this_thread::get_id() << " -> getnameinfo -> " << gai_strerror(error_code) << "\n";
                return;
            }

            // No Mesage
            if (!bytes)
                continue;

            std::cout << "== Thread [" << std::this_thread::get_id() << "]: ==\n";
            printf("%d bytes de %s:%s\n", bytes, host, serv);

            bytes = 0;
            // Process message
            switch (buf[0])
            {
            case 't':
                timeinfo = localtime(&rawtime);
                bytes = strftime(buf, BUF_SIZE, "%I:%M:%S %p.", timeinfo);
                break;
            case 'd':
                timeinfo = localtime(&rawtime);
                bytes = strftime(buf, BUF_SIZE, "%F", timeinfo);
                break;
            default:
                std::cout << "Thread: " << std::this_thread::get_id() << ".";
                std::cout << "Comando no soportado " << buf[0] << "\n";
            }

            if (bytes != 0)
                sendto(socket_dc, buf, bytes, 0, &cliente, cliente_len);
        }
    }

    int getDesc()
    {
        return socket_dc;
    }

private:
    int socket_dc;
    time_t rawtime;
    struct tm *timeinfo;
};

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

// int main(int argc, char *argv[]){
//     int socketDesc;

//     addrinfo hints;
//     addrinfo* result;

//     memset(&hints, 0, sizeof(addrinfo));
//     memset(&result, 0, sizeof(addrinfo));

//     hints.ai_flags    = AI_PASSIVE; //Devolver 0.0.0.0
//     hints.ai_family   = AF_INET;    // IPv4
//     hints.ai_socktype = SOCK_DGRAM;

//     if(!createSocket(argv[1],argv[2], hints, result, &socketDesc)){
//         std::cerr << "Error: Initiliazation\n";
//         return -1;
//     }

//     int i = 0;
//     while(i < NUM_THREADS){
//         MessageThread *mt = new MessageThread(socketDesc);
//         std::thread([&mt](){
//             std::cout<<"hi";
//             mt->do_message();
//             delete mt;
//         }).detach();

//         i++;
//     }

//     char buf[BUF_SIZE];
//     do{
//         memset(buf, 0, BUF_SIZE);
//         fgets(buf, BUF_SIZE, stdin);
//     } while (buf[0] != 'q');
    
//     //close all threads
//     close(socketDesc);
//     freeaddrinfo(result);
//     std::cout << "All threads closed.\n";
//     return 0;
// }

// bool init_connection(const char* host, char* serv, addrinfo hints, addrinfo *result, int* socket_dc) {
//     // translate name to socket addresses
//     int rc = getaddrinfo(host, serv, &hints, &result);
//     if (rc != 0) {
//         std::cerr << "Error: getaddrinfo -> " << gai_strerror(rc) << "\n";
//         return false;
//     }

//     // open socket with result content. Always 0 to TCP and UDP
//     (*socket_dc) = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//     if ((*socket_dc) == -1) {
//         std::cerr << "Error: failure on socket open.\n";
//         return false;
//     }

//     // associate address. Where is going to listen
//     rc = bind((*socket_dc), result->ai_addr, result->ai_addrlen);
//     if (rc != 0) {
//         std::cerr << "Error: bind -> " << gai_strerror(rc) << "\n";
//         return false;
//     }

//     return true;
// }

// int main(int argc, char *argv[]) {

//     int socket_dc;

//     addrinfo hints;
//     addrinfo* result;

//     memset(&hints, 0, sizeof(addrinfo));
//     memset(&result, 0, sizeof(addrinfo));

//     hints.ai_flags    = AI_PASSIVE; // 0.0.0.0
//     hints.ai_family   = AF_INET;    // IPv4
//     hints.ai_socktype = SOCK_DGRAM; // UDP

//     // initialize connection
//     if(!init_connection(argv[1], argv[2], hints, result, &socket_dc)) {
//         std::cerr << "Error: failed connection init.\n";
//         return -1;
//     }
//     freeaddrinfo(result);

//     // open all threads
//     int i = 0;
//     while(i < NUM_THREADS) {
//         ConnectionThread *ct = new ConnectionThread(socket_dc);
//         std::thread([&ct](){
//             ct->proccess_connection();
//             delete ct;
//         }).detach();

//         i++;
//     }

//     // wait for 'q' to end 
//     char buf[BUF_SIZE];
//     do {
//         memset(buf, 0, BUF_SIZE);
//         fgets(buf, BUF_SIZE, stdin);
//     } while (buf[0] != 'q');
    
//     // close all threads
//     close(socket_dc);
//     std::cout << "All threads closed.\n";

//     return 0;
// }