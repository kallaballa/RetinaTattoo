TARGET  := server
SRCS    := src/server.cpp src/color.cpp src/mapping.cpp src/heartbeat.cpp src/exception.cpp
OBJS    := ${SRCS:.cpp=.o} 
DEPS    := ${SRCS:.cpp=.dep} 

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	CXXFLAGS = -std=c++0x -pedantic -Wall -O3 -I/usr/local/include
else
	CXXFLAGS = -std=c++0x -pedantic -Wall -O3
endif

LIBS    = -lboost_thread -lboost_system -lpthread

.PHONY: all debug clean distclean 

all: release

release: ${TARGET}
release: client

debug: CXXFLAGS += -g3 -O0 -rdynamic
debug: LDFLAGS += -Wl,--export-dynamic
debug: LIBS+= -lbfd
debug: ${TARGET}
debug: client

client: src/client.cpp
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o client src/client.cpp ${LIBS}

${TARGET}: ${OBJS} 
	${CXX} ${LDFLAGS} -o $@ $^ ${LIBS} 

${OBJS}: %.o: %.cpp %.dep 
	${CXX} ${CXXFLAGS} -o $@ -c $< 

${DEPS}: %.dep: %.cpp Makefile 
	${CXX} ${CXXFLAGS} -MM $< > $@ 

clean:
	rm -f src/*~ src/*.o src/*.dep ${TARGET} client

distclean: clean

