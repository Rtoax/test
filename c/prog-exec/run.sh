#!/bin/bash

GDB_FILE=tmp.gdb.prog.txt

gcc program.c

entrypoint=$(gdb a.out --quiet < gdb.prog.txt | grep "Entry" | awk '{print $3}')
echo $entrypoint | sed 's/^/[EntryPoint] /g'

echo "info files" > $GDB_FILE
echo "b *$entrypoint" >> $GDB_FILE
echo "run" >> $GDB_FILE

gdb a.out --quiet < $GDB_FILE | sed 's/^/[GDB] /g'
