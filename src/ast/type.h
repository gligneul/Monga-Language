/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
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
    TYPE_UNDEFINED
} TypeTag;

/* Type declaration */
typedef struct {
    TypeTag tag;
    int pointers;
} Type;

/* Creates a type struct */
Type TypeCreate(TypeTag tag, int pointers);

/* Returns true if both types are equal */
bool TypeEquals(Type a, Type b);

/* Creates a string representation of the type
 * This string should be deallocated */
char* TypeToString(Type type);

/* Prints the type in stdout */
void TypePrint(Type type);

/* Type checking functions */
bool TypeIsVoid(Type type);
bool TypeIsBool(Type type);
bool TypeIsChar(Type type);
bool TypeIsInt(Type type);
bool TypeIsFloat(Type type);
bool TypeIsNumerical(Type type);
bool TypeIsString(Type type);
bool TypeIsArray(Type type);
bool TypeIsAssignable(Type variable, Type expression);

#endif

