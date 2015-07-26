#!/bin/sh
# PUC-Rio
# INF1715 Compiladores
# Gabriel de Quadros Ligneul 1212560

./bin/mc < benchmark/m.mng > benchmark/m.s && \
gcc -m32 -c -o benchmark/m.o benchmark/m.s && \
gcc -m32 -c -o0 -o benchmark/c.o benchmark/c.c && \
gcc -m32 -c -o0 -o benchmark/main.o benchmark/main.c && \
gcc -m32 -o benchmark/benchmark benchmark/m.o benchmark/c.o benchmark/main.o && \
./benchmark/benchmark && \
rm benchmark/m.s benchmark/*.o benchmark/benchmark
