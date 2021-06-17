linenoise_example.out: linenoise.h linenoise.c

linenoise_example.out: linenoise.c example.c
	$(CC) -Wall -W -Os -g -o linenoise_example.out linenoise.c example.c

clean:
	rm -f linenoise_example.out history.txt
