CC = g++
CFLAGS = -Wall -g -pthread

OUTPUT = serialization_player
TARGET = ex_serialization

all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $(OUTPUT).out $(TARGET).cc

clean: 
	-rm -f *.o $(OUTPUT).out

#https://www.techbeamers.com/makefile-tutorial-create-client-server-program/
#https://stackoverflow.com/questions/48261680/how-can-i-create-makefile-for-gcc-on-a-server-client-program	