/*
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * assembly.c
 */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "assembly.h"

#include "scanner/scanner.h"
#include "util/new.h"

static int curr_label = 0;
static ast_decl_t* curr_function = NULL;

static char* gen_string_symbol(const char* id);
static char* gen_function_end(ast_decl_t* func);
static char* gen_label();
static char* gen_float_value(float f);
static char* cat_dolar(char* symbol);
static void asm_section(const char* section, const char* symbol);
static void asm_variable(const char* name, const char* type, const char* value);
static void asm_label(const char* label);
static void asm_unary(const char* cmd);
static void asm_binary(const char* cmd, const char* reg);
static void asm_binaryk(const char* cmd, int val);
static void asm_ternary(const char* cmd, const char* from, const char* to);
static void asm_ternaryk(const char* cmd, int from, const char* to);
static void asm_comment(const char* format, ...);
static void assembly_strings();
static void assembly_global(ast_decl_t* decl);
static void assembly_function(ast_decl_t* decl);
static void assembly_cmd(ast_cmd_t* cmd);
static void assembly_if(ast_cmd_t* cmd);
static void assembly_while(ast_cmd_t* cmd);
static void assembly_assign(ast_cmd_t* cmd);
static void assembly_delete(ast_cmd_t* cmd);
static void assembly_return(ast_cmd_t* cmd);
static void assembly_exp_push(ast_exp_t* exp);
static void assembly_exp(ast_exp_t* exp);
static void assembly_exp_var(ast_exp_t* exp);
static void assembly_exp_new(ast_exp_t* exp);
static void assembly_exp_unary(ast_exp_t* exp);
static void assembly_exp_binary(ast_exp_t* exp);
static void assembly_exp_binary_int(ast_exp_t* exp);
static void assembly_exp_binary_float(ast_exp_t* exp);
static void assembly_exp_cast(ast_exp_t* exp);
static void assembly_exp_bool(ast_exp_t* exp);
static void assembly_jump_true(ast_exp_t* exp, char* label);
static void assembly_jump_true_binary(ast_exp_t* exp, char* label);
static void assembly_jump_true_binary_int(ast_exp_t* exp, char* label);
static void assembly_jump_true_binary_float(ast_exp_t* exp, char* label);
static void assembly_jump_false(ast_exp_t* exp, char* label);
static void assembly_jump_false_binary(ast_exp_t* exp, char* label);
static void assembly_jump_false_binary_int(ast_exp_t* exp, char* label);
static void assembly_jump_false_binary_float(ast_exp_t* exp, char* label);
static void assembly_call(ast_exp_t* call);
static void assembly_var(ast_var_t* var);

void assembly(ast_decl_t* tree)
{
    asm_section(".data", "");
    assembly_strings();
    assembly_global(tree);
    assembly_function(tree);
}

static char* gen_string_symbol(const char* str)
{
    char* output = NEW_ARRAY(char, 64);
    sprintf(output, "s$%p", str);
    return output;
}

static char* gen_function_end(ast_decl_t* func)
{
    char* output = NEW_ARRAY(char, strlen(func->identifier) + 5);
    sprintf(output, "end$%s", func->identifier);
    return output;
}

static char* gen_label()
{
    char* output = NEW_ARRAY(char, 64);
    sprintf(output, "l$%d", curr_label++);
    return output;
}

static char* gen_float_value(float f)
{
    char* value = NEW_ARRAY(char, 64);
    volatile union {
        float f;
        unsigned int ul;
    } union_value;
    union_value.f = f;
    sprintf(value, "$0x%x", union_value.ul);
    return value;
}

static char* cat_dolar(char* symbol)
{
    char* output = NEW_ARRAY(char, strlen(symbol) + 2);
    strcpy(output, "$");
    strcat(output, symbol);
    free(symbol);
    return output;
}

static void asm_section(const char* section, const char* symbol)
{
    printf("%s %s\n", section, symbol);
}

static void asm_variable(const char* name, const char* type, const char* value)
{
    printf("%s: %s %s\n", name, type, value);
}

