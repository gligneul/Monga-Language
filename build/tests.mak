# Monga
# Author: Gabriel de Quadros Ligneul

# This makefile runs the tests

all: \
	tests/ast/done \
	tests/monga/done \
	tests/parser/done \
	tests/scanner/done \
	tests/semantic/return/done \
	tests/semantic_test/done

tests/ast/done: bin/ast_test
tests/monga/done: bin/monga
tests/parser/done: bin/parser_test
tests/scanner/done: bin/scanner_test
tests/semantic/return/done: bin/semantic_test
tests/semantic_test/done: bin/semantic_test

tests/%:
	@build/test.sh $(@D) $< $(TEST_OPTIONS) && touch $@

