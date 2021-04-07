CC=gcc
CFLAGS=-Iinclude -lrt
OBJ = virtio_server.o virtio_client.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

uvirtio: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
	gcc virtio_server.o virtio_client.o test-tasks.c $(CFLAGS) -pthread -o test-tasks.out
	
clean:
	rm -f $(OBJ) uvirtio test-tasks.out