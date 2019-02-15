CFLAG = -std=c++0x -g 
LIB = -lpthread -lrt

CC = g++

all : server client

server : server.o EventLoop.o server.o Network.o
	${CC} -o $@ $^ ${LIB}

client : client.o EventLoop.o client.o Network.o
	${CC} -o $@ $^ ${LIB}

%.o : %.cpp
	${CC} -c $< ${CFLAG}

clean:
	rm -rf *.o
