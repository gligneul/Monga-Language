/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

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
    struct timeval start, end;
    gettimeofday(&start, NULL);
    function(value);
    gettimeofday(&end, NULL);
    printf("%s:\t%f s\n", cc, (end.tv_sec - start.tv_sec) +
            (end.tv_usec - start.tv_usec) * (double)1.0e-6);
}

