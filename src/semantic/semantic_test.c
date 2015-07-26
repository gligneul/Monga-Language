/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * semantic_test.c
 */

#include "ast/ast_print.h"
#include "parser/parser.h"
#include "semantic/semantic.h"

int main()
{
    if (!yyparse()) {
        semantic_analysis(parser_ast);
        ast_print(parser_ast);
    }
    return 0;
}


