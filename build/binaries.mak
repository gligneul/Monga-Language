# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile creates the executables

LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine jit interpreter analysis native bitwriter --system-libs`

all: \
	bin/scanner_test \
	bin/parser_test \
	bin/ast_test \
	bin/semantic_test \
    bin/monga

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

bin/monga: \
    obj/monga.o \
	obj/ast/ast.o \
	obj/ast/ast_print.o \
	obj/ast/type.o \
	obj/backend/ir.o \
	obj/parser/parser.tab.o \
	obj/scanner/scanner.o \
	obj/semantic/semantic.o \
	obj/semantic/symbols.o \
	obj/util/error.o \
	obj/util/new.o \
	obj/util/table.o \
	obj/util/vector.o

bin/%:
	clang++ -o $@ $^ $(LDFLAGS) 

