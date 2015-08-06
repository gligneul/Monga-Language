# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile creates the obj and bin folders.

all:
	@mkdir -p bin
	@mkdir -p `find src -type d | sed -e "s/^src/obj/"`

