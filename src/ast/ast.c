/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * ast.c
 */

#include "ast.h"

#include "util/new.h"

ast_decl_t* ast_decl_variable(type_t type, char* identifier, int line)
{
    ast_decl_t* node = NEW(ast_decl_t);
    node->tag = DECL_VARIABLE;
    node->type = type;
    node->identifier = identifier;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.variable_.global = false;
    node->u.variable_.offset = 0;
    return node;
}

ast_decl_t* ast_decl_function(type_t type, char* identifier, int line,
        ast_decl_t* parameters, ast_cmd_t* block)
{
    ast_decl_t* node = NEW(ast_decl_t);
    node->tag = DECL_FUNCTION;
    node->type = type;
    node->identifier = identifier;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.function_.parameters = parameters;
    node->u.function_.block = block;
    node->u.function_.space = 0;
    return node;
}

ast_decl_t* ast_decl_prototype(type_t type, char* identifier, int line,
        ast_decl_t* parameters)
{
    ast_decl_t* node = NEW(ast_decl_t);
    node->tag = DECL_PROTOTYPE;
    node->type = type;
    node->identifier = identifier;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.prototype_.parameters = parameters;
    return node;
}

ast_cmd_t* ast_cmd_block(ast_decl_t* variables, ast_cmd_t* commands,
        int line)
{
    ast_cmd_t* node = NEW(ast_cmd_t);
    node->tag = CMD_BLOCK;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.block_.variables = variables;
    node->u.block_.commands = commands;
    return node;
}

ast_cmd_t* ast_cmd_if(ast_exp_t* expression, ast_cmd_t* command_if,
        ast_cmd_t* command_else, int line)
{
    ast_cmd_t* node = NEW(ast_cmd_t);
    node->tag = CMD_IF;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.if_.expression = expression;
    node->u.if_.command_if = command_if;
    node->u.if_.command_else = command_else;
    return node;
}

ast_cmd_t* ast_cmd_while(ast_exp_t* expression, ast_cmd_t* command,
        int line)
{
    ast_cmd_t* node = NEW(ast_cmd_t);
    node->tag = CMD_WHILE;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.while_.expression = expression;
    node->u.while_.command = command;
    return node;
}

ast_cmd_t* ast_cmd_assign(ast_var_t* variable, ast_exp_t* expression,
        int line)
{
    ast_cmd_t* node = NEW(ast_cmd_t);
    node->tag = CMD_ASSIGN;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.assign_.variable = variable;
    node->u.assign_.expression = expression;
    return node;
}

ast_cmd_t* ast_cmd_delete(ast_exp_t* expression, int line)
{
    ast_cmd_t* node = NEW(ast_cmd_t);
    node->tag = CMD_DELETE;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.delete_.expression = expression;
    return node;
}

ast_cmd_t* ast_cmd_return(ast_exp_t* expression, int line)
{
    ast_cmd_t* node = NEW(ast_cmd_t);
    node->tag = CMD_RETURN;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.return_.expression = expression;
    return node;
}

ast_cmd_t* ast_cmd_call(ast_exp_t* call, int line)
{
    ast_cmd_t* node = NEW(ast_cmd_t);
    node->tag = CMD_CALL;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.call_ = call;
    return node;
}

ast_exp_t* ast_exp_kint(int value)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_KINT;
    node->line = 0;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.kint_ = value;
    return node;
}

ast_exp_t* ast_exp_kfloat(float value)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_KFLOAT;
    node->line = 0;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.kfloat_ = value;
    return node;
}

ast_exp_t* ast_exp_string(char* string)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_STRING;
    node->line = 0;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.string_ = string;
    return node;
}

ast_exp_t* ast_exp_null()
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_NULL;
    node->line = 0;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    return node;
}

ast_exp_t* ast_exp_bool(int value)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_BOOL;
    node->line = 0;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.bool_ = value;
    return node;
}

ast_exp_t* ast_exp_call(char* identifier, ast_exp_t* expressions, int line)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_CALL_ID;
    node->line = line;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.call_id_.identifier = identifier;
    node->u.call_id_.expressions = expressions;
    return node;
}

ast_exp_t* ast_exp_variable(ast_var_t* variable, int line)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_VARIABLE;
    node->line = line;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.variable_ = variable;
    return node;
}

ast_exp_t* ast_exp_new(type_t type, ast_exp_t* expression, int line)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_NEW;
    node->line = line;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.new_.type = type;
    node->u.new_.expression = expression;
    return node;
}

ast_exp_t* ast_exp_unary(ast_unop operator, ast_exp_t* expression,
        int line)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_UNARY;
    node->line = line;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.unary_.operator = operator;
    node->u.unary_.expression = expression;
    return node;
}

ast_exp_t* ast_exp_binary(ast_binop operator,
        ast_exp_t* expression_left, ast_exp_t* expression_right, int line)
{
    ast_exp_t* node = NEW(ast_exp_t);
    node->tag = EXP_BINARY;
    node->line = line;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.binary_.operator = operator;
    node->u.binary_.expression_left = expression_left;
    node->u.binary_.expression_right = expression_right;
    return node;
}

void ast_exp_cast(ast_exp_t* expression, type_t goal_type,
        ast_cast_tag cast_tag)
{
    ast_exp_t* subexpression = NEW(ast_exp_t);
    *subexpression = *expression;
    subexpression->next = NULL;
    expression->tag = EXP_CAST;
    expression->type = goal_type;
    expression->u.cast_.tag = cast_tag;
    expression->u.cast_.expression = subexpression;
}

ast_var_t* ast_var_identifier(char* identifier, int line)
{
    ast_var_t* node = NEW(ast_var_t);
    node->tag = VAR_IDENTIFIER;
    node->line = line;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->u.identifier_ = identifier;
    return node;
}

ast_var_t* ast_var_array(ast_exp_t* location, ast_exp_t* offset, int line)
{
    ast_var_t* node = NEW(ast_var_t);
    node->tag = VAR_ARRAY;
    node->line = line;
    node->type = type_create(TYPE_UNDEFINED, 0);
    node->u.array_.location = location;
    node->u.array_.offset = offset;
    return node;
}

