/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * assembly.h
 */

#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "ast/ast.h"

/* Prints the IA-32 representation of the AST */
void assembly(ast_decl_t* tree);

#endif

