# PUC-Rio
# INF1715 Compiladores
# Gabriel de Quadros Ligneul 1212560

# This makefile creates the obj and bin folders.

all:
	mkdir -p bin
	mkdir -p `find src -type d | sed -e "s/^src/obj/"`
