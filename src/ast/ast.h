/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * ast.h
 * Data type responsable for the AST representation
 */

#ifndef AST_H
#define AST_H

#include "type.h"

/* Concatenates two list
 * Returns the first element of the concatenated list
 * This elements will point to the last element */
#define AST_CONCAT(left, right) \
    (!left ? right : \
     (left->last->next = right, \
      left->last = right->last, \
      left))

/* Iterates over a list */
#define AST_FOREACH(type, iterator, init) \
    for (type* iterator = init; iterator != NULL; iterator = iterator->next)

/* Data type definitions */
typedef struct AstDeclaration AstDeclaration;
typedef struct AstStatement AstStatement;
typedef struct AstExpression AstExpression;
typedef struct AstVariable AstVariable;

/* AstDeclaration */
struct AstDeclaration {

    /* Types of declarations (tags) */
    enum {
        AST_DECLARATION_VARIABLE,
        AST_DECLARATION_FUNCTION,
    } tag;

    /* Type of this declaration */
    Type type;

    /* Identifier of variable or function */
    char* identifier;

    /* Line in source file */
    int line;

    /* List representation */
    AstDeclaration* next;
    AstDeclaration* last;

    /* Specific fields for each tag */
    union {
        /* AST_DECLARATION_VARIABLE */
        struct {
            bool global;
            int offset;
        } variable_;

        /* AST_DECLARATION_FUNCTION */
        struct {
            AstDeclaration* parameters;
            int n_parameters;
            AstStatement* block;
            int space;
        } function_;
    } u;
};

/* AstStatement */
struct AstStatement {

    /* Types of statements (tags) */
    enum {
        AST_STATEMENT_BLOCK,
        AST_STATEMENT_IF,
        AST_STATEMENT_WHILE,
        AST_STATEMENT_ASSIGN,
        AST_STATEMENT_DELETE,
        AST_STATEMENT_PRINT,
        AST_STATEMENT_RETURN,
        AST_STATEMENT_CALL
    } tag;

    /* Line in source file */
    int line;

    /* True if this statement or it's substatements have returned */
    bool returned;

    /* List representation */
    AstStatement* next;
    AstStatement* last;

    /* Specific fields for each tag */
    union {
        /* AST_STATEMENT_BLOCK */
        struct {
            AstDeclaration* variables;
            AstStatement* statements;
        } block_;

        /* AST_STATEMENT_IF */
        struct {
            AstExpression* expression;
            AstStatement* then_statement;
            AstStatement* else_statement;
        } if_;

        /* AST_STATEMENT_WHILE */
        struct {
            AstExpression* expression;
            AstStatement* statement;
        } while_;

        /* AST_STATEMENT_ASSIGN */
        struct {
            AstVariable* variable;
            AstExpression* expression;
        } assign_;

        /* AST_STATEMENT_DELETE */
        struct {
            AstExpression* expression;
        } delete_;

        /* AST_STATEMENT_PRINT */
        struct {
            AstExpression* expressions;
        } print_;

        /* AST_STATEMENT_RETURN */
        struct {
            AstExpression* expression;
        } return_;

        /* AST_STATEMENT_CALL */
        AstExpression* call_;
    } u;
};

/* Unary expression operators */
typedef enum {
    AST_OPERATOR_NEGATE,
    AST_OPERATOR_NOT
} AstUnaryOperator;

/* Binary expression operators */
typedef enum {
    AST_OPERATOR_ADD,
    AST_OPERATOR_SUB,
    AST_OPERATOR_MUL,
    AST_OPERATOR_DIV,
    AST_OPERATOR_EQUALS,
    AST_OPERATOR_NOT_EQUALS,
    AST_OPERATOR_LESS,
    AST_OPERATOR_LESS_EQUALS,
    AST_OPERATOR_GREATER,
    AST_OPERATOR_GREATER_EQUALS,
    AST_OPERATOR_AND,
    AST_OPERATOR_OR
} AstBinaryOperator;

/* Cast tags */
typedef enum {
    AST_CAST_INT_TO_FLOAT,
    AST_CAST_FLOAT_TO_INT
} AstCastTag;

/* AstExpression */
struct AstExpression {

