/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * locations.c
 */

#include <stdlib.h>

#include "locations.h"

#include "util/error.h"

static int compute_block(ast_cmd_t* block, int location);

void locations_compute(ast_decl_t* tree)
{
    if (!tree) return;

    switch (tree->tag) {
    case DECL_VARIABLE:
        tree->u.variable_.global = true;
        break;
    case DECL_FUNCTION: {
        int offset = 8;
        int space = 0;
        ast_decl_t* parameter = tree->u.function_.parameters;

        for (; parameter != NULL; parameter = parameter->next) {
            parameter->u.variable_.offset = offset;
            offset += 4;
        }
        space = compute_block(tree->u.function_.block, 0);
        tree->u.function_.space = space;
        break;
    }
    case DECL_PROTOTYPE:
        break;
    }

    locations_compute(tree->next);
}

static int compute_block(ast_cmd_t* block, int location)
{
    int max_location = 0;
    ast_decl_t* var = NULL;
    ast_cmd_t* cmd = NULL;

    if (!block)
        return 0;

    switch (block->tag) {
    case CMD_BLOCK:
        /* continue */
        break;
    case CMD_IF: {
        int left = compute_block(block->u.if_.command_if, location);
        int right = compute_block(block->u.if_.command_else, location);
        return left > right ? left : right;
    }
    case CMD_WHILE:
        return compute_block(block->u.while_.command, location);
    case CMD_ASSIGN:
    case CMD_DELETE:
    case CMD_RETURN:
    case CMD_CALL:
        return 0;
    }

    for (var = block->u.block_.variables; var != NULL; var = var->next) {
        var->u.variable_.offset = -location - 4;
        location += 4;
    }

    max_location = location;
    for (cmd = block->u.block_.commands; cmd != NULL; cmd = cmd->next) {
        int new_location = compute_block(cmd, location);
        if (new_location > max_location)
            max_location = new_location;
    }

    return max_location;
}

