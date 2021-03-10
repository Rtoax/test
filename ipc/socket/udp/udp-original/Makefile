ALL:
	gcc server.c common.c -o server.out -lm
	gcc client.c common.c -o client.out -lm
	gcc client-rx-loop.c common.c -o client-rx-loop.out -lm
	gcc client-tx-loop.c common.c -o client-tx-loop.out -lm
	gcc client-txrx.c common.c -o client-txrx.out -lm
clean:
	rm *~ *.out -f
