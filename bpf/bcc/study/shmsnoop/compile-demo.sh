#!/bin/bash

function compile_demo() {
	folder=../../../../c/glibc/sys/shm/shm_rw
	make -C $folder
	mv $folder/*.out .
}

compile_demo