static void asm_label(const char* label)
{
    printf("%s:\n", label);
}

static void asm_unary(const char* cmd)
{
    printf("\t%s\n", cmd);
}

static void asm_binary(const char* cmd, const char* value)
{
    printf("\t%s\t%s\n", cmd, value);
}

static void asm_binaryk(const char* cmd, int value)
{
    printf("\t%s\t$%d\n", cmd, value);
}

static void asm_ternary(const char* cmd, const char* from, const char* to)
{
    printf("\t%s\t%s, %s\n", cmd, from, to);
}

static void asm_ternaryk(const char* cmd, int from, const char* to)
{
    printf("\t%s\t$%d, %s\n", cmd, from, to);
}

static void asm_comment(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\t/* ");
    vprintf(format, args);
    printf(" */\n");
    va_end(args);
}

static void assembly_strings()
{
    vector_t* strings = scanner_get_strings();
    size_t i = 0;

    if (strings == NULL)
        return;

    for (i = 0; i < vector_size(strings); ++i) {
        const char* str = vector_get(strings, i);
        char* symbol = gen_string_symbol(str);
        char* value = NEW_ARRAY(char, 4 * strlen(str) + 5);
        size_t j = 0;

        value[0] = '\0';
        strcpy(value, "\"");
        for (j = 0; str[j] != '\0'; ++j) {
            char buffer[5];
                if (isgraph(str[j]) || str[j] == ' ')
                sprintf(buffer, "%c", str[j]);
            else
                sprintf(buffer, "\\%o", str[j]);
            strcat(value, buffer);
        }
        strcat(value, "\\0\"");
        asm_variable(symbol, ".ascii", value);
        free(symbol);
        free(value);
    }
}

static void assembly_global(ast_decl_t* decl)
{
    ast_decl_t* global = NULL;

    for (global = decl; global != NULL; global = global->next) {
        const char* type = NULL;

        if (global->tag != DECL_VARIABLE)
            continue;

        if (global->type.pointers > 0) {
            type = ".long";
        } else {
            switch (global->type.tag) {
            case TYPE_VOID:
            case TYPE_NULL:
            case TYPE_UNDEFINED:
                assert(false);
                break;
            case TYPE_BOOL:
            case TYPE_CHAR:
                type = ".byte";
                break;
            case TYPE_INT:
                type = ".long";
                break;
            case TYPE_FLOAT:
                type = ".float";
                break;
            }
        }
        asm_variable(global->identifier, type, "0");
    }
}

static void assembly_function(ast_decl_t* decl)
{
    ast_decl_t* func = NULL;

    for (func = decl; func != NULL; func = func->next) {
        int space = 0;
        char* end = NULL;

        if (func->tag != DECL_FUNCTION)
            continue;

        asm_section(".global", func->identifier);
        asm_section(".text", "");

        curr_function = func;
        asm_label(func->identifier);
        asm_binary("pushl", "%ebp");
        asm_ternary("movl", "%esp", "%ebp");
        space = func->u.function_.space;
        if (space > 0)
            asm_ternaryk("subl", space, "%esp");
        assembly_cmd(func->u.function_.block);
        end = gen_function_end(func);
        asm_label(end);
        free(end);
        if (space > 0)
            asm_ternaryk("addl", space, "%esp");
        asm_ternary("movl", "%ebp", "%esp");
        asm_binary("popl", "%ebp");
        asm_unary("ret");
    }
}

static void assembly_cmd(ast_cmd_t* cmd)
{
    if (cmd == NULL) return;

    asm_comment("Line: %d", cmd->line);

    switch (cmd->tag) {
    case CMD_BLOCK:
        assembly_cmd(cmd->u.block_.commands);
        break;
    case CMD_IF:
        assembly_if(cmd);
        break;
    case CMD_WHILE:
        assembly_while(cmd);
        break;
    case CMD_ASSIGN:
        assembly_assign(cmd);
        break;
    case CMD_DELETE:
        assembly_delete(cmd);
        break;
    case CMD_RETURN:
        assembly_return(cmd);
        break;
    case CMD_CALL:
        assembly_exp(cmd->u.call_);
        break;
    }

    assembly_cmd(cmd->next);
}

