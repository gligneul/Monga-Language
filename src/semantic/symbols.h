/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * symbols.h
 * Data structure that represents the symbol's table.
 * It is implemented with two stacks, one for the symbols and another for the
 * blocks.
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "ast/ast.h"

/* Adds a symbol to the table */
void symbols_add(char* identifier, ast_decl_t* declaration, int line);

/* Retrieves a symbol from the table */
ast_decl_t* symbols_find(char* indentifer, int line);

/* Opens a block */
void symbols_open_block();

/* Closes a block */
void symbols_close_block();

#endif

