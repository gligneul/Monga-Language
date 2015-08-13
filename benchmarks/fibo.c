/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

#define CONCAT(x, y) x ## y
#define CONCAT2(x, y) CONCAT(x, y)

int CONCAT2(fibo, CC)(int value)
{
    if (value < 2)
        return value;

    return CONCAT2(fibo, CC)(value - 1) + CONCAT2(fibo, CC)(value - 2);
}

