#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);
    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char *tmp = _data;

    memcpy(tmp, &type, sizeof(uint8_t));
    tmp += sizeof(uint8_t);

    memcpy(tmp, nick.c_str(), NICK_SIZE);
    tmp += NICK_SIZE;

    memcpy(tmp, message.c_str(), MESSAGE_DATA_SIZE);
}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);
    memcpy(static_cast<void*>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data
    char *tmp = _data;

    memcpy(&type, tmp, sizeof(uint8_t));
    tmp += sizeof(uint8_t);

    nick = tmp;
    tmp += NICK_SIZE;

    message = tmp;

    return 0;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void ChatServer::do_messages() {
    while (true) {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */
        ChatMessage client_message;
        Socket* client_socket = new Socket(socket);
        socket.recv(client_message, client_socket);

        switch (client_message.type) {
        case ChatMessage::LOGIN: { // - LOGIN: Añadir al vector clients
            printf("LOG IN: ");
            std::cout << client_message.nick << "\n";
            
            auto message_ptr = std::make_unique<Socket>(*client_socket); client_socket = nullptr;
            clients.push_back(std::move(message_ptr));

            break;
        }
        case ChatMessage::LOGOUT: { // - LOGOUT: Eliminar del vector clients
            printf("LOG OUT: ");
            std::cout << client_message.nick << "\n";

            auto clients_it = clients.begin();
            while(clients_it != clients.end() && *(*clients_it) != *client_socket) clients_it++;

            if(clients_it != clients.end()) clients.erase(clients_it);
            else std::cout << "Invalid socket: " << client_socket << "\n";
            
            break;
        }
        case ChatMessage::MESSAGE: { // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
            printf("MESSAGE: ");
            std::cout << client_message.nick << " -> " << client_message.message << "\n";

            for(auto clients_it = clients.begin(); clients_it != clients.end(); clients_it++)
                if(*(*clients_it) != *client_socket)
                    socket.send(client_message, **clients_it);
            break;
        }
        }
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void ChatClient::login()
{
    std::string msg;
    ChatMessage login_message(nick, msg);
    login_message.type = ChatMessage::LOGIN;

    socket.send(login_message, socket);
}

void ChatClient::logout()
{
    std::string msg;

    ChatMessage logout_message(nick, msg);
    logout_message.type = ChatMessage::LOGOUT;

    socket.send(logout_message, socket);
}

void ChatClient::input_thread()
{
    while(true) {
        // Leer stdin con std::getline
        std::string msg;
        std::getline(std::cin, msg);

        std::transform(msg.begin(), msg.end(), msg.begin(),
            [](unsigned char c){ return std::tolower(c); });

        if(msg == "logout" || msg == "log out" || msg == "q")
            break;

        // Enviar al servidor usando socket
        ChatMessage input_message(nick, msg);
        input_message.type = ChatMessage::MESSAGE;
        socket.send(input_message, socket);
    }

    logout();
}

void ChatClient::net_thread(){
    ChatMessage net_message;
    Socket* net_socket = new Socket(socket);
    while(true) {
        // Recibir Mensajes de red
        socket.recv(net_message, net_socket);

        // Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        std::cout << net_message.nick << ": " << net_message.message << "\n";
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------