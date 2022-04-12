.PHONY: all test clean

CXX := g++
CXXFLAG := -g -std=c++11
LDFLAGS := ../logger/lib/liblogger.a \
		   -luuid \
		   -lpthread \
		   -L ../thirdpkg/lib/gtest -lgtest -lgtest_main \
		   -L ../thirdpkg/lib/hiredis -lhiredis \
		   -Wl,-rpath=../thirdpkg/lib/hiredis \

INCLUDE = -I ../thirdpkg/include \
		  -I ./src \
		  -I ../logger/include \


test:
	mkdir -p output
	mkdir -p output/bin
	g++ ${CXXFLAG} ${INCLUDE} test/redis_proxy_test.cpp src/redis_proxy.cpp ${LDFLAGS} -o output/bin/redis_proxy_test

clean:
	rm -rf output