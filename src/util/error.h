/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * error.h
 */

#ifndef ERROR_H
#define ERROR_H

/* Prints the message in stderr and exits the program
 * The message will be "mc: error, %formatedMessage\n" */
void Error(const char* formatedMessage, ...);

/* Prints the message in stderr and exits the program
 * The message will be "mc: error at line %line, %formatedMessage\n" */
void ErrorL(int line, const char* formatedMessage, ...);

#endif

