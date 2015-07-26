# PUC-Rio
# INF1715 Compiladores
# Gabriel de Quadros Ligneul 1212560

# This makefile generates the yacc and lex output files.

all: src/scanner/scanner.c src/parser/parser.tab.c

src/scanner/scanner.c: src/scanner/scanner.l
	lex -o $@ $<

src/parser/parser.tab.c: src/parser/parser.y
	yacc -d -o $@ $<
