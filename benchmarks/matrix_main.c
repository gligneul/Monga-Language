/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

size_t MATRIX_SIZE = 0;

/* External functions */
float** multiplyMatricesMonga(float** a, float** b, int n);
float** multiplyMatricesC(float** a, float** b, int n);

/* Auxiliary functions */
static float** createRandomMatrix();
static bool compareMatrices(float** a, float** b, size_t n);
static void destroyMatrix(float** m, size_t n);

int main(int argc, char* argv[])
{
    (void) argc;
    MATRIX_SIZE = (size_t)strtol(argv[1], NULL, 10);

    float** a = createRandomMatrix();
    float** b = createRandomMatrix();
    float** mongaOut = NULL;
    float** cOut = NULL;
    clock_t start, end;
    double monga_time, gcc_time;

    start = clock();
    mongaOut = multiplyMatricesMonga(a, b, MATRIX_SIZE);
    end = clock();
    monga_time = (end - start)/(double)CLOCKS_PER_SEC;

    start = clock();
    cOut = multiplyMatricesC(a, b, MATRIX_SIZE);
    end = clock();
    gcc_time = (end - start)/(double)CLOCKS_PER_SEC;

    if (!compareMatrices(mongaOut, cOut, MATRIX_SIZE)) {
        fprintf(stderr, "Output matrices are different\n");
        exit(1);
    }

    printf("Matrix multriplication float[%lu][%lu]\n", MATRIX_SIZE,
            MATRIX_SIZE);
    printf("monga: %f s\n", monga_time);
    printf("gcc:   %f s\n\n", gcc_time);

    destroyMatrix(a, MATRIX_SIZE);
    destroyMatrix(b, MATRIX_SIZE);
    destroyMatrix(mongaOut, MATRIX_SIZE);
    destroyMatrix(cOut, MATRIX_SIZE);

    return 0;
}

static float** createRandomMatrix()
{
    size_t i, j;
    float** m = (float**)malloc(sizeof(float*) * MATRIX_SIZE);
    for (i = 0; i < MATRIX_SIZE; i++) {
        m[i] = (float*)malloc(sizeof(float) * MATRIX_SIZE);
        for (j = 0; j < MATRIX_SIZE; j++)
            m[i][j] = (float)rand() / (float)RAND_MAX;
    }
    return m;
}

static bool compareMatrices(float** a, float** b, size_t n)
{
    size_t i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            if (a[i][j] != b[i][j])
                return false;
    return true;
}

static void destroyMatrix(float** m, size_t n)
{
    size_t i;
    for (i = 0; i < n; ++i)
        free(m[i]);
    free(m);
}

