/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * table.h
 */

#ifndef TABLE_H
#define TABLE_H

/* Abstract data type for a table. */
typedef struct Table* TableRef;

/* Function pointers. */
typedef void(*TableDestroyFunction)(void*);
typedef void*(*TableCopyFunction)(void*);
typedef int(*TableLessFunction)(void*, void*);

/* Return type */
typedef struct TablePair {
    void* key;
    void* data;
} TablePair;

/* Creates a new table.
 * destroykey Deallocation function for keys.
 * destroydata Deallocation function for data.
 * copykey Copy function for keys.
 * copydata Copy function for data.
 * less Compare function for keys. */
TableRef TableCreate(TableDestroyFunction destroykey,
        TableDestroyFunction destroydata, TableCopyFunction copykey,
        TableCopyFunction copydata, TableLessFunction less);

/* Destroys the table. */
void TableDestroy(TableRef table);

/* Inserts an element to the table.
 * If there is already an entrance to this key, the element is not inserted and
 * the function returns the existing one. */
TablePair TableInsert(TableRef table, void* key, void* data);

/* Erases an element from the table. */
void TableErase(TableRef table, void* key);

/* Finds an element in the table.
 * If the element is not found, returns a pair with null key. */
TablePair TableFind(TableRef table, void* key);

/* Get the number of elements in the table */
int TableSize(TableRef table);

/* Creates an array with table contents */
TablePair* TableToArray(TableRef table);

/* Dummy deallocation function, do nothing */
void TableDummyDestroy(void* p);

/* Dummy copy function, returns the passed argument */
void* TableDummyCopy(void* p);

/* Dummy comparison function, returns a < b */
int TableDummyLess(void* a, void* b);

/* String copy function */
void* TableStrCopy(void* p);

/* String coparison function */
int TableStrLess(void* a, void* b);

#endif

