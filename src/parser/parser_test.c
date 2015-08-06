/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * parser_test.c
 */

#include <stdio.h>

#include "parser.h"

int main()
{
    if (!yyparse())
        printf("monga: parse succeeded\n");
    return 0;
}

