/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * semantic.c
 */

#include <stdbool.h>
#include <stdlib.h>

#include "semantic.h"

#include "ast/ast_print.h"
#include "semantic/symbols.h"
#include "util/error.h"

static void analyse_decl(ast_decl_t* node);
static void analyse_cmd(ast_cmd_t* node);
static void analyse_cmd_block(ast_cmd_t* node, bool open_scope);
static void analyse_cmd_if(ast_cmd_t* node);
static void analyse_cmd_while(ast_cmd_t* node);
static void analyse_cmd_assign(ast_cmd_t* node);
static void analyse_cmd_delete(ast_cmd_t* node);
static void analyse_cmd_return(ast_cmd_t* node);
static void analyse_exp(ast_exp_t* node);
static void analyse_exp_new(ast_exp_t* node);
static void analyse_exp_call(ast_exp_t* node);
static void analyse_exp_unary(ast_exp_t* node);
static void analyse_exp_binary(ast_exp_t* node);
static void analyse_var(ast_var_t* node);
static bool is_bool(type_t type);
static bool is_char(type_t type);
static bool is_int(type_t type);
static bool is_float(type_t type);
static bool is_array(type_t type);
static bool is_null(type_t type);
static bool is_assignable(type_t variable, type_t expression);
static bool is_void(type_t type);
static void symbol_to_exp(ast_exp_t* node);
static void exp_to_symbol(ast_exp_t* node, type_t variable);

static type_t return_type = { TYPE_UNDEFINED, 0 };

void semantic_analysis(ast_decl_t* ast)
{
    analyse_decl(ast);
}

static void analyse_decl(ast_decl_t* node)
{
    if (!node) return;

    symbols_add(node->identifier, node, node->line);

    switch (node->tag) {
    case DECL_FUNCTION:
        symbols_open_block();
        analyse_decl(node->u.function_.parameters);
        return_type = node->type;
        analyse_cmd_block(node->u.function_.block, false);
        return_type = type_create(TYPE_UNDEFINED, 0);
        symbols_close_block();
        break;
    case DECL_PROTOTYPE:
        symbols_open_block();
        analyse_decl(node->u.prototype_.parameters);
        symbols_close_block();
        break;
    case DECL_VARIABLE:
        break;
    }

    analyse_decl(node->next);
}

static void analyse_cmd(ast_cmd_t* node)
{
    if (!node) return;

    switch (node->tag) {
    case CMD_BLOCK:
        analyse_cmd_block(node, true);
        break;
    case CMD_IF:
        analyse_cmd_if(node);
        break;
    case CMD_WHILE:
        analyse_cmd_while(node);
        break;
    case CMD_ASSIGN:
        analyse_cmd_assign(node);
        break;
    case CMD_DELETE:
        analyse_cmd_delete(node);
        break;
    case CMD_RETURN:
        analyse_cmd_return(node);
        break;
    case CMD_CALL:
        analyse_exp(node->u.call_);
        break;
    }

    analyse_cmd(node->next);
}

static void analyse_cmd_block(ast_cmd_t* node, bool open_scope)
{
    if (open_scope) symbols_open_block();
    analyse_decl(node->u.block_.variables);
    analyse_cmd(node->u.block_.commands);
    if (open_scope) symbols_close_block();
}

static void analyse_cmd_if(ast_cmd_t* node)
{
    ast_exp_t* expression = node->u.if_.expression;
    analyse_exp(expression);
    analyse_cmd(node->u.if_.command_if);
    analyse_cmd(node->u.if_.command_else);
    if (!is_bool(expression->type))
        errorl(node->line, "mismatch type in if's expression, expected 'bool', "
                "read '%s'", type_to_str(expression->type));
}

static void analyse_cmd_while(ast_cmd_t* node)
{
    ast_exp_t* expression = node->u.while_.expression;
    analyse_exp(expression);
    analyse_cmd(node->u.while_.command);
    if (!is_bool(expression->type))
        errorl(node->line, "mismatch type in while's expression, expected "
                "'bool', read '%s'", type_to_str(expression->type));
}

static void analyse_cmd_assign(ast_cmd_t* node)
{
    ast_var_t* variable = node->u.assign_.variable;
    ast_exp_t* expression = node->u.assign_.expression;
    analyse_var(variable);
    analyse_exp(expression);
    if (is_assignable(variable->type, expression->type))
        exp_to_symbol(expression, variable->type);
    else
        errorl(node->line, "mismatch type in '%s = %s' assignment",
                type_to_str(variable->type), type_to_str(expression->type));
}

static void analyse_cmd_delete(ast_cmd_t* node)
{
    ast_exp_t* expression = node->u.delete_.expression;
    analyse_exp(expression);
    if (!is_array(expression->type))
        errorl(node->line, "mismatch type in delete's expression, expected an "
                "array, read '%s'", type_to_str(expression->type));
}

