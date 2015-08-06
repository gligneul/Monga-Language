/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * new.h
 */

#ifndef NEW_H
#define NEW_H

#include <stdlib.h>

#define NEW(type) (type*)NewMalloc(sizeof(type))
#define NEW_ARRAY(type, size) (type*)NewMalloc(sizeof(type)*size)

/* Calls malloc and verifies if the memory was allocated */
void* NewMalloc(size_t size);

/* Calls realloc and verifies if the memory was allocated */
void* NewRealloc(void* mem, size_t size);

#endif

