#!/bin/bash

cat printf-format.txt | \
	grep "^%\([-+ #0]+\)\?\([\\d]+|\\*\)\?\(\\.\(\\d+|\\*\)\)\?\(hh|h|l|ll|j|z|Z|t|L\)\?\([diuoxXfFeEgGaAcspn]\)\?"
