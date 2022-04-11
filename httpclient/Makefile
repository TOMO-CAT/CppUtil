.PHONY: all test clean

CXX := g++
CXXFLAG := -g -std=c++11
LDFLAGS := ../logger/lib/liblogger.a \
		   -L../thirdpkg/lib/curl -lcurl \

INCLUDE = -I ../thirdpkg/include \
		  -I ./src \
		  -I ../logger/include \
		  -I ./include \

all:
	mkdir -p include
	mkdir -p lib
	cp src/*.h include/
	g++ -c ${CXXFLAG} ${INCLUDE} src/http_parse.cpp -o lib/http_parse.o
	g++ -c ${CXXFLAG} ${INCLUDE} src/http_server.cpp -o lib/http_server.o
	g++ -c ${CXXFLAG} ${INCLUDE} src/epoll_socket.cpp -o lib/epoll_socket.o
	ar -rcs libhttpserver.a lib/*.o
	mv libhttpserver.a lib/
	rm -rf lib/*.o

test:
	mkdir -p output
	mkdir -p output/bin
	g++ ${CXXFLAG} ${INCLUDE} test/httpclient_test.cpp src/httpclient.cpp ${LDFLAGS} -o output/bin/httpclient_test

clean:
	rm -rf output