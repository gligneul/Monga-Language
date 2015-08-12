/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

int fiboC(int value)
{
    if (value < 2)
        return value;

    return fiboC(value - 1) + fiboC(value - 2);
}

