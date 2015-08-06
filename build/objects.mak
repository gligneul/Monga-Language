# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile creates the objects

CFLAGS=-iquotesrc -std=c99 -Wall -Wextra -Wshadow -Werror -g \
	 `llvm-config --cflags | sed 's/-O./-O0/'`

all: $(patsubst src/%.c,obj/%.o,$(shell find src -name "*.c"))

include $(shell find obj -name "*.d")

# fix compilation caused by lint define in yacc output
obj/parser/parser.tab.o: src/parser/parser.tab.c
	clang -Dlint $(CFLAGS) -c -o $@ $<

obj/%.o: src/%.c
	clang $(CFLAGS) -c -o $@ $<
