

all:a.out hello.out

hello.o:hello.asm
	nasm hello.asm -felf64

a.out:hello.o
	ld hello.o

clean:
	rm -f *.o *.out

hello.out:
	ld -T linker.script
	@objdump -D hello.out | grep -e "\.text" -e "\.data" -A 2 | sed 's/^/[objdump] /g'
