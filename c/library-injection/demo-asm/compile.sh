#!/bin/bash

nasm -f elf64 inject_library.S -o inject_library.o
gcc main.c -c
ld -m elf_x86_64 inject_library.o main.o -o a.out
