/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
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
void SymbolsAdd(char* identifier, AstDeclaration* declaration, int line);

/* Retrieves a symbol from the table */
AstDeclaration* SymbolsFind(char* indentifer, int line);

/* Opens a block */
void SymbolsOpenBlock();

/* Closes a block */
void SymbolsCloseBlock();

#endif

