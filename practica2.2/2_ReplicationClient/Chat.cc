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

    memcpy(tmp, &nick, NICK_SIZE);
    tmp += NICK_SIZE;

    memcpy(tmp, &message, MESSAGE_DATA_SIZE);
}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);
    memcpy(static_cast<void*>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data
    char *tmp = _data;

    memcpy(&type, tmp, sizeof(uint8_t));
    tmp += sizeof(uint8_t);

    memcpy(&nick, tmp, NICK_SIZE);
    tmp += NICK_SIZE;

    memcpy(&message, tmp, MESSAGE_DATA_SIZE);

    return 0;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void ChatServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */

        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
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

        // Enviar al servidor usando socket
        ChatMessage input_message(nick, msg);
        input_message.type = ChatMessage::MESSAGE;
        socket.send(input_message, socket);
    }
}

void ChatClient::net_thread()
{
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