test:
	$(CC)  -DMW_STDIO test.c memwatch.c -g	-o test.out
	$(CC)  -DMW_STDIO test-tasks.c memwatch.c -g -pthread -o test-tasks.out

clean:
	rm -f memwatch.log *.out
	