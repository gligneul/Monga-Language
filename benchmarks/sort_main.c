/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

size_t ARRAY_SIZE = 0;

/* External functions */
void bubbleSortMonga(int* array, int n);
void bubbleSortC(int* array, int n);

/* Auxiliary functions */
static int* createArray();
static int* duplicateArray(int* array, size_t n);
static bool compareArray(int* a, int* b, size_t n);

int main(int argc, char* argv[])
{
    (void) argc;
    ARRAY_SIZE = (size_t)strtol(argv[1], NULL, 10);

    int* mongaArray = createArray();
    int* cArray = duplicateArray(mongaArray, ARRAY_SIZE);
    clock_t start, end;
    double monga_time, gcc_time;

    if (!compareArray(mongaArray, cArray, ARRAY_SIZE)) {
        fprintf(stderr, "Arrays are different before sort\n");
        exit(1);
    }

    start = clock();
    bubbleSortMonga(mongaArray, ARRAY_SIZE);
    end = clock();
    monga_time = (end - start)/(double)CLOCKS_PER_SEC;

    start = clock();
    bubbleSortC(cArray, ARRAY_SIZE);
    end = clock();
    gcc_time = (end - start)/(double)CLOCKS_PER_SEC;

    if (!compareArray(mongaArray, cArray, ARRAY_SIZE)) {
        fprintf(stderr, "Arrays are different after sort\n");
        exit(1);
    }

    printf("BubbleSort int[%lu]\n", ARRAY_SIZE);
    printf("monga: %f s\n", monga_time);
    printf("gcc:   %f s\n\n", gcc_time);

    free(mongaArray);
    free(cArray);

    return 0;
}

static int* createArray()
{
    int* out = (int*)malloc(sizeof(int) * ARRAY_SIZE);
    size_t i;
    for (i = 0; i < ARRAY_SIZE; ++i)
        out[i] = rand() % 10000;
    return out;
}

static int* duplicateArray(int* array, size_t n)
{
    int* out = (int*)malloc(sizeof(int) * n);
    size_t i;
    for (i = 0; i < n; ++i)
        out[i] = array[i];
    return out;
}

static bool compareArray(int* a, int* b, size_t n)
{
    size_t i;
    for (i = 0; i < n; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}