static void assembly_if(ast_cmd_t* cmd)
{
    if (cmd->u.if_.command_else == NULL) {
        char* label = gen_label();

        assembly_jump_false(cmd->u.if_.expression, label);
        assembly_cmd(cmd->u.if_.command_if);
        asm_label(label);

        free(label);
    } else {
        char* label_else = gen_label();
        char* label_end = gen_label();

        assembly_jump_false(cmd->u.if_.expression, label_else);
        assembly_cmd(cmd->u.if_.command_if);
        asm_binary("jmp", label_end);
        asm_label(label_else);
        assembly_cmd(cmd->u.if_.command_else);
        asm_label(label_end);

        free(label_else);
        free(label_end);
    }
}

static void assembly_while(ast_cmd_t* cmd)
{
    char* label_begin = gen_label();
    char* label_end = gen_label();

    asm_label(label_begin);
    assembly_jump_false(cmd->u.while_.expression, label_end);
    assembly_cmd(cmd->u.while_.command);
    asm_binary("jmp", label_begin);
    asm_label(label_end);

    free(label_begin);
    free(label_end);
}

static void assembly_assign(ast_cmd_t* cmd)
{
    assembly_exp_push(cmd->u.assign_.expression);
    assembly_var(cmd->u.assign_.variable);
    asm_binary("popl", "%eax");

    if (cmd->u.assign_.variable->type.pointers > 0) {
        asm_ternary("movl", "%eax", "(%ecx)");
    } else {
        switch (cmd->u.assign_.variable->type.tag) {
        case TYPE_VOID:
        case TYPE_NULL:
        case TYPE_UNDEFINED:
            assert(false);
            break;
        case TYPE_BOOL:
        case TYPE_CHAR:
            asm_ternary("movb", "%al", "(%ecx)");
            break;
        case TYPE_INT:
        case TYPE_FLOAT:
            asm_ternary("movl", "%eax", "(%ecx)");
            break;
        }
    }
}

static void assembly_delete(ast_cmd_t* cmd)
{
    assembly_exp_push(cmd->u.delete_.expression);
    asm_binary("call", "free");
    asm_ternaryk("addl", 4, "%esp");
}

static void assembly_return(ast_cmd_t* cmd)
{
    char* end = gen_function_end(curr_function);
    if (cmd->u.return_.expression != NULL)
        assembly_exp(cmd->u.return_.expression);
    asm_binary("jmp", end);
    free(end);
}

static void assembly_exp_push(ast_exp_t* exp)
{
    switch (exp->tag) {
    case EXP_KINT:
        asm_binaryk("pushl", exp->u.kint_);
        break;
    case EXP_KFLOAT: {
        char* value = gen_float_value(exp->u.kfloat_);
        asm_binary("pushl", value);
        free(value);
        break;
    }
    case EXP_STRING: {
        char* symbol = cat_dolar(gen_string_symbol(exp->u.string_));
        asm_binary("pushl", symbol);
        free(symbol);
        break;
    }
    case EXP_NULL:
        asm_binaryk("pushl", 0);
        break;
    case EXP_BOOL:
        asm_binaryk("pushl", exp->u.bool_);
        break;
    case EXP_CALL_ID:
    case EXP_CALL_DECL:
    case EXP_VARIABLE:
    case EXP_NEW:
    case EXP_UNARY:
    case EXP_BINARY:
    case EXP_CAST:
        assembly_exp(exp);
        if (exp->type.pointers > 0) {
            asm_binary("pushl", "%eax");
        } else {
            switch (exp->type.tag) {
            case TYPE_VOID:
            case TYPE_UNDEFINED:
                assert(false);
                break;
            case TYPE_NULL:
            case TYPE_BOOL:
            case TYPE_CHAR:
            case TYPE_INT:
                asm_binary("pushl", "%eax");
                break;
            case TYPE_FLOAT:
                asm_ternaryk("subl", 4, "%esp");
                asm_binary("fstps", "(%esp)");
                break;
            }
        }
    }
}

