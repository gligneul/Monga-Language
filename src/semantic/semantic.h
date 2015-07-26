/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * semantic.h
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast/ast.h"

/* Makes the semantic analysis in the AST */
void semantic_analysis(ast_decl_t* ast);

#endif

