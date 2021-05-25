
FLAGS=$(shell pkg-config --cflags --libs sane-backends) -pthread -ggdb -g

all:
	g++ -g -std=c++11 -o test.out main.cc ${FLAGS}
	gcc main.c  -o test.tcmalloc.out ${FLAGS} -ltcmalloc_debug
	gcc main.c  -o test.c.out ${FLAGS}

clean:
	rm -f test.out test.c.out
