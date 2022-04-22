#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define NAME_SIZE 80
#define SIZE_RD_BUF 256

class Jugador: public Serializable
{
public:
    Jugador(const char * _n, int16_t _x, int16_t _y):x(_x),y(_y)
    {
        strncpy(name, _n, 80);
    };

    virtual ~Jugador(){};

    void to_bin(){
                        //size of x , y and name
        int32_t size = 2 *sizeof(int16_t) + NAME_SIZE* sizeof(char);

        alloc_data(size);

        char* tmp = _data;

        memcpy(tmp, name, NAME_SIZE * sizeof(char));

        tmp+= NAME_SIZE * sizeof(char);

        memcpy(tmp, &x, sizeof(int16_t));

        tmp+= sizeof(int16_t);

        memcpy(tmp, &y, sizeof(int16_t));
    }

    int from_bin(char * data) {
        
        return 0;
    }


public:
    char name[NAME_SIZE];

    int16_t x;
    int16_t y;
};

int main(int argc, char **argv)
{
    Jugador one_r("", 0, 0);
    Jugador one_w("Player_ONE", 123, 987);

    const char* name = "./jugador.bin";

    // 1. Serializar el objeto one_w
    one_w.to_bin();

    // 2. Escribir la serialización en un fichero
    int fd = open(name, O_CREAT|O_TRUNC | O_RDWR, 0666);
    write(fd, one_w.data(), one_w.size());

    close(fd);
    // 3. Leer el fichero
    // char buff[SIZE_RD_BUF];

    // fd = open(name, O_RDONLY);

    // read(fd, buff, SIZE_RD_BUF);

    // close(fd);
    // one_r.from_bin(buff);
    
    // 4. "Deserializar" en one_r

    
    // 5. Mostrar el contenido de one_r

    return 0;
}

