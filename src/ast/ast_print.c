/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * ast_print.c
 */

#include <stdio.h>

#include "ast.h"
#include "ast_print.h"

static void print_spaces(int spaces);
static void print_decl(int spaces, ast_decl_t* node);
static void print_cmd(int spaces, ast_cmd_t* node);
static void print_exp(int space, ast_exp_t* node);
static void print_cast(ast_cast_tag tag);
static void print_var(ast_var_t* var);

void ast_print(ast_decl_t* tree)
{
    print_decl(0, tree);
}

const char* ast_print_unop(ast_unop operator)
{
    switch (operator) {
    case OP_NEGATE: return "-";
    case OP_NOT:    return "not";
    }
    return 0;
}

const char* ast_print_binop(ast_binop operator)
{
    switch (operator) {
    case OP_ADD:            return "+";
    case OP_SUB:            return "-";
    case OP_MUL:            return "*";
    case OP_DIV:            return "/";
    case OP_EQUALS:         return "==";
    case OP_NOT_EQUALS:     return "!=";
    case OP_LESS:           return "<";
    case OP_LESS_EQUALS:    return "<=";
    case OP_GREATER:        return ">";
    case OP_GREATER_EQUALS: return ">=";
    case OP_AND:            return "and";
    case OP_OR:             return "or";
    }
    return 0;
}

static void print_spaces(int spaces)
{
    int i;
    printf("\n");
    for (i = 0; i < spaces; i++)
        printf(" ");
}

static void print_decl(int spaces, ast_decl_t* node)
{
    if (!node) return;

    print_spaces(spaces);
    printf("(");

    switch (node->tag) {
    case DECL_FUNCTION:
        printf("func");
        break;
    case DECL_PROTOTYPE:
        printf("proto");
        break;
    case DECL_VARIABLE:
        printf("var");
        break;
    }

    printf(" ");
    type_print(node->type);
    printf(" %s<%d>", node->identifier, node->line);

    switch (node->tag) {
    case DECL_FUNCTION:
        print_decl(spaces + 2, node->u.function_.parameters);
        print_cmd(spaces + 2, node->u.function_.block);
        break;
    case DECL_PROTOTYPE:
        print_decl(spaces + 2, node->u.prototype_.parameters);
        break;
    case DECL_VARIABLE:
        break;
    }

    printf(")");
    if (!spaces) printf("\n");
    print_decl(spaces, node->next);
}

static void print_cmd(int spaces, ast_cmd_t* node)
{
    if (!node) return;

    print_spaces(spaces);
    if (node->tag != CMD_CALL)
        printf("(");

    switch (node->tag) {
    case CMD_BLOCK:
        printf("block");
        print_decl(spaces + 2, node->u.block_.variables);
        print_cmd(spaces + 2, node->u.block_.commands);
        break;
    case CMD_IF:
        printf("if");
        print_exp(1, node->u.if_.expression);
        print_cmd(spaces + 4, node->u.if_.command_if);
        print_cmd(spaces + 4, node->u.if_.command_else);
        break;
    case CMD_WHILE:
        printf("while");
        print_exp(1, node->u.while_.expression);
        print_cmd(spaces + 2, node->u.while_.command);
        break;
    case CMD_ASSIGN:
        printf("assign ");
        print_var(node->u.assign_.variable);
        print_exp(1, node->u.assign_.expression);
        break;
    case CMD_DELETE:
        printf("delete");
        print_exp(1, node->u.delete_.expression);
        break;
    case CMD_RETURN:
        printf("return");
        if (node->u.return_.expression)
            print_exp(1, node->u.return_.expression);
        break;
    case CMD_CALL:
        print_exp(0, node->u.call_);
        break;
    }

    if (node->tag != CMD_CALL)
        printf(")");
    print_cmd(spaces, node->next);
}

static void print_exp(int space, ast_exp_t* node)
{
    if (!node) return;

    if (space)
        printf(" ");

    switch (node->tag) {
    case EXP_KINT:
        printf("%d", node->u.kint_);
        break;
    case EXP_KFLOAT:
        printf("%f", node->u.kfloat_);
        break;
    case EXP_STRING:
        printf("\"%s\"", node->u.string_);
        break;
    case EXP_NULL:
        printf("null");
        break;
    case EXP_BOOL:
        printf("%s", node->u.bool_ ? "true" : "false");
        break;
    case EXP_CALL_ID:
        printf("(%s", node->u.call_id_.identifier);
        print_exp(1, node->u.call_id_.expressions);
        printf(")");
        break;
    case EXP_CALL_DECL:
        printf("(%s<%d>", node->u.call_decl_.declaration->identifier,
                node->u.call_decl_.declaration->line);
        print_exp(1, node->u.call_decl_.expressions);
        printf(")");
        break;
    case EXP_VARIABLE:
        print_var(node->u.variable_);
        break;
    case EXP_NEW:
        printf("(new ");
        type_print(node->u.new_.type);
        printf("[");
        print_exp(0, node->u.new_.expression);
        printf("])");
        break;
    case EXP_UNARY:
        printf("(");
        printf(ast_print_unop(node->u.unary_.operator));
        print_exp(1, node->u.unary_.expression);
        printf(")");
        break;
    case EXP_BINARY:
        printf("(");
        printf(ast_print_binop(node->u.binary_.operator));
        print_exp(1, node->u.binary_.expression_left);
        print_exp(1, node->u.binary_.expression_right);
        printf(")");
        break;
    case EXP_CAST:
        printf("(");
        print_cast(node->u.cast_.tag);
        print_exp(1, node->u.cast_.expression);
        printf(")");
        break;
    }

    if (node->type.tag != TYPE_UNDEFINED) {
        printf(":");
        type_print(node->type);
    }

    print_exp(1, node->next);
}

static void print_cast(ast_cast_tag tag)
{
    switch (tag) {
    case CAST_INT_TO_FLOAT: printf("int->float"); break;
    case CAST_FLOAT_TO_INT: printf("float->int"); break;
    }
}

static void print_var(ast_var_t* node)
{
    switch (node->tag) {
    case VAR_IDENTIFIER:
        printf("%s", node->u.identifier_);
        break;
    case VAR_DECLARATION:
        printf("%s<%d>", node->u.declaration_->identifier,
            node->u.declaration_->line);
        break;
    case VAR_ARRAY:
        print_exp(0, node->u.array_.location);
        printf("[");
        print_exp(0, node->u.array_.offset);
        printf("]");
        break;
    }
}