static void analyse_cmd_return(ast_cmd_t* node)
{
    const char* read_type = NULL;
    ast_exp_t* expression = node->u.return_.expression;
    analyse_exp(expression);
    if (expression == NULL) {
        if (!is_void(return_type))
            read_type = "void";
    } else {
        if (is_assignable(return_type, expression->type))
            exp_to_symbol(expression, return_type);
        else
            read_type = type_to_str(expression->type);
    }
    if (read_type != NULL)
        errorl(node->line, "mismatch type in return's expression, expected "
                "'%s', read '%s'", type_to_str(return_type), read_type);
}

static void analyse_exp(ast_exp_t* node)
{
    if (!node) return;

    switch (node->tag) {
    case EXP_KINT:
        node->type = type_create(TYPE_INT, 0);
        break;
    case EXP_KFLOAT:
        node->type = type_create(TYPE_FLOAT, 0);
        break;
    case EXP_STRING:
        node->type = type_create(TYPE_CHAR, 1);
        break;
    case EXP_NULL:
        node->type = type_create(TYPE_NULL, 0);
        break;
    case EXP_BOOL:
        node->type = type_create(TYPE_BOOL, 0);
        break;
    case EXP_CALL_ID:
        analyse_exp_call(node);
        break;
    case EXP_CALL_DECL:
        error("unexpected EXP_CALL_DECL expression");
        break;
    case EXP_VARIABLE:
        analyse_var(node->u.variable_);
        node->type = node->u.variable_->type;
        symbol_to_exp(node);
        break;
    case EXP_NEW:
        analyse_exp_new(node);
        break;
    case EXP_UNARY:
        analyse_exp_unary(node);
        break;
    case EXP_BINARY:
        analyse_exp_binary(node);
        break;
    case EXP_CAST:
        error("unexpected EXP_CAST expression");
        break;
    }

    analyse_exp(node->next);
}

static void analyse_exp_new(ast_exp_t* node)
{
    type_t array_type = node->u.new_.type;
    ast_exp_t* expression = node->u.new_.expression;
    analyse_exp(expression);
    if (!is_int(expression->type))
        errorl(node->line, "mismatch type in new expression, expected 'int', "
                "read '%s'", type_to_str(expression->type));
    node->type = type_create(array_type.tag, array_type.pointers + 1);
}

static void analyse_exp_call(ast_exp_t* node)
{
    char* id = node->u.call_id_.identifier;
    ast_decl_t* decl = symbols_find(id, node->line);
    ast_exp_t* exps = node->u.call_id_.expressions;
    ast_decl_t* decl_args = NULL;
    ast_exp_t* call_args = NULL;

    if (decl->tag != DECL_FUNCTION && decl->tag != DECL_PROTOTYPE)
        errorl(node->line, "cannot call non-function symbol '%s'", id);

    analyse_exp(exps);

    decl_args = decl->tag == DECL_FUNCTION ?
                decl->u.function_.parameters :
                decl->u.prototype_.parameters;
    call_args = exps;
    while (decl_args != NULL && call_args != NULL) {
        if (!is_assignable(decl_args->type, call_args->type))
            errorl(node->line, "mismatch type in parameter of '%s' function "
                    "call, cannot assign '%s' to '%s'", id,
                    type_to_str(call_args->type), type_to_str(decl_args->type));
        exp_to_symbol(call_args, decl_args->type);
        decl_args = decl_args->next;
        call_args = call_args->next;
    }
    if (decl_args != NULL || call_args != NULL)
        errorl(node->line, "mismatch number of parameters in '%s' function "
                "call", id);

    node->tag = EXP_CALL_DECL;
    node->type = decl->type;
    node->u.call_decl_.expressions = exps;
    node->u.call_decl_.declaration = decl;
    symbol_to_exp(node);
}

static void analyse_exp_unary(ast_exp_t* node)
{
    const char* expected = NULL;
    ast_unop operator = node->u.unary_.operator;
    ast_exp_t* expression = node->u.unary_.expression;

    analyse_exp(expression);
    node->type = expression->type;

    switch (operator) {
    case OP_NEGATE:
        if (!is_int(expression->type) && !is_float(expression->type))
            expected = "int' or 'float";
        break;
    case OP_NOT:
        if (!is_bool(expression->type))
            expected = "bool";
        break;
    }

    if (expected != NULL)
        errorl(node->line, "mismatch type in '%s' unary operation, expected "
                "'%s', read '%s'", ast_print_unop(operator), expected,
                type_to_str(expression->type));
}

