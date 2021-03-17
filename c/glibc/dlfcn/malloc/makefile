


MM=${mm}

all:vos mymalloc test

vos:
#gcc vos.c -I. -fPIC -shared -o libvos.a
	@echo MM=${MM}
	gcc -c vos.c -I. ${MM} 
	ar -crv libvos.a vos.o

mymalloc:
	gcc mymalloc.c -ldl -I. -fPIC -shared -o libmymalloc.so

test:
	@echo MM=${MM}
	gcc vos-test.c ${MM} libvos.a -I. 

clean:
	rm -f *.o *.a *.so
