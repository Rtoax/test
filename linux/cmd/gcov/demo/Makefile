

all:test run

test:
	@echo "[compile]---------------------------------------"
	gcc -fprofile-arcs -ftest-coverage -o test test.c

run:
	@echo "[execute]---------------------------------------"
	./test
	@echo "[gcov test.c]---------------------------------------"
	gcov test.c
	@echo "[show]----------------------------------------"
	cat test.c.gcov | sed 's/^/>>  /'

clean:
	rm -rf test test.gcda  test.gcno test.c.gcov
