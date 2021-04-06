


MM=${mm}

all:memcloober_dynamic test

memclobber: memcloober_dynamic

memcloober_dynamic:
	gcc memclobber.c memhook.c -I. -fPIC -shared -o libmemclobber.so -ldl

test:
	@echo "Allocator is MM=${MM}"
	gcc vos-test.c ${MM} vos.c -I. -pthread -DVOS_DEBUG

clean:
	rm -f *.o *.a *.so *.out

