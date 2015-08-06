/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * vector.c
 */

#include "vector.h"

#include "util/new.h"

#define BUFFER_SIZE 16

struct Vector {
    void** buffer;
    size_t size;
    size_t capacity;
};

static void change_capacity(Vector* vector, size_t size);

Vector* VectorCreate()
{
    Vector* vector = NEW(Vector);
    vector->buffer = NEW_ARRAY(void*, BUFFER_SIZE);
    vector->size = 0;
    vector->capacity = BUFFER_SIZE;
    return vector;
}

void VectorDestroy(Vector* vector)
{
    free(vector->buffer);
    free(vector);
}

bool VectorEmpty(Vector* vector)
{
    return vector->size == 0;
}

size_t VectorSize(Vector* vector)
{
    return vector->size;
}

void VectorPush(Vector* vector, void* value)
{
    if (vector->size == vector->capacity)
        change_capacity(vector, vector->size * 2);
    vector->buffer[(vector->size)++] = value;
}

void* VectorPop(Vector* vector)
{
    return vector->buffer[--(vector->size)];
}

void* VectorPeek(Vector* vector)
{
    return vector->buffer[vector->size - 1];
}

void* VectorGet(Vector* vector, size_t pos)
{
    return vector->buffer[pos];
}

void VectorSet(Vector* vector, size_t pos, void* value)
{
    vector->buffer[pos] = value;
}

void VectorResize(Vector* vector, size_t size)
{
    if (size > vector->capacity)
        change_capacity(vector, size);
    vector->size = size;
}

static void change_capacity(Vector* vector, size_t size)
{
    vector->buffer = NewRealloc(vector->buffer, size * sizeof(void*));
    vector->capacity = size;
}

