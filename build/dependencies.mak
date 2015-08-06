# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile generates the dependencies files.

CFLAGS=-iquotesrc `llvm-config --cflags`

all: $(patsubst src/%.c,obj/%.d,$(shell find src -name "*.c"))

obj/%.d: src/%.c
	clang $(CFLAGS) -MM $< -MT $(patsubst %.d,%.o,$@) -o $@

