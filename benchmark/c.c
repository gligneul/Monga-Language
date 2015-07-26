/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 */

#include <stdlib.h>

float** multiplyMatricesC(float** a, float** b, int n) {
    int i, j, k;
    float value;
    float** out;

    out = (float**)malloc(sizeof(float*) * n);
    i = 0;
    while (i < n) {
        out[i] = (float*)malloc(sizeof(float) * n);
        j = 0;
        while (j < n) {
            value = 0;
            k = 0;
            while (k < n) {
                value = value + a[i][k] * b[k][j];
                k = k + 1;
            }
            out[i][j] = value;
            j = j + 1;
        }
        i = i + 1;
    }
    return out;
}

void bubbleSortC(int* array, int size) {
    while (size > 1) {
        int i;
        i = 0;
        while (i < size - 1) {
            if (array[i] > array[i + 1]) {
                int temp;
                temp = array[i];
                array[i] = array[i + 1];
                array[i + 1] = temp;
            }
            i = i + 1;
        }
        size = size - 1;
    }
}

