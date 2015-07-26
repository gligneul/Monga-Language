# PUC-Rio
# INF1715 Compiladores
# Gabriel de Quadros Ligneul 1212560

# This makefile creates the libs binaries.

CFLAGS=-m32 -ansi -Wall -Wextra -Wshadow -Werror -O2

all: $(patsubst lib/src/%.c, lib/obj/%.o, $(wildcard lib/src/*.c))

lib/obj/%.o: lib/src/%.c
	gcc $(CFLAGS) -c -o $@ $<
