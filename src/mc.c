/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * backend.c
 */

#include "backend/locations.h"
#include "backend/assembly.h"
#include "parser/parser.h"
#include "semantic/semantic.h"

int main()
{
    if (!yyparse()) {
        semantic_analysis(parser_ast);
        locations_compute(parser_ast);
        assembly(parser_ast);
    }
    return 0;
}

