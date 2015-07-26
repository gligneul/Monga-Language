/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * scanner.h
 * Reads the stdin and converts it to tokens.
 */

#ifndef SCANNER_H
#define SCANNER_H

#include "util/vector.h"

/* Obtains the current line in scanner input */
int scanner_line();

/* Obtains the last token read */
char* scanner_token();

/* Obtains the literals strings */
vector_t* scanner_get_strings();

/* Lex scanner prototype
 * Returns the token read */
int yylex();

#endif

