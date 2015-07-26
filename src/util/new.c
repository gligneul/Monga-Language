/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * new.c
 */

#include "new.h"

#include "util/error.h"

#define ERROR error("unable to allocate memory")

void* new_alloc(size_t size)
{
    void* out = malloc(size);
    if (!out) ERROR;
    return out;
}

void* new_realloc(void* mem, size_t size)
{
    void* out = realloc(mem, size);
    if (!out) ERROR;
    return out;
}

