#include <thread>

#include "Chat.h"

int main(int argc, char **argv)
{
    ChatClient ec(argv[1], argv[2], argv[3]);

    std::thread ([&ec](){
        ec.net_thread();
    }).detach();

    ec.login();

    ec.input_thread();
}