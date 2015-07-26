/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * ast.h
 * Data type responsable for the AST representation.
 */

#ifndef AST_H
#define AST_H

#include "type.h"

/* Concatenates two nodes
 * Both nodes must be the first element of the lists and must point to the last
 * element of the lists
 * Returns the first element of the concatenated list
 * This elements points to the last elemtement */
#define AST_CONCAT(left, right) \
    (!left ? right : \
     (left->last->next = right, \
      left->last = right->last, \
      left))

/* Data type definitions */
typedef struct ast_decl_t ast_decl_t;
typedef struct ast_cmd_t ast_cmd_t;
typedef struct ast_exp_t ast_exp_t;
typedef struct ast_var_t ast_var_t;

/* Declaration tags */
typedef enum {
    DECL_VARIABLE,
    DECL_FUNCTION,
    DECL_PROTOTYPE
} ast_decl_tag;

/* Declaration data type */
struct ast_decl_t {
    ast_decl_tag tag;
    type_t type;
    char* identifier;
    int line;
    ast_decl_t* next;
    ast_decl_t* last;
    union {
        /* DECL_VARIABLE */
        struct {
            bool global;
            int offset;
        } variable_;

        /* DECL_FUNCTION */
        struct {
            ast_decl_t* parameters;
            ast_cmd_t* block;
            int space;
        } function_;

        /* DECL_PROTOTYPE */
        struct {
            ast_decl_t* parameters;
        } prototype_;
    } u;
};

/* Command tags */
typedef enum {
    CMD_BLOCK,
    CMD_IF,
    CMD_WHILE,
    CMD_ASSIGN,
    CMD_DELETE,
    CMD_RETURN,
    CMD_CALL
} ast_cmd_tag;

/* Command data type */
struct ast_cmd_t {
    ast_cmd_tag tag;
    int line;
    ast_cmd_t* next;
    ast_cmd_t* last;
    union {
        /* CMD_BLOCK */
        struct {
            ast_decl_t* variables;
            ast_cmd_t* commands;
        } block_;

        /* CMD_IF */
        struct {
            ast_exp_t* expression;
            ast_cmd_t* command_if;
            ast_cmd_t* command_else;
        } if_;

        /* CMD_WHILE */
        struct {
            ast_exp_t* expression;
            ast_cmd_t* command;
        } while_;

        /* CMD_ASSIGN */
        struct {
            ast_var_t* variable;
            ast_exp_t* expression;
        } assign_;

        /* CMD_DELETE */
        struct {
            ast_exp_t* expression;
        } delete_;

        /* CMD_RETURN */
        struct {
            ast_exp_t* expression;
        } return_;

        /* CMD_CALL */
        ast_exp_t* call_;
    } u;
};

/* Expression tags */
typedef enum {
    EXP_KINT,
    EXP_KFLOAT,
    EXP_STRING,
    EXP_NULL,
    EXP_BOOL,
    EXP_CALL_ID,
    EXP_CALL_DECL,
    EXP_VARIABLE,
    EXP_NEW,
    EXP_UNARY,
    EXP_BINARY,
    EXP_CAST
} ast_exp_tag;

/* Unary expression operators */
typedef enum {
    OP_NEGATE,
    OP_NOT
} ast_unop;

/* Binary expression operators */
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQUALS,
    OP_NOT_EQUALS,
    OP_LESS,
    OP_LESS_EQUALS,
    OP_GREATER,
    OP_GREATER_EQUALS,
    OP_AND,
    OP_OR
} ast_binop;

/* Cast tags */
typedef enum {
    CAST_INT_TO_FLOAT,
    CAST_FLOAT_TO_INT,
} ast_cast_tag;

/* Expression data type */
struct ast_exp_t {
    ast_exp_tag tag;
    int line;
    type_t type;
    ast_exp_t* next;
    ast_exp_t* last;
    union {
        /* EXP_KINT */
        int kint_;

        /* EXP_KFLOAT */
        float kfloat_;

        /* EXP_STRING */
        char* string_;

        /* EXP_BOOL */
        int bool_;

        /* EXP_CALL_ID */
        struct {
            char* identifier;
            ast_exp_t* expressions;
        } call_id_;

        /* EXP_CALL_DECL */
        struct {
            ast_decl_t* declaration;
            ast_exp_t* expressions;
        } call_decl_;

        /* EXP_VARIABLE */
        ast_var_t* variable_;

        /* EXP_NEW */
        struct {
            type_t type;
            ast_exp_t* expression;
        } new_;

        /* EXP_UNARY */
        struct {
            ast_unop operator;
            ast_exp_t* expression;
        } unary_;

        /* EXP_BINARY */
        struct {
            ast_binop operator;
            ast_exp_t* expression_left;
            ast_exp_t* expression_right;
        } binary_;

        /* EXP_CAST */
        struct {
            ast_cast_tag tag;
            ast_exp_t* expression;
        } cast_;
    } u;
};

/* Variable tags */
typedef enum {
    VAR_IDENTIFIER,
    VAR_DECLARATION,
    VAR_ARRAY
} ast_var_tag;

/* Variable data type */
struct ast_var_t {
    ast_var_tag tag;
    int line;
    type_t type;
    union {
        /* VAR_IDENTIFIER */
        char* identifier_;

        /* VAR_DECLARATION */
        ast_decl_t* declaration_;

        /* VAR_ARRAY */
        struct {
            ast_exp_t* location;
            ast_exp_t* offset;
        } array_;
    } u;
};

/* Functions for creating the nodes */
ast_decl_t* ast_decl_variable(type_t type, char* identifier, int line);

ast_decl_t* ast_decl_function(type_t type, char* identifier, int line,
        ast_decl_t* parameters, ast_cmd_t* block);

ast_decl_t* ast_decl_prototype(type_t type, char* identifier, int line,
        ast_decl_t* parameters);

ast_cmd_t* ast_cmd_block(ast_decl_t* variables, ast_cmd_t* commands,
        int line);

ast_cmd_t* ast_cmd_if(ast_exp_t* expression, ast_cmd_t* command_if,
        ast_cmd_t* command_else, int line);

ast_cmd_t* ast_cmd_while(ast_exp_t* expression, ast_cmd_t* command,
        int line);

ast_cmd_t* ast_cmd_assign(ast_var_t* variable, ast_exp_t* expression,
        int line);

ast_cmd_t* ast_cmd_delete(ast_exp_t* expression, int line);

ast_cmd_t* ast_cmd_return(ast_exp_t* expression, int line);

ast_cmd_t* ast_cmd_call(ast_exp_t* call, int line);

ast_exp_t* ast_exp_kint(int value);

ast_exp_t* ast_exp_kfloat(float value);

ast_exp_t* ast_exp_string(char* string);

ast_exp_t* ast_exp_null();

ast_exp_t* ast_exp_bool(int value);

ast_exp_t* ast_exp_call(char* identifier, ast_exp_t* expressions,
        int line);

ast_exp_t* ast_exp_variable(ast_var_t* variable, int line);

ast_exp_t* ast_exp_new(type_t type, ast_exp_t* expression, int line);

ast_exp_t* ast_exp_unary(ast_unop operator, ast_exp_t* expression,
        int line);

ast_exp_t* ast_exp_binary(ast_binop operator, 
        ast_exp_t* expression_left, ast_exp_t* expression_right, int line);

void ast_exp_cast(ast_exp_t* expression, type_t goal_type,
        ast_cast_tag cast_tag);

ast_var_t* ast_var_identifier(char* identifier, int line);

ast_var_t* ast_var_array(ast_exp_t* location, ast_exp_t* offset, int line);

#endif