static void assembly_exp(ast_exp_t* exp)
{
    switch (exp->tag) {
    case EXP_KINT:
        asm_ternaryk("movl", exp->u.kint_, "%eax");
        break;
    case EXP_KFLOAT: {
        char* value = gen_float_value(exp->u.kfloat_);
        asm_binary("pushl", value);
        asm_binary("flds", "(%esp)");
        asm_ternaryk("addl", 4, "%esp");
        free(value);
        break;
    }
    case EXP_STRING: {
        char* symbol = cat_dolar(gen_string_symbol(exp->u.string_));
        asm_ternary("movl", symbol, "%eax");
        free(symbol);
        break;
    }
    case EXP_NULL:
        asm_ternaryk("movl", 0, "%eax");
        break;
    case EXP_BOOL:
        asm_ternaryk("movl", exp->u.bool_, "%eax");
        break;
    case EXP_CALL_ID:
        assert(false);
        break;
    case EXP_CALL_DECL:
        assembly_call(exp);
        break;
    case EXP_VARIABLE:
        assembly_exp_var(exp);
        break;
    case EXP_NEW:
        assembly_exp_new(exp);
        break;
    case EXP_UNARY:
        assembly_exp_unary(exp);
        break;
    case EXP_BINARY:
        assembly_exp_binary(exp);
        break;
    case EXP_CAST:
        assembly_exp_cast(exp);
        break;
    }
}

static void assembly_exp_var(ast_exp_t* exp)
{
    assembly_var(exp->u.variable_);
    if (exp->u.variable_->type.pointers > 0) {
        asm_ternary("movl", "(%ecx)", "%eax");
    } else {
        switch (exp->u.variable_->type.tag) {
        case TYPE_VOID:
        case TYPE_NULL:
        case TYPE_UNDEFINED:
            assert(false);
            break;
        case TYPE_BOOL:
        case TYPE_CHAR:
            asm_ternary("movzbl", "(%ecx)", "%eax");
            break;
        case TYPE_INT:
            asm_ternary("movl", "(%ecx)", "%eax");
            break;
        case TYPE_FLOAT:
            asm_binary("flds", "(%ecx)");
            break;
        }
    }
}

static void assembly_exp_new(ast_exp_t* exp)
{
    char* label = gen_label();

    /* eax <- size */
    assembly_exp(exp->u.new_.expression);

    /* if sizeof type > 4 then eax =<< 2 */
    if ((exp->u.new_.type.pointers > 0) ||
        (exp->u.new_.type.tag != TYPE_BOOL &&
         exp->u.new_.type.tag != TYPE_CHAR))
        asm_ternaryk("shl", 2, "%eax");

    /* if eax < 0 then error */
    asm_ternaryk("cmp", 0, "%eax");
    asm_binary("jge", label);
    asm_ternaryk("movl", 1, "%eax");
    asm_ternaryk("movl", 2, "%ecx");
    asm_binaryk("int", 0x80);

    /* else call malloc */
    asm_label(label);
    asm_binary("pushl", "%eax");
    asm_binary("call", "malloc");
    asm_ternaryk("addl", 4, "%esp");

    free(label);
}

static void assembly_exp_unary(ast_exp_t* exp)
{
    if (exp->type.pointers > 0) {
        assert(false);
    } else {
        switch (exp->type.tag) {
        case TYPE_VOID:
        case TYPE_NULL:
        case TYPE_UNDEFINED:
        case TYPE_CHAR:
            assert(false);
            break;
        case TYPE_BOOL:
            if (exp->u.unary_.operator != OP_NOT)
                assert(false);
            assembly_exp_bool(exp);
            break;
        case TYPE_INT:
            if (exp->u.unary_.operator != OP_NEGATE)
                assert(false);
            assembly_exp(exp->u.unary_.expression);
            asm_binary("neg", "%eax");
            break;
        case TYPE_FLOAT:
            if (exp->u.unary_.operator != OP_NEGATE)
                assert(false);
            assembly_exp(exp->u.unary_.expression);
            asm_unary("fchs");
            break;
        }
    }
}

