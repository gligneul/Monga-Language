/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * vector.c
 */

#include "vector.h"

#include "util/new.h"

#define BUFFER_SIZE 16

struct vector_t {
    void** buffer;
    size_t size;
    size_t capacity;
};

static void change_capacity(vector_t* vector, size_t size);

vector_t* vector_create()
{
    vector_t* vector = NEW(vector_t);
    vector->buffer = NEW_ARRAY(void*, BUFFER_SIZE);
    vector->size = 0;
    vector->capacity = BUFFER_SIZE;
    return vector;
}

void vector_destroy(vector_t* vector)
{
    free(vector->buffer);
    free(vector);
}

bool vector_empty(vector_t* vector)
{
    return vector->size == 0;
}

size_t vector_size(vector_t* vector)
{
    return vector->size;
}

void vector_push(vector_t* vector, void* value)
{
    if (vector->size == vector->capacity)
        change_capacity(vector, vector->size * 2);
    vector->buffer[(vector->size)++] = value;
}

void* vector_pop(vector_t* vector)
{
    return vector->buffer[--(vector->size)];
}

void* vector_peek(vector_t* vector)
{
    return vector->buffer[vector->size - 1];
}

void* vector_get(vector_t* vector, size_t pos)
{
    return vector->buffer[pos];
}

void vector_set(vector_t* vector, size_t pos, void* value)
{
    vector->buffer[pos] = value;
}

void vector_resize(vector_t* vector, size_t size)
{
    if (size > vector->capacity)
        change_capacity(vector, size);
    vector->size = size;
}

static void change_capacity(vector_t* vector, size_t size)
{
    vector->buffer = new_realloc(vector->buffer, size * sizeof(void*));
    vector->capacity = size;
}

