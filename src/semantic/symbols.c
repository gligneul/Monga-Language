/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
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
    ast_decl_t* declaration;
} symbol_t;

vector_t* symbols = NULL;
vector_t* blocks = NULL;

static void init();
static int same_prototype(ast_decl_t* a, ast_decl_t* b);

void symbols_add(char* identifier, ast_decl_t* declaration, int line)
{
    int current = 0;
    int last = 0;
    symbol_t* new_symbol = NULL;

    if (!symbols)
        init();

    /* Verifies if this declaration shadows a symbol in the current block */
    if (!vector_empty(blocks))
        last = (int)(intptr_t)vector_peek(blocks);
    for (current = vector_size(symbols) - 1; current >= last; current--) {
        symbol_t* symbol = (symbol_t*)vector_get(symbols, current);
        if (symbol->identifier == identifier) {
            if (same_prototype(declaration, symbol->declaration))
                return;
            errorl(line, "symbol '%s' is already declared", identifier);
        }
    }

    /* Inserts the the symbol */
    new_symbol = NEW(symbol_t);
    new_symbol->identifier = identifier;
    new_symbol->declaration = declaration;
    vector_push(symbols, new_symbol);
}

ast_decl_t* symbols_find(char* identifier, int line)
{
    int current = vector_size(symbols) - 1;
    for (; current >= 0; current--) {
        symbol_t* symbol = (symbol_t*)vector_get(symbols, current);
        if (symbol->identifier == identifier)
            return symbol->declaration;
    }
    errorl(line, "symbol '%s' is not declared", identifier);
    return NULL;
}

void symbols_open_block()
{
    int next_block = vector_size(symbols);
    vector_push(blocks, (void*)(intptr_t)next_block);
}

void symbols_close_block()
{
    int current = 0;
    int last = (int)(intptr_t)vector_pop(blocks);
    for (current = vector_size(symbols) - 1; current >= last; current--)
        vector_pop(symbols);
}

static void init()
{
    symbols = vector_create();
    blocks = vector_create();
}

static int same_prototype(ast_decl_t* a, ast_decl_t* b)
{
    ast_decl_t* a_args = NULL;
    ast_decl_t* b_args = NULL;

    if (!(a->tag == DECL_PROTOTYPE &&
          b->tag == DECL_PROTOTYPE &&
          type_cmp(a->type, b->type)))
        return 0;

    a_args = a->u.prototype_.parameters;
    b_args = b->u.prototype_.parameters;
    while (a_args != NULL && b_args != NULL)
        if (!type_cmp(a_args->type, b_args->type))
            return 0;

    return a_args == NULL && b_args == NULL;
}

