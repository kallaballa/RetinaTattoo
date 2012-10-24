CC = g++
CFLAGS =-O0 -g3 -Wall -std=c++0x
LDFLAGS =-L/lib64 -lboost_thread -lboost_system -lpthread

all: 
	$(CC) $(CFLAGS) $(LDFLAGS) client.cpp color.cpp -o client
	$(CC) $(CFLAGS) $(LDFLAGS) server.cpp -o server

clean:
	$(RM) client.o client.d server.o server.d client server

