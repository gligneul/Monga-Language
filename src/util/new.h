/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * new.h
 */

#ifndef NEW_H
#define NEW_H

#include <stdlib.h>

#define NEW(type) (type*)new_alloc(sizeof(type))
#define NEW_ARRAY(type, size) (type*)new_alloc(sizeof(type)*size)

/* Calls malloc and verifies if the memory was allocated */
void* new_alloc(size_t size);

/* Calls realloc and verifies if the memory was allocated */
void* new_realloc(void* mem, size_t size);

#endif

