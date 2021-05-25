
all:
	g++ -g -std=c++11 -o test.out main.cc $(shell pkg-config --cflags --libs sane-backends) -ggdb
	gcc main.c -g -ggdb -o test.c.out $(shell pkg-config --cflags --libs sane-backends) -pthread

clean:
	rm -f test.out test.c.out
