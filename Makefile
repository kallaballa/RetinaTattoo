CXX = g++-4.7
CXXFLAGS =-c -O0 -ggdb -Wall -std=c++0x
LDFLAGS =-L/lib64 -lboost_thread -lboost_system -lpthread
SOURCES=server.cpp client.cpp color.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) server client
	
client: client.o
	$(CXX) $(LDFLAGS) client.o -o $@

server: server.o
	$(CXX) $(LDFLAGS) server.o -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@


#clean:
#	$(RM) client.o client.d server.o server.d client server