static void assembly_exp_binary(ast_exp_t* exp)
{
    if (exp->type.pointers > 0) {
        assert(false);
    } else {
        switch (exp->type.tag) {
        case TYPE_VOID:
        case TYPE_NULL:
        case TYPE_UNDEFINED:
        case TYPE_CHAR:
            assert(false);
            break;
        case TYPE_BOOL:
            assembly_exp_bool(exp);
            break;
        case TYPE_INT:
            assembly_exp_binary_int(exp);
            break;
        case TYPE_FLOAT:
            assembly_exp_binary_float(exp);
            break;
        }
    }
}

static void assembly_exp_binary_int(ast_exp_t* exp)
{
    assembly_exp_push(exp->u.binary_.expression_right);
    assembly_exp(exp->u.binary_.expression_left);
    asm_binary("popl", "%ecx");

    switch (exp->u.binary_.operator) {
    case OP_ADD:
        asm_ternary("addl", "%ecx", "%eax");
        break;
    case OP_SUB:
        asm_ternary("subl", "%ecx", "%eax");
        break;
    case OP_MUL:
        asm_ternary("imull", "%ecx", "%eax");
        break;
    case OP_DIV:
        asm_unary("cdq");
        asm_binary("idivl", "%ecx");
        break;
    case OP_EQUALS:
    case OP_NOT_EQUALS:
    case OP_LESS:
    case OP_LESS_EQUALS:
    case OP_GREATER:
    case OP_GREATER_EQUALS:
    case OP_AND:
    case OP_OR:
        assert(false);
    }
}

static void assembly_exp_binary_float(ast_exp_t* exp)
{
    assembly_exp_push(exp->u.binary_.expression_left);
    assembly_exp(exp->u.binary_.expression_right);
    asm_binary("flds", "(%esp)");
    asm_ternaryk("addl", 4, "%esp");

    switch (exp->u.binary_.operator) {
    case OP_ADD:
        asm_unary("faddp");
        break;
    case OP_SUB:
        asm_unary("fsubp");
        break;
    case OP_MUL:
        asm_unary("fmulp");
        break;
    case OP_DIV:
        asm_unary("fdivp");
        break;
    case OP_EQUALS:
    case OP_NOT_EQUALS:
    case OP_LESS:
    case OP_LESS_EQUALS:
    case OP_GREATER:
    case OP_GREATER_EQUALS:
    case OP_AND:
    case OP_OR:
        assert(false);
    }
}

static void assembly_exp_cast(ast_exp_t* exp)
{
    switch (exp->u.cast_.tag) {
    case CAST_INT_TO_FLOAT:
        assembly_exp_push(exp->u.cast_.expression);
        asm_binary("fildl", "(%esp)");
        asm_ternaryk("addl", 4, "%esp");
        break;
    case CAST_FLOAT_TO_INT:
        assembly_exp(exp->u.cast_.expression);
        asm_ternaryk("subl", 4, "%esp");
        asm_binary("fistpl", "(%esp)");
        asm_binary("pop", "%eax");
        break;
    }
}

static void assembly_exp_bool(ast_exp_t* exp)
{
    char* label_false = gen_label();
    char* label_end = gen_label();

    assembly_jump_false(exp, label_false);
    asm_ternaryk("movl", 1, "%eax");
    asm_binary("jmp", label_end);
    asm_label(label_false);
    asm_ternaryk("movl", 0, "%eax");
    asm_label(label_end);

    free(label_false);
    free(label_end);
}

static void assembly_jump_true(ast_exp_t* exp, char* label)
{
    switch (exp->tag) {
    case EXP_BOOL:
        if (exp->u.bool_)
            asm_binary("jmp", label);
        break;
    case EXP_CALL_DECL:
    case EXP_VARIABLE:
        assembly_exp(exp);
        asm_ternaryk("cmp", 0, "%eax");
        asm_binary("jne", label);
        break;
    case EXP_UNARY:
        if (exp->u.unary_.operator != OP_NOT)
            assert(false);
        assembly_jump_false(exp->u.unary_.expression, label);
        break;
    case EXP_BINARY:
        assembly_jump_true_binary(exp, label);
        break;
    case EXP_KINT:
    case EXP_KFLOAT:
    case EXP_STRING:
    case EXP_NULL:
    case EXP_CALL_ID:
    case EXP_NEW:
    case EXP_CAST:
        assert(false);
    }
}