static void analyse_exp_binary(ast_exp_t* node)
{
    bool type_error = false;
    ast_binop operator = node->u.binary_.operator;
    ast_exp_t* left = node->u.binary_.expression_left;
    ast_exp_t* right = node->u.binary_.expression_right;

    analyse_exp(left);
    analyse_exp(right);

    switch (operator) {
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
        if ((is_int(left->type) && is_int(right->type)) ||
            (is_float(left->type) && is_float(right->type))) {
            node->type = left->type;
        } else if (is_int(left->type) && is_float(right->type)) {
            node->type = right->type;
            ast_exp_cast(left, right->type, CAST_INT_TO_FLOAT);
        } else if (is_float(left->type) && is_int(right->type)) {
            node->type = left->type;
            ast_exp_cast(right, left->type, CAST_INT_TO_FLOAT);
        } else {
            type_error = true;
        }
        break;
    case OP_LESS:
    case OP_LESS_EQUALS:
    case OP_GREATER:
    case OP_GREATER_EQUALS:
        if ((is_int(left->type) && is_int(right->type)) ||
            (is_float(left->type) && is_float(right->type)))
            /* ok */;
        else if (is_int(left->type) && is_float(right->type))
            ast_exp_cast(left, right->type, CAST_INT_TO_FLOAT);
        else if (is_float(left->type) && is_int(right->type))
            ast_exp_cast(right, left->type, CAST_INT_TO_FLOAT);
        else
            type_error = true;
        node->type = type_create(TYPE_BOOL, 0);
        break;
    case OP_EQUALS:
    case OP_NOT_EQUALS:
        if ((is_bool(left->type) && is_bool(right->type)) ||
            (is_int(left->type) && is_int(right->type)) ||
            (is_float(left->type) && is_float(right->type)) ||
            (is_array(left->type) && type_cmp(left->type, right->type)) ||
            (is_array(left->type) && is_null(right->type)) ||
            (is_null(left->type) && is_array(right->type))) {
            node->type = type_create(TYPE_BOOL, 0);
        } else if (is_int(left->type) && is_float(right->type)) {
            node->type = type_create(TYPE_BOOL, 0);
            ast_exp_cast(left, right->type, CAST_INT_TO_FLOAT);
        } else if (is_float(left->type) && is_int(right->type)) {
            node->type = type_create(TYPE_BOOL, 0);
            ast_exp_cast(right, left->type, CAST_INT_TO_FLOAT);
        } else {
            type_error = true;
        }
        break;
    case OP_AND:
    case OP_OR:
        if (is_bool(left->type) && is_bool(right->type))
            node->type = type_create(TYPE_BOOL, 0);
        else
            type_error = true;
        break;
    }

    if (type_error)
        errorl(node->line, "mismatch type in '%s %s %s' binary operation",
                type_to_str(left->type), ast_print_binop(operator),
                type_to_str(right->type));
}

static void analyse_var(ast_var_t* node)
{
    switch (node->tag) {
    case VAR_IDENTIFIER: {
        char* id = node->u.identifier_;
        node->tag = VAR_DECLARATION;
        node->u.declaration_ = symbols_find(id, node->line);
        if (node->u.declaration_->tag != DECL_VARIABLE)
            errorl(node->line, "cannot access '%s' function's value", id);
        node->type = node->u.declaration_->type;
        break;
    }
    case VAR_DECLARATION:
        error("unexpected VAR_DECLARATION variable");
        break;
    case VAR_ARRAY: {
        ast_exp_t* location = node->u.array_.location;
        ast_exp_t* offset = node->u.array_.offset;
        analyse_exp(location);
        analyse_exp(offset);
        if (!is_array(location->type))
            errorl(node->line, "mismatch type in left expression of access, "
                    "expected an array, read '%s'",
                    type_to_str(location->type));
        if (!is_int(offset->type))
            errorl(node->line, "mismatch type in right expression of access, "
                    "expected 'int', read '%s'", type_to_str(offset->type));
        node->type = type_create(location->type.tag,
                location->type.pointers - 1);
        break;
    }
    }
}

static bool is_bool(type_t type)
{
    return type.tag == TYPE_BOOL && type.pointers == 0;
}

static bool is_char(type_t type)
{
    return type.tag == TYPE_CHAR && type.pointers == 0;
}

static bool is_int(type_t type)
{
    return type.tag == TYPE_INT && type.pointers == 0;
}

static bool is_float(type_t type)
{
    return type.tag == TYPE_FLOAT && type.pointers == 0;
}

static bool is_array(type_t type)
{
    return type.tag != TYPE_VOID && type.tag != TYPE_NULL &&
           type.tag != TYPE_UNDEFINED && type.pointers > 0;
}

static bool is_null(type_t type)
{
    return type.tag == TYPE_NULL;
}

static bool is_assignable(type_t variable, type_t expression)
{
    return type_cmp(variable, expression) ||
           (is_array(variable) && is_null(expression)) ||
           (is_int(variable) && is_float(expression)) ||
           (is_float(variable) && is_int(expression)) ||
           (is_char(variable) && is_float(expression)) ||
           (is_char(variable) && is_int(expression));
}

static bool is_void(type_t type)
{
    return type.tag == TYPE_VOID;
}

static void symbol_to_exp(ast_exp_t* node)
{
    if (is_char(node->type))
        node->type = type_create(TYPE_INT, 0);
}

static void exp_to_symbol(ast_exp_t* node, type_t variable)
{
    if (is_char(variable))
        variable = type_create(TYPE_INT, 0);
    if (is_int(variable) && is_float(node->type))
        ast_exp_cast(node, variable, CAST_FLOAT_TO_INT);
    else if (is_float(variable) && is_int(node->type))
        ast_exp_cast(node, variable, CAST_INT_TO_FLOAT);
}

