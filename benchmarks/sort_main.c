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
void bubbleSortGcc(int* array, int n);
void bubbleSortClang(int* array, int n);

/* Creates an array of random elements with size n */
static int* createArray(size_t n);

/* Creates a copy of the array */
static int* duplicateArray(int* array, size_t n);

/* Auxiliar benchmark function */
static void benchmark(void(*function)(int*, int), int* array, size_t n,
        const char* cc);

int main(int argc, char* argv[])
{
    if (argc < 2) exit(1);

    size_t n = (size_t)strtol(argv[1], NULL, 10);
    srand(time(NULL));
    int* mongaArray = createArray(n);
    int* gccArray = duplicateArray(mongaArray, n);
    int* clangArray = duplicateArray(mongaArray, n);

    printf("BubbleSort int[%lu]\n", n);
    benchmark(bubbleSortMonga, mongaArray, n, "monga");
    benchmark(bubbleSortGcc, gccArray, n, "gcc");
    benchmark(bubbleSortClang, clangArray, n, "clang");

    return 0;
}

static int* createArray(size_t n)
{
    int* array = (int*)malloc(sizeof(int) * n);
    for (size_t i = 0; i < n; ++i)
        array[i] = rand() % 10000;
    return array;
}

static int* duplicateArray(int* array, size_t n)
{
    int* array_copy = (int*)malloc(sizeof(int) * n);
    size_t i;
    for (i = 0; i < n; ++i)
        array_copy[i] = array[i];
    return array_copy;
}

static void benchmark(void(*function)(int*, int), int* array, size_t n,
        const char* cc)
{
    clock_t start = clock();
    function(array, n);
    clock_t end = clock();
    printf("%s:\t%f s\n", cc, (end - start) / (double)CLOCKS_PER_SEC);
}

