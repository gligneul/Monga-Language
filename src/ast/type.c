/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * type.c
 */

#include <stdio.h>
#include <string.h>

#include "type.h"

#include "util/new.h"

type_t type_create(type_tag tag, int pointers)
{
    type_t type;
    type.tag = tag;
    type.pointers = pointers;
    return type;
}

bool type_cmp(type_t a, type_t b)
{
    return a.tag == b.tag && a.pointers == b.pointers;
}

char* type_to_str(type_t type)
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
    case TYPE_NULL:  sprintf(buffer, "null"); break;
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

void type_print(type_t type)
{
    char* buffer = type_to_str(type);
    printf("%s", buffer);
    free(buffer);
}

