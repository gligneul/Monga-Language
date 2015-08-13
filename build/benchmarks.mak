# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile runs the benchmarks

cflags=-std=c99 -Wall -Wextra -Werror
opt=-O2

runs=$(wildcard benchmarks/*.sh)
binaries=$(patsubst %.sh,%.bin,$(runs))
benchmarks=$(patsubst %.sh,%.benchmark,$(runs))

all: $(benchmarks)

%.o: %.c
	gcc $(cflags) $(opt) -c -o $@ $<

%_gcc.o: %.c
	gcc -DCC=Gcc $(cflags) $(opt) -c -o $@ $<

%_clang.o: %.c
	clang -DCC=Clang $(cflags) $(opt) -c -o $@ $<

%_mng.o: %.mng
	./bin/monga -dump -no-execution < $< |& llc $(opt) | gcc $(opt) -x assembler -c -o $@ -

%.bin: %_main.o %_gcc.o %_clang.o %_mng.o
	gcc $(opt) -o $@ $^

%.benchmark: %.sh %.bin
	./$^

