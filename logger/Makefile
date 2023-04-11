.PHONY: all test clean

CXX := g++
CXXFLAG := -g -std=c++11 
LDFLAGS := -lpthread -L../thirdpkg/lib/uuid -luuid
INCLUDE = -I ./ \
          -I ./src \
		  -I ../thirdpkg/include \
		  -I ../ \

all:
	mkdir -p output/include
	mkdir -p output/lib
	cp ./*.h output/include
	$(CXX) $(CXXFLAG) $(INCLUDE) -c logger.cpp -o output/lib/logger.o
	$(CXX) $(CXXFLAG) $(INCLUDE) -c file_appender.cpp -o output/lib/file_appender.o
	ar -rcs liblogger.a output/lib/*.o
	mv liblogger.a output/lib/
	rm -rf output/lib/*.o

test:
	mkdir -p output
	mkdir -p output/bin
	cp -r conf output/conf

	# $(LDFLAGS) 必须放在最后, 否则会出现 `undefined reference to `uuid_generate'` 错误
	# $(CXX) $(CXXFLAG) $(INCLUDE) $(LDFLAGS) test/TestFileAppender.cpp file_appender.cpp -o output/bin/TestFileAppender
	$(CXX) $(CXXFLAG) $(INCLUDE) test/TestFileAppender.cpp file_appender.cpp -o output/bin/TestFileAppender $(LDFLAGS)
	$(CXX) $(CXXFLAG) $(INCLUDE) test/TestLogger.cpp file_appender.cpp logger.cpp -o output/bin/TestLogger $(LDFLAGS)

clean:
	rm -rf output