#include <stdio.h>

int main() {
	unsigned long nr = nr_free_pagecache_pages();
	printf("nr == %ld\n", nr);
}
