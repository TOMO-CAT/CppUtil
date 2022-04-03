.PHONY: test clean

test:
	mkdir -p output
	mkdir -p output/bin
	g++ -g -std=c++11 -I ./include test/TestThreadpool.cpp -o output/bin/TestThreadpool -lpthread

clean:
	rm -rf output