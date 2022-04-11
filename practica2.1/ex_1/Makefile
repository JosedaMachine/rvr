TARGET = program
CC = gcc
DEBUG = -g
CFLAGS = -Wall $(DEBUG) -pthread
OBJS = ex1.cc ex1.o
#SOURCES = $(addsuffix .c, $(basename $(OBJS)))
# HEADERS = mytar.h

all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

#.c.o: 
#	$(CC) $(CFLAGS) -c  $< -o $@


clean: 
	-rm -f *.o $(TARGET) 

#https://www.techbeamers.com/makefile-tutorial-create-client-server-program/
#https://stackoverflow.com/questions/48261680/how-can-i-create-makefile-for-gcc-on-a-server-client-program	
