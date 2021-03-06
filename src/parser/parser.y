/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * parser.y
 */

%{
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "scanner/scanner.h"
#include "util/error.h"

void yyerror(const char* message);

AstDeclaration* parser_ast = NULL;
%}

%token <int_> TK_VOID
%token <int_> TK_BOOL
%token <int_> TK_CHAR
%token <int_> TK_INT
%token <int_> TK_FLOAT
%token <int_> TK_IF
%token <int_> TK_ELSE
%token <int_> TK_WHILE
%token <int_> TK_RETURN
%token <int_> TK_NEW
%token <int_> TK_DELETE
%token <int_> TK_PRINT
%token <int_> TK_NULL
%token <int_> TK_TRUE
%token <int_> TK_FALSE

%token <int_> TK_EQUALS
%token <int_> TK_NOT_EQUALS
%token <int_> TK_LESS_EQUALS
%token <int_> TK_GREATER_EQUALS
%token <int_> TK_AND
%token <int_> TK_OR

%token <int_> TK_KINT
%token <float_> TK_KFLOAT
%token <string_> TK_STRING

%token <identifier_> TK_ID

%union {
    int int_;
    float float_;
    char* string_;
    struct {
        char* str;
        int line;
    } identifier_;
    Type Type_;
    AstDeclaration* AstDeclaration_;
    AstStatement* AstStatement_;
    AstExpression* AstExpression_;
    AstVariable* AstVariable_;
};

%nonassoc TKX_IF
%nonassoc TK_ELSE

%left TK_OR
%left TK_AND
%left TK_NOT_EQUALS TK_EQUALS
%left '<' TK_LESS_EQUALS '>' TK_GREATER_EQUALS
%left '+' '-'
%left '*' '/'
%nonassoc TKX_UNARY
%nonassoc '['

%type <int_> '<' '>' '+' '-' '*' '/' '{' '!' ';' '[' '='
%type <Type_> base_type type
%type <AstDeclaration_> declarations variable_declaration identifier_list function_declaration
                    parameters parameters_list variables_block
%type <AstStatement_> block commands_block command
%type <AstExpression_> call expression expression_list
%type <AstVariable_> variable

%%

program             : declarations
                        {
                            parser_ast = $1;
                        }
                    ;

declarations        : declarations variable_declaration
                        {
                            $$ = AST_CONCAT($1, $2);
                        }
                    | declarations function_declaration
                        {
                            $$ = AST_CONCAT($1, $2);
                        }
                    | /* empty */
                        {
                            $$ = NULL;
                        }
                    ;

variable_declaration: type identifier_list ';'
                        {
                            AstDeclaration* node = $2;
                            $$ = $2;
                            while (node) {
                                node->type = $1;
                                node = node->next;
                            }
                        }
                    ;

type                : type '[' ']'
                        {
                            $$ = $1;
                            $$.pointers += 1;
                        }
                    | base_type
                        {
                            $$ = $1;
                        }
                    ;

base_type           : TK_BOOL
                        {
                            $$ = TypeCreate(TYPE_BOOL, 0);
                        }
                    | TK_CHAR
                        {
                            $$ = TypeCreate(TYPE_CHAR, 0);
                        }
                    | TK_INT
                        {
                            $$ = TypeCreate(TYPE_INT, 0);
                        }
                    | TK_FLOAT
                        {
                            $$ = TypeCreate(TYPE_FLOAT, 0);
                        }
                    ;

identifier_list     : identifier_list ',' TK_ID
                        {
                            Type type = TypeCreate(TYPE_UNDEFINED, 0);
                            AstDeclaration* node = AstDeclarationVariable(
                                    type, $3.str, $3.line);
                            $$ = AST_CONCAT($1, node);
                        }
                    | TK_ID
                        {
                            Type type = TypeCreate(TYPE_UNDEFINED, 0);
                            $$ = AstDeclarationVariable(type, $1.str, $1.line);
                        }
                    ;

function_declaration: type TK_ID '(' parameters ')' block
                        {
                            $$ = AstDeclarationFunction($1, $2.str, $2.line, $4, $6);
                        }
                    | TK_VOID TK_ID '(' parameters ')' block
                        {
                            Type type = TypeCreate(TYPE_VOID, 0);
                            $$ = AstDeclarationFunction(type, $2.str, $2.line, $4, $6);
                        }
                    ;

parameters          : parameters_list
                        {
                            $$ = $1;
                        }
                    | /* empty */
                        {
                            $$ = NULL;
                        }
                    ;

parameters_list     : parameters_list ',' type TK_ID
                        {
                            AstDeclaration* node = AstDeclarationVariable($3, $4.str, $4.line);
                            $$ = AST_CONCAT($1, node);
                        }
                    | type TK_ID
                        {
                            $$ = AstDeclarationVariable($1, $2.str, $2.line);
                        }
                    ;

block               : '{' variables_block commands_block '}'
                        {
                            $$ = AstStatementBlock($2, $3, $1);
                        }
                    ;

