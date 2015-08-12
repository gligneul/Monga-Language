# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile runs the benchmarks

runs=$(wildcard benchmarks/*.sh)
binaries=$(patsubst %.sh,%.bin,$(runs))
benchmarks=$(patsubst %.sh,%.benchmark,$(runs))

all: $(benchmarks)

%_c.o: %.c
	gcc -Wall -Wextra -Werror -O2 -c -o $@ $<

%_mng.o: %.mng
	./bin/monga -dump -no-execution < $< |& llc -O2 | gcc -O2 -x assembler -c -o $@ -

%.bin: %_main_c.o %_c.o %_mng.o
	gcc -O2 -o $@ $^

%.benchmark: %.sh %.bin
	./$^

