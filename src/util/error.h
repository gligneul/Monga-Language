/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * error.h
 */

#ifndef ERROR_H
#define ERROR_H

/* Prints the message in stderr and exits the program
 * The message will be "mc: error, %formatedMessage\n" */
void error(const char* formatedMessage, ...);

/* Prints the message in stderr and exits the program
 * The message will be "mc: error at line %line, %formatedMessage\n" */
void errorl(int line, const char* formatedMessage, ...);

#endif

