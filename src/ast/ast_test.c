/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * ast_test.c
 */

#include "ast_print.h"

#include "parser/parser.h"

int main()
{
    if (!yyparse())
        AstPrintTree(parser_ast);
    return 0;
}

