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
float** multiplyMatricesMonga(float** a, float** b, int n);
float** multiplyMatricesGcc(float** a, float** b, int n);
float** multiplyMatricesClang(float** a, float** b, int n);
float** multiplyMatricesClangLlc(float** a, float** b, int n);

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
    benchmark(multiplyMatricesClangLlc, a, b, n, "clang -O0 + llc");

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
    double total = 0;

    for (int i = 0; i < N_TESTS; i++) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        function(a, b, n);
        gettimeofday(&end, NULL);
        total += (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)
                * (double)1.0e-6;
    }

    printf("%-16s%f s\n", cc, total / N_TESTS);
}

