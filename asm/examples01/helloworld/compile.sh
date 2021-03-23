#To assemble, link and run this program under Windows:
#
#    nasm -fwin32 helloworld.asm
#    gcc helloworld.obj
#    a
#Under Linux, you'll need to remove the leading underscores from function names, and execute
#
#    nasm -felf helloworld.asm
#    gcc helloworld.o
#    ./a.out

nasm -felf64 helloworld.asm
gcc helloworld.o
./a.out
