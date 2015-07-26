/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * locations_test.c
 */

#include <stdbool.h>
#include <stdio.h>

#include "backend/locations.h"
#include "parser/parser.h"
#include "semantic/semantic.h"

static void print_decl(ast_decl_t* decl, bool global);
static void print_cmd(ast_cmd_t* cmd);

int main()
{
    if (!yyparse()) {
        semantic_analysis(parser_ast);
        locations_compute(parser_ast);
        print_decl(parser_ast, true);
    }
    return 0;
}

static void print_decl(ast_decl_t* decl, bool global)
{
    if (!decl) return;

    switch (decl->tag) {
    case DECL_FUNCTION:
        printf("function: %s --- ", decl->identifier); 
        if (decl->u.function_.space > 0)
            printf("subl $%d, %%ebp\n", decl->u.function_.space);
        else
            printf("0\n");
        print_decl(decl->u.function_.parameters, false);
        print_cmd(decl->u.function_.block);
        break;
    case DECL_VARIABLE:
        if (global)
            printf("global: %s\n", decl->identifier);
        else
            printf("%s --- movl %d(%%ebp), %%eax\n", decl->identifier,
                    decl->u.variable_.offset);
        break;
    case DECL_PROTOTYPE:
        break;
    }

    print_decl(decl->next, global);
}

static void print_cmd(ast_cmd_t* cmd)
{
    if (!cmd) return;

    switch (cmd->tag) {
    case CMD_BLOCK:
        print_decl(cmd->u.block_.variables, false);
        print_cmd(cmd->u.block_.commands);
        break;
    case CMD_IF:
        print_cmd(cmd->u.if_.command_if);
        print_cmd(cmd->u.if_.command_else);
        break;
    case CMD_WHILE:
        print_cmd(cmd->u.while_.command);
        break;
    case CMD_ASSIGN:
    case CMD_DELETE:
    case CMD_RETURN:
    case CMD_CALL:
        break;
    }

    print_cmd(cmd->next);
}

