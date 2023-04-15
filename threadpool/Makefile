.PHONY: test clean

test:
	mkdir -p output
	mkdir -p output/bin
	g++ -g -std=c++11 -I .. example/threadpool_example.cpp -o output/bin/threadpool_example -lpthread

clean:
	rm -rf output