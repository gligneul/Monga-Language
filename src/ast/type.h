/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * type.h
 * Struct responsable for the type representation.
 */

#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>

/* Type tags */
typedef enum {
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_NULL,
    TYPE_UNDEFINED
} type_tag;

/* Type declaration */
typedef struct {
    type_tag tag;
    int pointers;
} type_t;

/* Creates a type struct */
type_t type_create(type_tag tag, int pointers);

/* Returns true if both types are equals */
bool type_cmp(type_t a, type_t b);

/* Creates a string representation of the type
 * This string should be deallocated */
char* type_to_str(type_t type);

/* Prints the type in stdout */
void type_print(type_t type);

#endif

