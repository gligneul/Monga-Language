/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * parser_test.c
 */

#include <stdio.h>

#include "parser.h"

int main()
{
    if (!yyparse())
        printf("mc: parse succeeded\n");
    return 0;
}

