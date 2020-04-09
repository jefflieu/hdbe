#!/bin/bash
clang --target=arm -O3  -fno-slp-vectorize -S -emit-llvm -c -o $1.03.ll $1.c
clang --target=arm -O2  -fno-slp-vectorize -S -emit-llvm -c -o $1.02.ll $1.c
clang --target=arm -O1  -fno-slp-vectorize -S -emit-llvm -c -o $1.01.ll $1.c


