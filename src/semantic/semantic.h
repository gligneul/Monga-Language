/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * semantic.h
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast/ast.h"

/* Makes the semantic analysis in the AST */
AstDeclaration* SemanticAnalyseTree(AstDeclaration* ast);

#endif

