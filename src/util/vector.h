/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * vector.h
 * Abstract data type that represents a dynamic array
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct Vector Vector;

/* Creates an empty vector */
Vector* VectorCreate();

/* Destroys the vector, doesn't free it's elements */
void VectorDestroy(Vector* vector);

/* Returns true if the size == 0 */
bool VectorEmpty(Vector* vector);

/* Returns the number of elements */
size_t VectorSize(Vector* vector);

/* Inserts an element in the last position */
void VectorPush(Vector* vector, void* value);

/* Removes the element at the last position and returns it */
void* VectorPop(Vector* vector);

/* Returns the elements at the last position */
void* VectorPeek(Vector* vector);

/* Obtains the element at the passed position */
void* VectorGet(Vector* vector, size_t pos);

/* Sets the element at the passed position */
void VectorSet(Vector* vector, size_t pos, void* value);

/* Change the vector size */
void VectorResize(Vector* vector, size_t size);

#endif

