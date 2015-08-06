# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile generates the yacc and lex output files.

all: src/scanner/scanner.c src/parser/parser.tab.c

src/scanner/scanner.c: src/scanner/scanner.l
	lex $< && \
	mv lex.yy.c $@

src/parser/parser.tab.c: src/parser/parser.y
	yacc -d -o $@ $<
