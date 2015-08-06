/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * scanner.h
 * Reads the stdin and converts it to tokens.
 */

#ifndef SCANNER_H
#define SCANNER_H

#include "util/vector.h"

/* Obtains the current line in scanner input */
int ScannerGetCurrentLine();

/* Obtains the last token read */
char* ScannerGetCurrentToken();

/* Obtains the literals strings */
Vector* ScannerGetStrings();

/* Lex scanner prototype, returns the token read */
int yylex();

#endif

