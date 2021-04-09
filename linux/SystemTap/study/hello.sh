#!/bin/bash
stap -ve 'probe begin{printf("Hello, World\n"); exit();}'
