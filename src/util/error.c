/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * error.c
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

void error(const char* formatedMessage, ...)
{
    va_list args;
    va_start(args, formatedMessage);
    fprintf(stderr, "mc: error, ");
    vfprintf(stderr, formatedMessage, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

void errorl(int line, const char* formatedMessage, ...)
{
    va_list args;
    va_start(args, formatedMessage);
    fprintf(stderr, "mc: error at line %d, ", line);
    vfprintf(stderr, formatedMessage, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

