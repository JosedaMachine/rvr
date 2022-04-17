#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 80

class MessageThread
{
public:
    MessageThread(int sockdesc_, char* host_, char* serv_)
    {
        sockDesc = sockdesc_;
        host = host_; serv = serv_;
    }

    ~MessageThread()
    {
        close(sockDesc);
        std::cout << "Thread: " << std::this_thread::get_id() << "closed.\n";
    }

    void do_message()
    {
        bool exit = false;
        while (!exit)
        {
            // ---------------------------------------------------------------------- //
            // RECEPCIÓN DEL MENSAJE DEL CLIENTE //
            // ---------------------------------------------------------------------- //
            char buffer[80];

            int bytes = recv(sockDesc, buffer, sizeof(char) * BUF_SIZE, 0);

            std::cout << "== Thread [" << std::this_thread::get_id() << "]: ==\n";
            printf("%d bytes de %s:%s\n", bytes, host, serv);

            if ((exit = bytes <= 0 || send(sockDesc, buffer, bytes, 0) == -1))
                continue;
        }
    }

private:
    int sockDesc;
    char* host, *serv;
};
