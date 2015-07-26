# PUC-Rio
# INF1715 Compiladores
# Gabriel de Quadros Ligneul 1212560

# This makefile creates the objects and the executables.

CFLAGS=-iquotesrc -ansi -Wall -Wextra -Wshadow -Werror -O2
#CFLAGS=-iquotesrc -ansi -Wall -Wextra -Wshadow -Werror -g

all: \
	bin/scanner_test \
	bin/parser_test \
	bin/ast_test \
	bin/semantic_test \
	bin/locations_test \
	bin/mc

bin/scanner_test: \
	obj/scanner/scanner.o \
	obj/scanner/scanner_test.o \
	obj/util/error.o \
	obj/util/new.o \
	obj/util/table.o \
	obj/util/vector.o

bin/parser_test: \
	obj/ast/ast.o \
	obj/ast/type.o \
	obj/parser/parser.tab.o \
	obj/parser/parser_test.o \
	obj/scanner/scanner.o \
	obj/util/error.o \
	obj/util/new.o \
	obj/util/table.o \
	obj/util/vector.o

bin/ast_test: \
	obj/ast/ast.o \
	obj/ast/ast_test.o \
	obj/ast/ast_print.o \
	obj/ast/type.o \
	obj/parser/parser.tab.o \
	obj/scanner/scanner.o \
	obj/util/error.o \
	obj/util/new.o \
	obj/util/table.o \
	obj/util/vector.o

bin/semantic_test: \
	obj/ast/ast.o \
	obj/ast/ast_print.o \
	obj/ast/type.o \
	obj/parser/parser.tab.o \
	obj/scanner/scanner.o \
	obj/semantic/semantic.o \
	obj/semantic/semantic_test.o \
	obj/semantic/symbols.o \
	obj/util/error.o \
	obj/util/new.o \
	obj/util/table.o \
	obj/util/vector.o

bin/locations_test: \
	obj/ast/ast.o \
	obj/ast/ast_print.o \
	obj/ast/type.o \
	obj/backend/locations.o \
	obj/backend/locations_test.o \
	obj/parser/parser.tab.o \
	obj/scanner/scanner.o \
	obj/semantic/semantic.o \
	obj/semantic/symbols.o \
	obj/util/error.o \
	obj/util/new.o \
	obj/util/table.o \
	obj/util/vector.o

bin/mc: \
	obj/mc.o \
	obj/ast/ast.o \
	obj/ast/ast_print.o \
	obj/ast/type.o \
	obj/backend/assembly.o \
	obj/backend/locations.o \
	obj/parser/parser.tab.o \
	obj/scanner/scanner.o \
	obj/semantic/semantic.o \
	obj/semantic/symbols.o \
	obj/util/error.o \
	obj/util/new.o \
	obj/util/table.o \
	obj/util/vector.o

bin/%:
	gcc -o $@ $^

include $(shell find obj -name "*.d")

obj/%.o: src/%.c
	gcc $(CFLAGS) -c -o $@ $<
