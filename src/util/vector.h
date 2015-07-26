/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * vector.h
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stdlib.h>

/* Abstract data type for a vector */
typedef struct vector_t vector_t;

vector_t* vector_create();
void vector_destroy(vector_t* vector);

bool vector_empty(vector_t* vector);
size_t vector_size(vector_t* vector);

void vector_push(vector_t* vector, void* value);
void* vector_pop(vector_t* vector);
void* vector_peek(vector_t* vector);

void* vector_get(vector_t* vector, size_t pos);
void vector_set(vector_t* vector, size_t pos, void* value);
void vector_resize(vector_t* vector, size_t size);

#endif