variables_block     : variables_block variable_declaration
                        {
                            $$ = AST_CONCAT($1, $2);
                        }
                    | /* empty */
                        {
                            $$ = NULL;
                        }
                    ;

commands_block      : commands_block command
                        {
                            $$ = AST_CONCAT($1, $2);
                        }
                    | /* empty */
                        {
                            $$ = NULL;
                        }
                    ;

command             : TK_IF '(' expression ')' command %prec TKX_IF
                        {
                            $$ = AstStatementIf($3, $5, NULL, $1);
                        }
                    | TK_IF '(' expression ')' command TK_ELSE command
                        {
                            $$ = AstStatementIf($3, $5, $7, $1);
                        }
                    | TK_WHILE '(' expression ')' command
                        {
                            $$ = AstStatementWhile($3, $5, $1);
                        }
                    | variable '=' expression ';'
                        {
                            $$ = AstStatementAssign($1, $3, $2);
                        }
                    | TK_DELETE expression ';'
                        {
                            $$ = AstStatementDelete($2, $1);
                        }
                    | TK_PRINT expression_list ';'
                        {
                            $$ = AstStatementPrint($2, $1);
                        }
                    | TK_RETURN ';'
                        {
                            $$ = AstStatementReturn(NULL, $1);
                        }
                    | TK_RETURN expression ';'
                        {
                            $$ = AstStatementReturn($2, $1);
                        }
                    | call ';'
                        {
                            $$ = AstStatementCall($1, $2);
                        }
                    | block
                        {
                            $$ = $1;
                        }
                    ;

variable            : TK_ID
                        {
                            $$ = AstVariableReference($1.str, $1.line);
                        }
                    | expression '[' expression ']'
                        {
                            $$ = AstVariableArray($1, $3, $2);
                        }
                    ;

expression          : TK_KINT
                        {
                            $$ = AstExpressionKInt($1);
                        }
                    | TK_KFLOAT
                        {
                            $$ = AstExpressionKFloat($1);
                        }
                    | TK_STRING
                        {
                            $$ = AstExpressionString($1);
                        }
                    | TK_NULL
                        {
                            $$ = AstExpressionNull();
                        }
                    | TK_TRUE
                        {
                            $$ = AstExpressionKBool(1);
                        }
                    | TK_FALSE
                        {
                            $$ = AstExpressionKBool(0);
                        }
                    | variable
                        {
                            $$ = AstExpressionVariable($1, $1->line);
                        }
                    | '(' expression ')'
                        {
                            $$ = $2;
                        }
                    | call
                        {
                            $$ = $1;
                        }
                    | TK_NEW type '[' expression ']'
                        {
                            $$ = AstExpressionNew($2, $4, $3);
                        }
                    | '-' expression %prec TKX_UNARY
                        {
                            $$ = AstExpressionUnary(AST_OPERATOR_NEGATE, $2, $1);
                        }
                    | expression '+' expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_ADD, $1, $3, $2);
                        }
                    | expression '-' expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_SUB, $1, $3, $2);
                        }
                    | expression '*' expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_MUL, $1, $3, $2);
                        }
                    | expression '/' expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_DIV, $1, $3, $2);
                        }
                    | expression TK_EQUALS expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_EQUALS, $1, $3, $2);
                        }
                    | expression TK_NOT_EQUALS expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_NOT_EQUALS, $1, $3, $2);
                        }
                    | expression '<' expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_LESS, $1, $3, $2);
                        }
                    | expression TK_LESS_EQUALS expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_LESS_EQUALS, $1, $3, $2);
                        }
                    | expression '>' expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_GREATER, $1, $3, $2);
                        }
                    | expression TK_GREATER_EQUALS expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_GREATER_EQUALS, $1, $3, $2);
                        }
                    | '!' expression %prec TKX_UNARY
                        {
                            $$ = AstExpressionUnary(AST_OPERATOR_NOT, $2, $1);
                        }
                    | expression TK_AND expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_AND, $1, $3, $2);
                        }
                    | expression TK_OR expression
                        {
                            $$ = AstExpressionBinary(AST_OPERATOR_OR, $1, $3, $2);
                        }
                    ;

call                : TK_ID '(' ')'
                        {
                            $$ = AstExpressionCall($1.str, NULL, $1.line);
                        }
                    | TK_ID '(' expression_list ')'
                        {
                            $$ = AstExpressionCall($1.str, $3, $1.line);
                        }
                    ;

expression_list     : expression_list ',' expression
                        {
                            $$ = AST_CONCAT($1, $3);
                        }
                    | expression
                        {
                            $$ = $1;
                        }
                    ;

%%

void yyerror(const char* s)
{
    char* token = ScannerGetCurrentToken();
    int line = ScannerGetCurrentLine();
    if (*token == '\0')
        ErrorL(line, "%s, unexpected end of file", s);
    else
        ErrorL(line, "%s, unexpected token '%s'", s, token);
}

