/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 */

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
