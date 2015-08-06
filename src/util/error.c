/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * error.c
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

void Error(const char* formatedMessage, ...)
{
    va_list args;
    va_start(args, formatedMessage);
    fprintf(stderr, "monga: error, ");
    vfprintf(stderr, formatedMessage, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

void ErrorL(int line, const char* formatedMessage, ...)
{
    va_list args;
    va_start(args, formatedMessage);
    fprintf(stderr, "monga: error at line %d, ", line);
    vfprintf(stderr, formatedMessage, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

