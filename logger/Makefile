.PHONY: all test clean

CXX := g++
CXXFLAG := -g -std=c++11
LDFLAGS := -lpthread -luuid
INCLUDE = -I ./ \
          -I ./src \
		  -I ../thirdpkg/include \
		  -I ../ \

test:
	mkdir -p output
	mkdir -p output/bin
	cp -r conf output/conf
	$(CXX) $(CXXFLAG) $(LDFLAGS) $(INCLUDE) test/TestFileAppender.cpp src/file_appender.cpp -o output/bin/TestFileAppender
	$(CXX) $(CXXFLAG) $(LDFLAGS) $(INCLUDE) test/TestLogger.cpp src/file_appender.cpp src/logger.cpp -o output/bin/TestLogger

clean:
	rm -rf output