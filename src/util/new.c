/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * new.c
 */

#include "new.h"

#include "util/error.h"

#define ERROR Error("unable to allocate memory")

void* NewMalloc(size_t size)
{
    void* out = malloc(size);
    if (!out) ERROR;
    return out;
}

void* NewRealloc(void* mem, size_t size)
{
    void* out = realloc(mem, size);
    if (!out) ERROR;
    return out;
}

