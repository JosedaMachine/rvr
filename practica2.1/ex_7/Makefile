CC = g++
CFLAGS = -Wall -g -pthread

TARGET = ex7
OBJS = $(TARGET).o

all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o : %.cc MessageThread.cc
	$(CC) -c $(CFLAGS) $< -o $@

clean: 
	-rm -f *.o $(TARGET) 

#https://www.techbeamers.com/makefile-tutorial-create-client-server-program/
#https://stackoverflow.com/questions/48261680/how-can-i-create-makefile-for-gcc-on-a-server-client-program	