static void assembly_jump_true_binary(ast_exp_t* exp, char* label)
{
    switch (exp->u.binary_.operator) {
    case OP_EQUALS:
    case OP_NOT_EQUALS:
    case OP_LESS:
    case OP_LESS_EQUALS:
    case OP_GREATER:
    case OP_GREATER_EQUALS:
        if (exp->u.binary_.expression_left->type.tag == TYPE_FLOAT)
            assembly_jump_true_binary_float(exp, label);
        else
            assembly_jump_true_binary_int(exp, label);
        break;
    case OP_AND: {
        char* label_end = gen_label();
        assembly_jump_false(exp->u.binary_.expression_left, label_end);
        assembly_jump_true(exp->u.binary_.expression_right, label);
        asm_label(label_end);
        free(label_end);
        break;
    }
    case OP_OR:
        assembly_jump_true(exp->u.binary_.expression_left, label);
        assembly_jump_true(exp->u.binary_.expression_right, label);
        break;
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
        assert(false);
    }
}

static void assembly_jump_true_binary_int(ast_exp_t* exp, char* label)
{
    assembly_exp_push(exp->u.binary_.expression_right);
    assembly_exp(exp->u.binary_.expression_left);
    asm_binary("popl", "%ecx");
    asm_ternary("cmp", "%ecx", "%eax");

    switch (exp->u.binary_.operator) {
    case OP_EQUALS:
        asm_binary("je", label);
        break;
    case OP_NOT_EQUALS:
        asm_binary("jne", label);
        break;
    case OP_LESS:
        asm_binary("jl", label);
        break;
    case OP_LESS_EQUALS:
        asm_binary("jle", label);
        break;
    case OP_GREATER:
        asm_binary("jg", label);
        break;
    case OP_GREATER_EQUALS:
        asm_binary("jge", label);
        break;
    case OP_AND:
    case OP_OR:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
        assert(false);
    }
}

static void assembly_jump_true_binary_float(ast_exp_t* exp, char* label)
{
    assembly_exp_push(exp->u.binary_.expression_left);
    assembly_exp(exp->u.binary_.expression_right);
    asm_binary("flds", "(%esp)");
    asm_ternaryk("addl", 4, "%esp");
    asm_binary("fcomip", "%st(1)");
    asm_binary("ffree", "%st(0)");

    switch (exp->u.binary_.operator) {
    case OP_EQUALS:
        asm_binary("je", label);
        break;
    case OP_NOT_EQUALS:
        asm_binary("jne", label);
        break;
    case OP_LESS:
        asm_binary("jb", label);
        break;
    case OP_LESS_EQUALS:
        asm_binary("jbe", label);
        break;
    case OP_GREATER:
        asm_binary("ja", label);
        break;
    case OP_GREATER_EQUALS:
        asm_binary("jae", label);
        break;
    case OP_AND:
    case OP_OR:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
        assert(false);
    }
}

static void assembly_jump_false(ast_exp_t* exp, char* label)
{
    switch (exp->tag) {
    case EXP_BOOL:
        if (!exp->u.bool_)
            asm_binary("jmp", label);
        break;
    case EXP_CALL_DECL:
    case EXP_VARIABLE:
        assembly_exp(exp);
        asm_ternaryk("cmp", 0, "%eax");
        asm_binary("je", label);
        break;
    case EXP_UNARY:
        if (exp->u.unary_.operator != OP_NOT)
            assert(false);
        assembly_jump_true(exp->u.unary_.expression, label);
        break;
    case EXP_BINARY:
        assembly_jump_false_binary(exp, label);
        break;
    case EXP_KINT:
    case EXP_KFLOAT:
    case EXP_STRING:
    case EXP_NULL:
    case EXP_CALL_ID:
    case EXP_NEW:
    case EXP_CAST:
        assert(false);
    }
}

