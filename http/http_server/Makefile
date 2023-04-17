.PHONY: all test clean

CXX := g++
CXXFLAG := -g -std=c++11

# 由于libhttpserver.a依赖liblogger.a, 因此这两者顺序不可颠倒
LDFLAGS := ../thirdpkg/lib/jsoncpp/libjsoncpp.a \
		   -luuid \
		   ./lib/libhttpserver.a \
		   ../logger/lib/liblogger.a \
		   -L../thirdpkg/lib/curl -lcurl \

INCLUDE = -I ../thirdpkg/include \
		  -I ./src \
		  -I ../logger/include \
		  -I ../ \
		  -I ./include \

all:
	mkdir -p include
	mkdir -p lib
	cp src/*.h include/
	g++ -c ${CXXFLAG} ${INCLUDE} src/http_request.cpp -o lib/http_request.o
	g++ -c ${CXXFLAG} ${INCLUDE} src/http_response.cpp -o lib/http_response.o
	g++ -c ${CXXFLAG} ${INCLUDE} src/http_server.cpp -o lib/http_server.o
	g++ -c ${CXXFLAG} ${INCLUDE} src/epoll_socket.cpp -o lib/epoll_socket.o
	g++ -c ${CXXFLAG} ${INCLUDE} src/http_epoll_event_handler.cpp -o lib/http_epoll_event_handler.o
	ar -rcs libhttpserver.a lib/*.o
	mv libhttpserver.a lib/
	rm -rf lib/*.o

test:
	mkdir -p output
	mkdir -p output/bin
	g++ ${CXXFLAG} ${INCLUDE} ../util/string_util/string_util.cpp test/http_server_test.cpp ${LDFLAGS} -o output/bin/http_server_test

clean:
	rm -rf output