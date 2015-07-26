/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * ast_test.c
 */

#include "ast_print.h"

#include "parser/parser.h"

int main()
{
    if (!yyparse())
        ast_print(parser_ast);
    return 0;
}

