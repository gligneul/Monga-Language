# Monga
#
# Author: Gabriel de Quadros Ligneul

all:
	@make -f build/folders.mak
	@make -f build/sources.mak
	@make -f build/dependencies.mak
	@make -f build/objects.mak
	@make -f build/binaries.mak
	@echo "build succeeded"

tests: all
	@make -f build/tests.mak

benchmarks: all
	@make -f build/benchmarks.mak

clean_benchmarks:
	@make -f build/clean_benchmarks.mak

clean:
	rm -rf src/scanner/scanner.c src/parser/parser.tab.* obj/* bin/*
