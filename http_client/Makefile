.PHONY: all test clean

CXX := g++
CXXFLAG := -g -std=c++11
LDFLAGS := ../logger/lib/liblogger.a \
		   -L../thirdpkg/lib/curl -lcurl \
		   -luuid \

INCLUDE = -I ../thirdpkg/include \
		  -I ./src \
		  -I ../logger/include \
		  -I ./include \

test:
	mkdir -p output
	mkdir -p output/bin
	g++ ${CXXFLAG} ${INCLUDE} -D_HTTP_CLIENT_DEBUG=true test/httpclient_test.cpp src/httpclient.cpp ${LDFLAGS} -o output/bin/httpclient_test

clean:
	rm -rf output