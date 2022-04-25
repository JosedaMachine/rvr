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
void ChatServer::do_messages() {
    while (true) {
        
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------