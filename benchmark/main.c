/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 500
#define ARRAY_SIZE 10000

float** multiplyMatricesMonga(float** a, float** b, int n);
float** multiplyMatricesC(float** a, float** b, int n);
void bubbleSortMonga(int* array, int n);
void bubbleSortC(int* array, int n);

static void benchmarkMatrix();
static float** createRandomMatrix();
static bool compareMatrices(float** a, float** b, size_t n);
static bool destroyMatrix(float** m, size_t n);
static void benchmarkSort();
static int* createArray();
static int* duplicateArray(int* array, size_t n);
static bool compareArray(int* a, int* b, size_t n);

int main()
{
    benchmarkSort();
    benchmarkMatrix();
    return 0;
}

static void benchmarkMatrix()
{
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

    printf("Matrix multriplication float[%d][%d]\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("monga: %f s\n", monga_time);
    printf("gcc:   %f s\n\n", gcc_time);

    destroyMatrix(a, MATRIX_SIZE);
    destroyMatrix(b, MATRIX_SIZE);
    destroyMatrix(mongaOut, MATRIX_SIZE);
    destroyMatrix(cOut, MATRIX_SIZE);
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

static bool destroyMatrix(float** m, size_t n)
{
    size_t i;
    for (i = 0; i < n; ++i)
        free(m[i]);
    free(m);
}

static void benchmarkSort()
{
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

    printf("BubbleSort int[%d]\n", ARRAY_SIZE);
    printf("monga: %f s\n", monga_time);
    printf("gcc:   %f s\n\n", gcc_time);

    free(mongaArray);
    free(cArray);
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

