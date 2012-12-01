CC = g++
CFLAGS =-O3 -Wall -std=c++0x
LDFLAGS =-L/lib -lboost_thread -lboost_system -lpthread

all: 
	$(CC) $(CFLAGS) $(LDFLAGS) client.cpp -o client
	$(CC) $(CFLAGS) $(LDFLAGS) server.cpp color.cpp -o server

clean:
	$(RM) client.o client.d server.o server.d client server

