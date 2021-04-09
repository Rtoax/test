#!/bin/bash
#
stap -v -e 'probe vfs.read {printf("read performed\n"); exit()}'
