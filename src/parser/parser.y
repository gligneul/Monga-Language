/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * parser.y
 */

%{
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "scanner/scanner.h"
#include "util/error.h"

void yyerror(char* message);

ast_decl_t* parser_ast = NULL;
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
    type_t type_t_;
    ast_decl_t* ast_decl_t_;
    ast_cmd_t* ast_cmd_t_;
    ast_exp_t* ast_exp_t_;
    ast_var_t* ast_var_t_;
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
%type <type_t_> base_type type
%type <ast_decl_t_> declarations variable_declaration identifier_list function_declaration
                    prototype_declaration parameters parameters_list variables_block
%type <ast_cmd_t_> block commands_block command
%type <ast_exp_t_> call expression expression_list
%type <ast_var_t_> variable

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
                    | declarations prototype_declaration
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
                            ast_decl_t* node = $2;
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
                            $$ = type_create(TYPE_BOOL, 0);
                        }
                    | TK_CHAR
                        {
                            $$ = type_create(TYPE_CHAR, 0);
                        }
                    | TK_INT
                        {
                            $$ = type_create(TYPE_INT, 0);
                        }
                    | TK_FLOAT
                        {
                            $$ = type_create(TYPE_FLOAT, 0);
                        }
                    ;

identifier_list     : identifier_list ',' TK_ID
                        {
                            type_t type = type_create(TYPE_UNDEFINED, 0);
                            ast_decl_t* node = ast_decl_variable(
                                    type, $3.str, $3.line);
                            $$ = AST_CONCAT($1, node);
                        }
                    | TK_ID
                        {
                            type_t type = type_create(TYPE_UNDEFINED, 0);
                            $$ = ast_decl_variable(type, $1.str, $1.line);
                        }
                    ;

function_declaration: type TK_ID '(' parameters ')' block
                        {
                            $$ = ast_decl_function($1, $2.str, $2.line, $4, $6);
                        }
                    | TK_VOID TK_ID '(' parameters ')' block
                        {
                            type_t type = type_create(TYPE_VOID, 0);
                            $$ = ast_decl_function(type, $2.str, $2.line, $4, $6);
                        }
                    ;

prototype_declaration: type TK_ID '(' parameters ')' ';'
                        {
                            $$ = ast_decl_prototype($1, $2.str, $2.line, $4);
                        }
                    | TK_VOID TK_ID '(' parameters ')' ';'
                        {
                            type_t type = type_create(TYPE_VOID, 0);
                            $$ = ast_decl_prototype(type, $2.str, $2.line, $4);
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
                            ast_decl_t* node = ast_decl_variable($3, $4.str, $4.line);
                            $$ = AST_CONCAT($1, node);
                        }
                    | type TK_ID
                        {
                            $$ = ast_decl_variable($1, $2.str, $2.line);
                        }
                    ;

block               : '{' variables_block commands_block '}'
                        {
                            $$ = ast_cmd_block($2, $3, $1);
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
                            $$ = ast_cmd_if($3, $5, NULL, $1);
                        }
                    | TK_IF '(' expression ')' command TK_ELSE command
                        {
                            $$ = ast_cmd_if($3, $5, $7, $1);
                        }
                    | TK_WHILE '(' expression ')' command
                        {
                            $$ = ast_cmd_while($3, $5, $1);
                        }
                    | variable '=' expression ';'
                        {
                            $$ = ast_cmd_assign($1, $3, $2);
                        }
                    | TK_DELETE expression ';'
                        {
                            $$ = ast_cmd_delete($2, $1);
                        }
                    | TK_RETURN ';'
                        {
                            $$ = ast_cmd_return(NULL, $1);
                        }
                    | TK_RETURN expression ';'
                        {
                            $$ = ast_cmd_return($2, $1);
                        }
                    | call ';'
                        {
                            $$ = ast_cmd_call($1, $2);
                        }
                    | block
                        {
                            $$ = $1;
                        }
                    ;

variable            : TK_ID
                        {
                            $$ = ast_var_identifier($1.str, $1.line);
                        }
                    | expression '[' expression ']'
                        {
                            $$ = ast_var_array($1, $3, $2);
                        }
                    ;

expression          : TK_KINT
                        {
                            $$ = ast_exp_kint($1);
                        }
                    | TK_KFLOAT
                        {
                            $$ = ast_exp_kfloat($1);
                        }
                    | TK_STRING
                        {
                            $$ = ast_exp_string($1);
                        }
                    | TK_NULL
                        {
                            $$ = ast_exp_null();
                        }
                    | TK_TRUE
                        {
                            $$ = ast_exp_bool(1);
                        }
                    | TK_FALSE
                        {
                            $$ = ast_exp_bool(0);
                        }
                    | variable
                        {
                            $$ = ast_exp_variable($1, $1->line);
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
                            $$ = ast_exp_new($2, $4, $3);
                        }
                    | '-' expression %prec TKX_UNARY
                        {
                            $$ = ast_exp_unary(OP_NEGATE, $2, $1);
                        }
                    | expression '+' expression
                        {
                            $$ = ast_exp_binary(OP_ADD, $1, $3, $2);
                        }
                    | expression '-' expression
                        {
                            $$ = ast_exp_binary(OP_SUB, $1, $3, $2);
                        }
                    | expression '*' expression
                        {
                            $$ = ast_exp_binary(OP_MUL, $1, $3, $2);
                        }
                    | expression '/' expression
                        {
                            $$ = ast_exp_binary(OP_DIV, $1, $3, $2);
                        }
                    | expression TK_EQUALS expression
                        {
                            $$ = ast_exp_binary(OP_EQUALS, $1, $3, $2);
                        }
                    | expression TK_NOT_EQUALS expression
                        {
                            $$ = ast_exp_binary(OP_NOT_EQUALS, $1, $3, $2);
                        }
                    | expression '<' expression
                        {
                            $$ = ast_exp_binary(OP_LESS, $1, $3, $2);
                        }
                    | expression TK_LESS_EQUALS expression
                        {
                            $$ = ast_exp_binary(OP_LESS_EQUALS, $1, $3, $2);
                        }
                    | expression '>' expression
                        {
                            $$ = ast_exp_binary(OP_GREATER, $1, $3, $2);
                        }
                    | expression TK_GREATER_EQUALS expression
                        {
                            $$ = ast_exp_binary(OP_GREATER_EQUALS, $1, $3, $2);
                        }
                    | '!' expression %prec TKX_UNARY
                        {
                            $$ = ast_exp_unary(OP_NOT, $2, $1);
                        }
                    | expression TK_AND expression
                        {
                            $$ = ast_exp_binary(OP_AND, $1, $3, $2);
                        }
                    | expression TK_OR expression
                        {
                            $$ = ast_exp_binary(OP_OR, $1, $3, $2);
                        }
                    ;

call                : TK_ID '(' ')'
                        {
                            $$ = ast_exp_call($1.str, NULL, $1.line);
                        }
                    | TK_ID '(' expression_list ')'
                        {
                            $$ = ast_exp_call($1.str, $3, $1.line);
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

void yyerror(char* s)
{
    char* token = scanner_token();
    int line = scanner_line();
    if (*token == '\0')
        errorl(line, "%s, unexpected end of file", s);
    else
        errorl(line, "%s, unexpected token '%s'", s, token);
}

