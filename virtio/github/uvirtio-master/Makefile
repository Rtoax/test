CC=gcc
CFLAGS=-Iinclude -lrt
OBJ = virtio_server.o virtio_client.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

uvirtio: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -f $(OBJ) uvirtio
