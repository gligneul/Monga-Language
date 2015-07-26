/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * scanner_test.c
 */

#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"

#include "parser/parser.h"

YYSTYPE yylval;

int main()
{
    char* token_names[] = {
            "TK_VOID",
            "TK_BOOL",
            "TK_CHAR",
            "TK_INT",
            "TK_FLOAT",
            "TK_IF",
            "TK_ELSE",
            "TK_WHILE",
            "TK_RETURN",
            "TK_NEW",
            "TK_DELETE",
            "TK_NULL",
            "TK_TRUE",
            "TK_FALSE",
            "TK_EQUALS",
            "TK_NOT_EQUALS",
            "TK_LESS_EQUALS",
            "TK_GREATER_EQUALS",
            "TK_AND",
            "TK_OR",
            "TK_KINT",
            "TK_KFLOAT",
            "TK_STRING",
            "TK_ID"
    };
    int token;
    for (;;) {
        token = yylex();
        if (!token)
            break;

        if (token > 256)
            printf("%s", token_names[token - 257]);
        else
            printf("%c", (char)token);

        switch (token) {
        case TK_KINT:
            printf(" (%d)", yylval.int_);
            break;
        case TK_KFLOAT:
            printf(" (%f)", yylval.float_);
            break;
        case TK_STRING:
            printf(" (%s)", yylval.string_);
            break;
        case TK_ID:
            printf(" (%s)", yylval.identifier_.str);
            break;
        }
        printf("\n");
    }
    return 0;
}

