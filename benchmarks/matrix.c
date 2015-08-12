/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
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

