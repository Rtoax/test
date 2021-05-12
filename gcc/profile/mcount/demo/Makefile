

all: recordmcount.out demo

recordmcount.out:
	make -C ../recordmcount
	cp ../recordmcount/recordmcount.out .

demo:
	gcc foo.c -pg -S -o foo-pg.s
	gcc foo.c -pg -c -o foo-pg.o
	gcc foo.c -S -o foo.s
	gcc foo.c -c -o foo.o

test:
#diff foo.s foo-pg.s
	./recordmcount.out ./foo.o

clean:
	make -C ../recordmcount clean
	rm -f foo.s foo-pg.s foo.o foo-pg.o recordmcount.out
