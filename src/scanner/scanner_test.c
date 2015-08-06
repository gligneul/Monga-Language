/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * scanner_test.c
 */

#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"

#include "parser/parser.h"

YYSTYPE yylval;

static const char* tokenToString(int token);

int main()
{
    int token;
    for (;;) {
        token = yylex();
        if (!token)
            break;

        if (token > 256)
            printf("%s", tokenToString(token));
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

static const char* tokenToString(int token)
{
    switch (token) {
    case TK_VOID:           return "TK_VOID";
    case TK_BOOL:           return "TK_BOOL";
    case TK_CHAR:           return "TK_CHAR";
    case TK_INT:            return "TK_INT";
    case TK_FLOAT:          return "TK_FLOAT";
    case TK_IF:             return "TK_IF";
    case TK_ELSE:           return "TK_ELSE";
    case TK_WHILE:          return "TK_WHILE";
    case TK_RETURN:         return "TK_RETURN";
    case TK_NEW:            return "TK_NEW";
    case TK_DELETE:         return "TK_DELETE";
    case TK_PRINT:          return "TK_PRINT";
    case TK_NULL:           return "TK_NULL";
    case TK_TRUE:           return "TK_TRUE";
    case TK_FALSE:          return "TK_FALSE";
    case TK_EQUALS:         return "TK_EQUALS";
    case TK_NOT_EQUALS:     return "TK_NOT_EQUALS";
    case TK_LESS_EQUALS:    return "TK_LESS_EQUALS";
    case TK_GREATER_EQUALS: return "TK_GREATER_EQUALS";
    case TK_AND:            return "TK_AND";
    case TK_OR:             return "TK_OR";
    case TK_KINT:           return "TK_KINT";
    case TK_KFLOAT:         return "TK_KFLOAT";
    case TK_STRING:         return "TK_STRING";
    case TK_ID:             return "TK_ID";
    default:                return "UNDEFINED";
    }
}

