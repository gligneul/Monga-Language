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
int fiboC(int value);

int main(int argc, char* argv[])
{
    (void) argc;
    int n = strtol(argv[1], NULL, 10);

    clock_t start, end;
    double monga_time, gcc_time;

    start = clock();
    fiboMonga(n);
    end = clock();
    monga_time = (end - start)/(double)CLOCKS_PER_SEC;

    start = clock();
    fiboC(n);
    end = clock();
    gcc_time = (end - start)/(double)CLOCKS_PER_SEC;

    printf("Recursive Fibonacci(%d)\n", n);
    printf("monga: %f s\n", monga_time);
    printf("gcc:   %f s\n\n", gcc_time);

    return 0;
}

