/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * ast_print.c
 */

#include <stdio.h>

#include "ast.h"
#include "ast_print.h"

static void printIndentation(int spaces);
static void printDeclaration(int spaces, AstDeclaration* node);
static void printStatement(int spaces, AstStatement* node);
static void printExpression(int space, AstExpression* node);
static void printCast(AstCastTag tag);
static void printVariable(AstVariable* var);

void AstPrintTree(AstDeclaration* tree)
{
    printDeclaration(0, tree);
}

const char* AstPrintUnaryOperator(AstUnaryOperator operator)
{
    switch (operator) {
    case AST_OPERATOR_NEGATE: return "-";
    case AST_OPERATOR_NOT:    return "not";
    }
    return 0;
}

const char* AstPrintBinaryOperator(AstBinaryOperator operator)
{
    switch (operator) {
    case AST_OPERATOR_ADD:            return "+";
    case AST_OPERATOR_SUB:            return "-";
    case AST_OPERATOR_MUL:            return "*";
    case AST_OPERATOR_DIV:            return "/";
    case AST_OPERATOR_EQUALS:         return "==";
    case AST_OPERATOR_NOT_EQUALS:     return "!=";
    case AST_OPERATOR_LESS:           return "<";
    case AST_OPERATOR_LESS_EQUALS:    return "<=";
    case AST_OPERATOR_GREATER:        return ">";
    case AST_OPERATOR_GREATER_EQUALS: return ">=";
    case AST_OPERATOR_AND:            return "and";
    case AST_OPERATOR_OR:             return "or";
    }
    return 0;
}

static void printIndentation(int spaces)
{
    int i;
    printf("\n");
    for (i = 0; i < spaces; i++)
        printf(" ");
}

static void printDeclaration(int spaces, AstDeclaration* node)
{
    if (!node) return;

    printIndentation(spaces);
    printf("(");

    switch (node->tag) {
    case AST_DECLARATION_FUNCTION:
        printf("func");
        break;
    case AST_DECLARATION_VARIABLE:
        printf("var");
        break;
    }

    printf(" ");
    TypePrint(node->type);
    printf(" %s<%d>", node->identifier, node->line);

    switch (node->tag) {
    case AST_DECLARATION_FUNCTION:
        printDeclaration(spaces + 2, node->u.function_.parameters);
        printStatement(spaces + 2, node->u.function_.block);
        break;
    case AST_DECLARATION_VARIABLE:
        break;
    }

    printf(")");
    if (!spaces) printf("\n");
    printDeclaration(spaces, node->next);
}

static void printStatement(int spaces, AstStatement* node)
{
    if (!node) return;

    printIndentation(spaces);
    if (node->tag != AST_STATEMENT_CALL)
        printf("(");

    switch (node->tag) {
    case AST_STATEMENT_BLOCK:
        printf("block");
        printDeclaration(spaces + 2, node->u.block_.variables);
        printStatement(spaces + 2, node->u.block_.statements);
        break;
    case AST_STATEMENT_IF:
        printf("if");
        printExpression(1, node->u.if_.expression);
        printStatement(spaces + 4, node->u.if_.then_statement);
        printStatement(spaces + 4, node->u.if_.else_statement);
        break;
    case AST_STATEMENT_WHILE:
        printf("while");
        printExpression(1, node->u.while_.expression);
        printStatement(spaces + 2, node->u.while_.statement);
        break;
    case AST_STATEMENT_ASSIGN:
        printf("assign ");
        printVariable(node->u.assign_.variable);
        printExpression(1, node->u.assign_.expression);
        break;
    case AST_STATEMENT_DELETE:
        printf("delete");
        printExpression(1, node->u.delete_.expression);
        break;
    case AST_STATEMENT_PRINT:
        printf("print");
        printExpression(1, node->u.print_.expressions);
        break;
    case AST_STATEMENT_RETURN:
        printf("return");
        if (node->u.return_.expression)
            printExpression(1, node->u.return_.expression);
        break;
    case AST_STATEMENT_CALL:
        printExpression(0, node->u.call_);
        break;
    }

    if (node->tag != AST_STATEMENT_CALL)
        printf(")");
    printStatement(spaces, node->next);
}

static void printExpression(int space, AstExpression* node)
{
    if (!node) return;

    if (space)
        printf(" ");

    switch (node->tag) {
    case AST_EXPRESSION_KINT:
        printf("%d", node->u.kint_);
        break;
    case AST_EXPRESSION_KFLOAT:
        printf("%f", node->u.kfloat_);
        break;
    case AST_EXPRESSION_STRING:
        printf("\"%s\"", node->u.string_);
        break;
    case AST_EXPRESSION_NULL:
        printf("null");
        break;
    case AST_EXPRESSION_KBOOL:
        printf("%s", node->u.kbool_ ? "true" : "false");
        break;
    case AST_EXPRESSION_CALL:
        if (node->u.call_.is_declaration)
        {
            AstDeclaration* decl = node->u.call_.u.declaration_;
            printf("(%s<%d>", decl->identifier, decl->line);
            printExpression(1, node->u.call_.expressions);
            printf(")");
        } else {
            printf("(%s", node->u.call_.u.identifier_);
            printExpression(1, node->u.call_.expressions);
            printf(")");
        }
        break;
    case AST_EXPRESSION_VARIABLE:
        printVariable(node->u.variable_);
        break;
    case AST_EXPRESSION_NEW:
        printf("(new ");
        TypePrint(node->u.new_.type);
        printf("[");
        printExpression(0, node->u.new_.expression);
        printf("])");
        break;
    case AST_EXPRESSION_UNARY:
        printf("(");
        printf("%s", AstPrintUnaryOperator(node->u.unary_.operator));
        printExpression(1, node->u.unary_.expression);
        printf(")");
        break;
    case AST_EXPRESSION_BINARY:
        printf("(");
        printf("%s", AstPrintBinaryOperator(node->u.binary_.operator));
        printExpression(1, node->u.binary_.expression_left);
        printExpression(1, node->u.binary_.expression_right);
        printf(")");
        break;
    case AST_EXPRESSION_CAST:
        printf("(");
        printCast(node->u.cast_.tag);
        printExpression(1, node->u.cast_.expression);
        printf(")");
        break;
    }

    if (node->type.tag != TYPE_UNDEFINED) {
        printf(":");
        TypePrint(node->type);
    }

    printExpression(1, node->next);
}

static void printCast(AstCastTag tag)
{
    switch (tag) {
    case AST_CAST_INT_TO_FLOAT: printf("int->float"); break;
    case AST_CAST_FLOAT_TO_INT: printf("float->int"); break;
    }
}

static void printVariable(AstVariable* node)
{
    switch (node->tag) {
    case AST_VARIABLE_REFERENCE:
        if (node->u.reference_.is_declaration) {
            AstDeclaration* decl = node->u.reference_.u.declaration_;
            printf("%s<%d>", decl->identifier, decl->line);
        } else {
            printf("%s", node->u.reference_.u.identifier_);
        }
        break;
    case AST_VARIABLE_ARRAY:
        printExpression(0, node->u.array_.location);
        printf("[");
        printExpression(0, node->u.array_.offset);
        printf("]");
        break;
    }
}

