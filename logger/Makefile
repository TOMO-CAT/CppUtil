.PHONY: all test clean

CXX := g++
CXXFLAG := -g -std=c++11
LDFLAGS := -lpthread
INCLUDE = -I ./ \
          -I ./src \
		  -I ../thirdpkg/include \
		  -I ../ \

test:
	mkdir -p output
	mkdir -p output/bin
	cp -r conf output/conf
	$(CXX) $(CXXFLAG) $(LDFLAGS) $(INCLUDE) test/TestFileAppender.cpp src/file_appender.cpp -o output/bin/TestFileAppender

clean:
	rm -rf output