static void assembly_jump_false_binary(ast_exp_t* exp, char* label)
{
    switch (exp->u.binary_.operator) {
    case OP_EQUALS:
    case OP_NOT_EQUALS:
    case OP_LESS:
    case OP_LESS_EQUALS:
    case OP_GREATER:
    case OP_GREATER_EQUALS:
        if (exp->u.binary_.expression_left->type.tag == TYPE_FLOAT)
            assembly_jump_false_binary_float(exp, label);
        else
            assembly_jump_false_binary_int(exp, label);
        break;
    case OP_AND:
        assembly_jump_false(exp->u.binary_.expression_left, label);
        assembly_jump_false(exp->u.binary_.expression_right, label);
        break;
    case OP_OR: {
        char* label_end = gen_label();
        assembly_jump_true(exp->u.binary_.expression_left, label_end);
        assembly_jump_false(exp->u.binary_.expression_right, label);
        asm_label(label_end);
        free(label_end);
        break;
    }
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
        assert(false);
    }
}

static void assembly_jump_false_binary_int(ast_exp_t* exp, char* label)
{
    assembly_exp_push(exp->u.binary_.expression_right);
    assembly_exp(exp->u.binary_.expression_left);
    asm_binary("popl", "%ecx");
    asm_ternary("cmp", "%ecx", "%eax");

    switch (exp->u.binary_.operator) {
    case OP_EQUALS:
        asm_binary("jne", label);
        break;
    case OP_NOT_EQUALS:
        asm_binary("je", label);
        break;
    case OP_LESS:
        asm_binary("jnl", label);
        break;
    case OP_LESS_EQUALS:
        asm_binary("jnle", label);
        break;
    case OP_GREATER:
        asm_binary("jng", label);
        break;
    case OP_GREATER_EQUALS:
        asm_binary("jnge", label);
        break;
    case OP_AND:
    case OP_OR:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
        assert(false);
    }
}

static void assembly_jump_false_binary_float(ast_exp_t* exp, char* label)
{
    char* label_end = gen_label();
    assembly_jump_true_binary_float(exp, label_end);
    asm_binary("jmp", label);
    asm_label(label_end);
    free(label_end);
}

static void assembly_call(ast_exp_t* call)
{
    int num_parameters = 0;
    ast_exp_t* param = call->u.call_decl_.expressions;
    vector_t* stack = vector_create();

    while (param != NULL) {
        vector_push(stack, param);
        param = param->next;
        num_parameters++;
    }

    while (!vector_empty(stack)) {
        param = (ast_exp_t*)vector_pop(stack);
        assembly_exp_push(param);
    }

    asm_binary("call", call->u.call_decl_.declaration->identifier);
    if (num_parameters > 0)
        asm_ternaryk("addl", num_parameters * 4, "%esp");

    vector_destroy(stack);
}

static void assembly_var(ast_var_t* var)
{
    switch (var->tag) {
    case VAR_DECLARATION: {
        ast_decl_t* decl = var->u.declaration_;
        char location[64];

        if (decl->u.variable_.global) {
            sprintf(location, "%s", decl->identifier);
        } else {
            sprintf(location, "%d(%%ebp)", decl->u.variable_.offset);
        }
        asm_ternary("lea ", location, "%ecx");
        break;
    }
    case VAR_ARRAY: {
        assembly_exp_push(var->u.array_.offset);
        assembly_exp(var->u.array_.location);
        asm_binary("popl", "%edx");

        if ((var->type.pointers > 0) ||
            (var->type.tag != TYPE_BOOL &&
             var->type.tag != TYPE_CHAR))
            asm_ternary("lea ", "(%eax, %edx, 4)", "%ecx");
        else
            asm_ternary("lea ", "(%eax, %edx)", "%ecx");
            
        break;
    }
    case VAR_IDENTIFIER:
        assert(false);
    }
}

