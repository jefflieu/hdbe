#!/bin/bash
clang --target=avr -O3  -fno-slp-vectorize -S -emit-llvm -c -o $1.03.ll $1.c
clang --target=avr -O2  -fno-slp-vectorize -S -emit-llvm -c -o $1.02.ll $1.c
clang --target=avr -O1  -fno-slp-vectorize -S -emit-llvm -c -o $1.01.ll $1.c
clang --target=avr -O0  -fno-slp-vectorize -S -emit-llvm -c -o $1.00.ll $1.c


