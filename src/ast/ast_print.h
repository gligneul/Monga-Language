/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * ast_print.h
 * Responsable for printing the ast.
 */

#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast.h"

/* Prints the ast tree in stdout */
void ast_print(ast_decl_t* tree);

/* Gets the string representation of an unary operator */
const char* ast_print_unop(ast_unop operator);

/* Gets the string representation of a binary operator */
const char* ast_print_binop(ast_binop operator);

#endif
