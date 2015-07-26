/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * locations.h
 */

#ifndef LOCATIONS_H
#define LOCATIONS_H

#include "ast/ast.h"

/* Annotates the variables locations */
void locations_compute(ast_decl_t* tree);

#endif

