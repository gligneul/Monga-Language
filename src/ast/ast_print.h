/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * ast_print.h
 * Responsable for printing the ast.
 */

#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast.h"

/* Prints the ast tree in stdout */
void AstPrintTree(AstDeclaration* tree);

/* Gets the string representation of an unary operator */
const char* AstPrintUnaryOperator(AstUnaryOperator operator);

/* Gets the string representation of a binary operator */
const char* AstPrintBinaryOperator(AstBinaryOperator operator);

#endif
