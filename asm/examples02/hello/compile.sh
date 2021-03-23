#!/bin/bash

nasm -felf64 hello.asm 
ld hello.o 
./a.out 
