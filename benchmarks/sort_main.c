/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

const int N_TESTS = 10;

/* External functions */
void bubbleSortMonga(int* array, int n);
void bubbleSortGcc(int* array, int n);
void bubbleSortClang(int* array, int n);
void bubbleSortClangLlc(int* array, int n);

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
    int* array = createArray(n);

    printf("BubbleSort int[%lu]\n", n);
    benchmark(bubbleSortMonga, array, n, "monga");
    benchmark(bubbleSortGcc, array, n, "gcc");
    benchmark(bubbleSortClang, array, n, "clang");
    benchmark(bubbleSortClangLlc, array, n, "clang -O0 + llc");

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
    double total = 0;

    for (int i = 0; i < N_TESTS; ++i) {
        struct timeval start, end;
        int* copy = duplicateArray(array, n);
        gettimeofday(&start, NULL);
        function(copy, n);
        gettimeofday(&end, NULL);
        free(copy);
        total += (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)
                * (double)1.0e-6;
    }

    printf("%-16s%f s\n", cc, total / N_TESTS);
}

