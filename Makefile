CC = g++
CFLAGS =-O0 -g3 -Wall
LDFLAGS =-L/lib64 -lboost_thread -lboost_system -lpthread

all: 
	$(CC) $(CFLAGS) $(LDFLAGS) -c client.cpp -o client
	$(CC) $(CFLAGS) $(LDFLAGS) -c server.cpp -o server

clean:
	$(RM) client.o client.d server.o server.d client server

