/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * table.h
 */

#ifndef TABLE_H
#define TABLE_H

/* Abstract data type for a table. */
typedef struct table* table_t;

/* Function pointers. */
typedef void(*table_destroy_f)(void*);
typedef void*(*table_copy_f)(void*);
typedef int(*table_less_f)(void*, void*);

/* Return type */
struct table_pair {
    void* key;
    void* data;
};

/* Creates a new table.
 * destroykey Deallocation function for keys.
 * destroydata Deallocation function for data.
 * copykey Copy function for keys.
 * copydata Copy function for data.
 * less Compare function for keys. */
table_t table_create(table_destroy_f destroykey, table_destroy_f destroydata,
        table_copy_f copykey, table_copy_f copydata, table_less_f less);

/* Destroys the table. */
void table_destroy(table_t table);

/* Inserts an element to the table.
 * If there is already an entrance to this key, the element is not inserted and
 * the function returns the existing one. */
struct table_pair table_insert(table_t table, void* key, void* data);

/* Erases an element from the table. */
void table_erase(table_t table, void* key);

/* Finds an element in the table.
 * If the element is not found, returns a pair with null key. */
struct table_pair table_find(table_t table, void* key);

/* Get the number of elements in the table */
int table_size(table_t table);

/* Creates an array with table contents */
struct table_pair* table_toarray(table_t table);

#endif

