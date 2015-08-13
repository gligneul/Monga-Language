/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions */
float** multiplyMatricesMonga(float** a, float** b, int n);
float** multiplyMatricesGcc(float** a, float** b, int n);
float** multiplyMatricesClang(float** a, float** b, int n);

/* Create a random matrix of size n */
static float** createRandomMatrix(size_t n);

/* Auxiliar benchmark function */
static void benchmark(float**(*function)(float**, float**, int), 
        float** a, float** b, int n, const char* cc);

int main(int argc, char* argv[])
{
    if (argc < 2) exit(1);

    size_t n = (size_t)strtol(argv[1], NULL, 10);
    srand(time(NULL));
    float** a = createRandomMatrix(n);
    float** b = createRandomMatrix(n);

    printf("Matrix multriplication float[%lu][%lu]\n", n, n);
    benchmark(multiplyMatricesMonga, a, b, n, "monga");
    benchmark(multiplyMatricesGcc, a, b, n, "gcc");
    benchmark(multiplyMatricesClang, a, b, n, "clang");

    return 0;
}

static float** createRandomMatrix(size_t n)
{
    float** matrix = (float**)malloc(sizeof(float*) * n);
    for (size_t i = 0; i < n; i++) {
        matrix[i] = (float*)malloc(sizeof(float) * n);
        for (size_t j = 0; j < n; j++)
            matrix[i][j] = (float)rand() / (float)RAND_MAX;
    }
    return matrix;
}

static void benchmark(float**(*function)(float**, float**, int), 
        float** a, float** b, int n, const char* cc)
{
    clock_t start = clock();
    function(a, b, n);
    clock_t end = clock();
    printf("%s:\t%f s\n", cc, (end - start) / (double)CLOCKS_PER_SEC);
}

