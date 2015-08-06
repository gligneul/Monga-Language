/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * semantic_test.c
 */

#include "ast/ast_print.h"
#include "parser/parser.h"
#include "semantic/semantic.h"

int main()
{
    if (!yyparse()) {
        SemanticAnalyseTree(parser_ast);
        AstPrintTree(parser_ast);
    }
    return 0;
}


