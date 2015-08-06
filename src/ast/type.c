/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * type.c
 */

#include <stdio.h>
#include <string.h>

#include "type.h"

#include "util/new.h"

Type TypeCreate(TypeTag tag, int pointers)
{
    Type type;
    type.tag = tag;
    type.pointers = pointers;
    return type;
}

bool TypeEquals(Type a, Type b)
{
    return a.tag == b.tag && a.pointers == b.pointers;
}

char* TypeToString(Type type)
{
    size_t size = 6 + 2 * type.pointers;
    char* buffer = NEW_ARRAY(char, size);
    int len = 0;
    int i = 0;

    switch (type.tag) {
    case TYPE_VOID:  sprintf(buffer, "void"); break;
    case TYPE_BOOL:  sprintf(buffer, "bool"); break;
    case TYPE_CHAR:  sprintf(buffer, "char"); break;
    case TYPE_INT:   sprintf(buffer, "int"); break;
    case TYPE_FLOAT: sprintf(buffer, "float"); break;
    case TYPE_UNDEFINED: sprintf(buffer, "undefined"); break;
    }
    len = strlen(buffer);
    for (i = 0; i < type.pointers; i++) {
        sprintf(buffer + len, "[]");
        len += 2;
    }
    buffer[len] = '\0';
    return buffer;
}

void TypePrint(Type type)
{
    char* buffer = TypeToString(type);
    printf("%s", buffer);
    free(buffer);
}

bool TypeIsVoid(Type type)
{
    return type.tag == TYPE_VOID;
}

bool TypeIsBool(Type type)
{
    return type.tag == TYPE_BOOL && type.pointers == 0;
}

bool TypeIsChar(Type type)
{
    return type.tag == TYPE_CHAR && type.pointers == 0;
}

bool TypeIsInt(Type type)
{
    return type.tag == TYPE_INT && type.pointers == 0;
}

bool TypeIsFloat(Type type)
{
    return type.tag == TYPE_FLOAT && type.pointers == 0;
}

bool TypeIsNumerical(Type type)
{
    return TypeIsFloat(type) || TypeIsInt(type);
}

bool TypeIsString(Type type)
{
    return type.tag == TYPE_CHAR && type.pointers == 1;
}

bool TypeIsArray(Type type)
{
    return type.pointers > 0;
}

bool TypeIsAssignable(Type variable, Type expression)
{
    return TypeEquals(variable, expression) ||
           (TypeIsInt(variable) && TypeIsFloat(expression)) ||
           (TypeIsFloat(variable) && TypeIsInt(expression)) ||
           (TypeIsChar(variable) && TypeIsFloat(expression)) ||
           (TypeIsChar(variable) && TypeIsInt(expression));
}

