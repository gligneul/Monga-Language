/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * scanner.l
 */

%option nounput
%option noinput
%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser/parser.h"
#include "util/error.h"
#include "util/new.h"
#include "util/table.h"
#include "util/vector.h"

static int current_line = 1;
static TableRef symbols = NULL;
static Vector* strings = NULL;

/* Increments the current line based on yytext */
static void incrementCurrentLine();

/* Converts escape to an character */
static char convertEscape(char escape);

/* Removes the escapes from a literal string. */
static char* removeEscapes(char* in);

/* Adds a symbol to the symbol's pool. */
static char* insertSymbol(char* symbol);
%}

space       [ \n\t]+
comment     "/*"([^*]|"*"+[^*\/])*\*+"/"

integer     [0-9]+
hexa        0[xX][0-9A-Fa-f]+
literal     '(\\.|.)'

exp         [Ee][+-]?[0-9]+
float1      [0-9]+{exp}
float2      [0-9]+\.[0-9]*{exp}?
float3      \.[0-9]+{exp}?
float       {float1}|{float2}|{float3}

string      \"(\\.|[^\n\\"])*\"

id          [a-zA-Z_][a-zA-Z0-9_]*
any	        . 

%%

{space}     {
                incrementCurrentLine();
            }

{comment}   {
                incrementCurrentLine();
            }

void        {
                yylval.int_ = current_line;
                return TK_VOID; 
            }

bool        {
                yylval.int_ = current_line;
                return TK_BOOL;
            }
            
char        {
                yylval.int_ = current_line;
                return TK_CHAR;
            }

int         {
                yylval.int_ = current_line;
                return TK_INT;
            }

float       {
                yylval.int_ = current_line;
                return TK_FLOAT;
            }

if          {
                yylval.int_ = current_line;
                return TK_IF;
            }

else        {
                yylval.int_ = current_line;
                return TK_ELSE;
            }

while       {
                yylval.int_ = current_line;
                return TK_WHILE;
            }

return      {
                yylval.int_ = current_line;
                return TK_RETURN;
            }

new         {
                yylval.int_ = current_line;
                return TK_NEW;
            }

delete      {
                yylval.int_ = current_line;
                return TK_DELETE;
            }

print       {
                yylval.int_ = current_line;
                return TK_PRINT;
            }

null        {
                yylval.int_ = current_line;
                return TK_NULL;
            }

true        {
                yylval.int_ = current_line;
                return TK_TRUE;
            }

false       {
                yylval.int_ = current_line;
                return TK_FALSE;
            }

"=="        {
                yylval.int_ = current_line;
                return TK_EQUALS;
            }

"!="        {
                yylval.int_ = current_line;
                return TK_NOT_EQUALS;
            }

"<="        {
                yylval.int_ = current_line;
                return TK_LESS_EQUALS;
            }

">="        {
                yylval.int_ = current_line;
                return TK_GREATER_EQUALS;
            }

"&&"        {
                yylval.int_ = current_line;
                return TK_AND;
            }

"||"        {
                yylval.int_ = current_line;
                return TK_OR;
            }

{integer}   {
                yylval.int_ = (int)strtol(yytext, 0, 10);
                return TK_KINT;
            }

{hexa}      {
                yylval.int_ = (int)strtol(yytext, 0, 16);
                return TK_KINT;
            }

{literal}   {
                yylval.int_ = yyleng == 4 ? convertEscape(yytext[2]) : yytext[1];
                return TK_KINT;
            }

{float}     {
                yylval.float_ = (float)strtod(yytext, NULL);
                return TK_KFLOAT;
            }

{string}    {
                size_t i = 0;
                char* literal = removeEscapes(yytext);
                yylval.string_ = insertSymbol(literal); 
                free(literal);

                if (strings == NULL)
                    strings = VectorCreate();

                for (i = 0; i < VectorSize(strings); ++i) {
                    char* curr = (char*)VectorGet(strings, i);
                    if (curr == yylval.string_)
                        return TK_STRING;
                }
                VectorPush(strings, yylval.string_);
                return TK_STRING;
            }

{id}        {
                yylval.identifier_.str = insertSymbol(yytext);
                yylval.identifier_.line = current_line;
                return TK_ID;
            }

{any}       {
                yylval.int_ = current_line;
                return yytext[0];
            }

%%

int ScannerGetCurrentLine()
{
    return current_line;
}

char* ScannerGetCurrentToken()
{
    return &(yytext[0]);
}

Vector* ScannerGetStrings()
{
    return strings;
}

static void incrementCurrentLine()
{
    char* str = yytext;
    for (size_t i = 0; str[i] != '\0'; ++i)
        if (str[i] == '\n')
            current_line++;
}

static char convertEscape(char escape)
{
    switch (escape) {
    case '\\':
    case '"':
        return escape;
    case 'n':
        return '\n';
    case 't':
        return '\t';
    default:
        ErrorL(current_line, "unexpected escape '\\%c' (%d)", escape, escape);
    }
    return 0;
}

static char* removeEscapes(char* in)
{
    size_t len = strlen(in) - 2;
    char* out = (char*)malloc(len + 1);
    int i = 0;
    in++;
    while (*in != '"') {
        char c = *in++;
        if (c == '\\')
            c = convertEscape(*in++);
        out[i++] = c;
    }
    out[i] = '\0';
    return out;
}

static char* insertSymbol(char* symbol)
{
    if (!symbols) {
        symbols = TableCreate(free, free, TableStrCopy, TableDummyCopy,
                TableStrLess);
    }
    return TableInsert(symbols, symbol, NULL).key;
}

