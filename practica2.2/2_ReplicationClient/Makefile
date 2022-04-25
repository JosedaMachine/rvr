CC = g++
CFLAGS = -g -I.

DEPS = Socket.h Chat.h
OBJ = Socket.o Chat.o

LIBS = -lpthread

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: chat_server.out chat_client.out

chat_server.out: $(OBJ) main_chat_server.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

chat_client.out: $(OBJ) main_chat_client.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f *.o cs cc

