/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions */
int fiboMonga(int value);
int fiboGcc(int value);
int fiboClang(int value);

/* Auxiliar benchmark function */
static void benchmark(int(*function)(int), int value, const char* cc);

int main(int argc, char* argv[])
{
    if (argc < 2) exit(1);

    int n = strtol(argv[1], NULL, 10);

    printf("Recursive Fibonacci(%d)\n", n);
    benchmark(fiboMonga, n, "monga");
    benchmark(fiboGcc, n, "gcc");
    benchmark(fiboClang, n, "clang");

    return 0;
}

static void benchmark(int(*function)(int), int value, const char* cc)
{
    clock_t start = clock();
    function(value);
    clock_t end = clock();
    printf("%s:\t%f s\n", cc, (end - start)/(double)CLOCKS_PER_SEC);
}

