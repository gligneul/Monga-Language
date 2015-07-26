/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 */

#include <stdio.h>

void printInt(int value)
{
    printf("%d\n", value);
}

void printFloat(float value)
{
    printf("%f\n", value);
}

void printString(char* string)
{
    printf("%s\n", string);
}

void printChar(char value)
{
    printf("%c\n", value);
}

void printBool(int value)
{
    printf("%s\n", value ? "true" : "false");
}

void printArray(int* array, int n)
{
    int i;
    for (i = 0; i < n; ++i)
        printf("%d ", array[i]);
    printf("\n");
}

void printMatrix(float** matrix, int n, int m)
{
    int i, j;
    for (i = 0; i < n; ++i) {
        for (j = 0; j < m; ++j) {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}
