# PUC-Rio
# INF1715 Compiladores
# Gabriel de Quadros Ligneul 1212560

all:
	@make -f build/lib.mak
	@make -f build/sources.mak
	@make -f build/folders.mak
	@make -f build/dependencies.mak
	@make -f build/objects.mak
	@echo "build succeeded"

clean:
	rm -rf src/scanner/scanner.c src/parser/parser.tab.* obj/* bin/*
