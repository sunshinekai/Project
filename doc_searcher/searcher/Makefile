FLAG=-std=c++11 -ljsoncpp

.PHONY:all
all:index_test searcher_test libsearcher.so

index_test:searcher.cpp index_test.cpp
	g++ $^ -o $@ $(FLAG)
searcher_test:searcher_test.cpp searcher.cpp
	g++ $^ -o $@ $(FLAG)

libsearcher.so:searcher.cpp searcher.h
	g++ searcher.cpp -o $@ -shared -fPIC $(FLAG)

.PHONY:clean
clean:
	rm index_test searcher_test libsearcher.so