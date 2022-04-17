#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 80
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
            std::cerr << "Error: Thread :" << std::this_thread::get_id() << " ->" << "recvfrom.\n"; 
    
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

    int getDesc(){
        return sockDesc;
    }

private:
  int sockDesc;
  time_t rawtime;
  struct tm * timeinfo;

};
