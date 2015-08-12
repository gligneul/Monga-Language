# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile runs the benchmarks

mains=$(wildcard benchmarks/*_main.c)
binaries=$(patsubst %_main.c,%.b,$(mains))
benchmarks=$(patsubst %.b,%.benchmarks,$(binaries))

all: $(benchmarks)

%_c.o: %.c
	@gcc -Wall -Wextra -Werror -O2 -c -o $@ $<

%_mng.o: %.mng
	@./bin/monga -dump -no-execution < $< |& llc -O2 | gcc -O2 -x assembler -c -o $@ -

%.b: %_main_c.o %_c.o %_mng.o
	@gcc -O2 -o $@ $^

%.benchmarks: %.b
	@./$<