    /* Types of expressions (tags) */
    enum {
        AST_EXPRESSION_KBOOL,
        AST_EXPRESSION_KINT,
        AST_EXPRESSION_KFLOAT,
        AST_EXPRESSION_STRING,
        AST_EXPRESSION_NULL,
        AST_EXPRESSION_CALL,
        AST_EXPRESSION_VARIABLE,
        AST_EXPRESSION_NEW,
        AST_EXPRESSION_UNARY,
        AST_EXPRESSION_BINARY,
        AST_EXPRESSION_CAST
    } tag;

    /* Type of this expression */
    Type type;

    /* Line in source file */
    int line;

    /* List representation */
    AstExpression* next;
    AstExpression* last;

    /* Specific fields for each tag */
    union {
        /* AST_EXPRESSION_KBOOL */
        bool kbool_;

        /* AST_EXPRESSION_KINT */
        int kint_;

        /* AST_EXPRESSION_KFLOAT */
        float kfloat_;

        /* AST_EXPRESSION_STRING */
        char* string_;

        /* AST_EXPRESSION_CALL */
        struct {
            bool is_declaration;
            union {
                char* identifier_;
                AstDeclaration* declaration_;
            } u;
            AstExpression* expressions;
        } call_;

        /* AST_EXPRESSION_VARIABLE */
        AstVariable* variable_;

        /* AST_EXPRESSION_NEW */
        struct {
            Type type;
            AstExpression* expression;
        } new_;

        /* AST_EXPRESSION_UNARY */
        struct {
            AstUnaryOperator operator;
            AstExpression* expression;
        } unary_;

        /* AST_EXPRESSION_BINARY */
        struct {
            AstBinaryOperator operator;
            AstExpression* expression_left;
            AstExpression* expression_right;
        } binary_;

        /* AST_EXPRESSION_CAST */
        struct {
            AstCastTag tag;
            AstExpression* expression;
        } cast_;
    } u;
};

/* AstVariable */
struct AstVariable {

    /* Types of variables (tags) */
    enum {
        AST_VARIABLE_REFERENCE,
        AST_VARIABLE_ARRAY
    } tag;

    /* Type of this declaration */
    Type type;

    /* Line in source file */
    int line;

    /* Specific fields for each tag */
    union {
        /* AST_VARIABLE_REFERENCE */
        struct {
            bool is_declaration;
            union {
                char* identifier_;
                AstDeclaration* declaration_;
            } u;
        } reference_;

        /* AST_VARIABLE_ARRAY */
        struct {
            AstExpression* location;
            AstExpression* offset;
        } array_;
    } u;
};

/* Functions for creating the nodes */
AstDeclaration* AstDeclarationVariable(Type type, char* identifier, int line);
AstDeclaration* AstDeclarationFunction(Type type, char* identifier, int line,
        AstDeclaration* parameters, AstStatement* block);

AstStatement* AstStatementBlock(AstDeclaration* variables,
        AstStatement* statements, int line);
AstStatement* AstStatementIf(AstExpression* expression,
        AstStatement* then_statement, AstStatement* else_statement, int line);
AstStatement* AstStatementWhile(AstExpression* expression,
        AstStatement* statement, int line);
AstStatement* AstStatementAssign(AstVariable* variable,
        AstExpression* expression, int line);
AstStatement* AstStatementDelete(AstExpression* expression, int line);
AstStatement* AstStatementPrint(AstExpression* expressions, int line);
AstStatement* AstStatementReturn(AstExpression* expression, int line);
AstStatement* AstStatementCall(AstExpression* call, int line);

AstExpression* AstExpressionKBool(bool value);
AstExpression* AstExpressionKInt(int value);
AstExpression* AstExpressionKFloat(float value);
AstExpression* AstExpressionString(char* string);
AstExpression* AstExpressionNull();
AstExpression* AstExpressionCall(char* identifier, AstExpression* expressions,
        int line);
AstExpression* AstExpressionVariable(AstVariable* variable, int line);
AstExpression* AstExpressionNew(Type type, AstExpression* expression, int line);
AstExpression* AstExpressionUnary(AstUnaryOperator operator, 
        AstExpression* expression, int line);
AstExpression* AstExpressionBinary(AstBinaryOperator operator, 
        AstExpression* expression_left, AstExpression* expression_right,
        int line);
void AstExpressionCast(AstExpression* expression, Type goal_type,
        AstCastTag cast_tag);

AstVariable* AstVariableReference(char* identifier, int line);
AstVariable* AstVariableArray(AstExpression* location, AstExpression* offset,
        int line);

#endif

