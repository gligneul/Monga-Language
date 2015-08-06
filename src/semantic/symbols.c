/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * symbols.c
 */

#include <stdint.h>

#include "symbols.h"

#include "util/error.h"
#include "util/new.h"
#include "util/vector.h"

typedef struct {
    char* identifier;
    AstDeclaration* declaration;
} Symbol;

Vector* symbols = NULL;
Vector* blocks = NULL;

static void init();

void SymbolsAdd(char* identifier, AstDeclaration* declaration, int line)
{
    int current = 0;
    int last = 0;
    Symbol* new_symbol = NULL;

    if (!symbols)
        init();

    /* Verifies if this declaration shadows a symbol in the current block */
    if (!VectorEmpty(blocks))
        last = (int)(intptr_t)VectorPeek(blocks);
    for (current = VectorSize(symbols) - 1; current >= last; current--) {
        Symbol* symbol = (Symbol*)VectorGet(symbols, current);
        if (symbol->identifier == identifier)
            ErrorL(line, "symbol '%s' is already declared", identifier);
    }

    /* Inserts the the symbol */
    new_symbol = NEW(Symbol);
    new_symbol->identifier = identifier;
    new_symbol->declaration = declaration;
    VectorPush(symbols, new_symbol);
}

AstDeclaration* SymbolsFind(char* identifier, int line)
{
    int current = VectorSize(symbols) - 1;
    for (; current >= 0; current--) {
        Symbol* symbol = (Symbol*)VectorGet(symbols, current);
        if (symbol->identifier == identifier)
            return symbol->declaration;
    }
    ErrorL(line, "symbol '%s' is not declared", identifier);
    return NULL;
}

void SymbolsOpenBlock()
{
    int next_block = VectorSize(symbols);
    VectorPush(blocks, (void*)(intptr_t)next_block);
}

void SymbolsCloseBlock()
{
    int current = 0;
    int last = (int)(intptr_t)VectorPop(blocks);
    for (current = VectorSize(symbols) - 1; current >= last; current--)
        VectorPop(symbols);
}

static void init()
{
    symbols = VectorCreate();
    blocks = VectorCreate();
}

