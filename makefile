CC="g++-6" -g -std=c++11 -I include/ -O3

all: prog

prog: incremental.o object.o
	$(CC) src/main.cpp src/*.o -o test

object.o: src/object.cpp include/object.hpp
	$(CC) src/object.cpp -c -o src/object.o

src/object.cpp:

include/object.hpp:

incremental.o: src/incremental.cpp include/incremental.hpp
	$(CC) src/incremental.cpp -c -o src/incremental.o

src/incremental.cpp:

include/incremental.hpp:
