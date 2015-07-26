# PUC-Rio
# INF1715 Compiladores
# Gabriel de Quadros Ligneul 1212560

# This makefile generates the dependencies files.

all: $(patsubst src/%.c,obj/%.d,$(shell find src -name "*.c"))

obj/%.d: src/%.c
	gcc -iquotesrc -MM $< -MT $(patsubst %.d,%.o,$@) -o $@
