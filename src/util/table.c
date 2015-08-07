/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * table.c
 */

#include <string.h>
#include <stdlib.h>

#include "table.h"

typedef struct Node {
    void* key;
    void* data;
    struct Node* father;
    struct Node* left;
    struct Node* right;
    int color;
} Node;

typedef struct Table {
    TableDestroyFunction destroykey;
    TableDestroyFunction destroydata;
    TableCopyFunction copykey;
    TableCopyFunction copydata;
    TableLessFunction less;
    Node* root;
    int size;
} Table;

static const int BLACK = 0;
static const int RED = 1;

static Node* nodeCreate(TableRef table, void* key, void* data,
        Node* father, int color);
static void nodeDestroy(Node* node, TableRef table);
static void nodeSwap(Node* a, Node* b);
static Node* nodeGrandfather(Node* node);
static Node* nodeUncle(Node* node);
static Node* nodeBrother(Node* node, Node* father);
static void nodeRotateLeft(Node* father, TableRef table);
static void nodeRotateRight(Node* father, TableRef table);
static Node* nodeInsert(Node* node, TableRef table, void* key,
        void* data);
static void nodeInsertRebalance(Node* node, TableRef table);
static void nodeErase(Node* node, TableRef table);
static void nodeEraseRebalance(Node* node, Node* father,
         TableRef table);
static Node* nodeFind(Node* node, TableRef table, void* key);
static void nodeToArray(Node* node, TablePair* pairs,
        int* index);

TableRef TableCreate(TableDestroyFunction destroykey,
        TableDestroyFunction destroydata, TableCopyFunction copykey,
        TableCopyFunction copydata, TableLessFunction less)
{
    TableRef table = (TableRef)malloc(sizeof(Table));
    table->destroykey = destroykey;
    table->destroydata = destroydata;
    table->copykey = copykey;
    table->copydata = copydata;
    table->less = less;
    table->root = NULL;
    table->size = 0;
    return table;
}

TableRef TableCreateDummy()
{
    return TableCreate(TableDummyDestroy, TableDummyDestroy, TableDummyCopy,
            TableDummyCopy, TableDummyLess);
}

void TableDestroy(TableRef table)
{
    nodeDestroy(table->root, table);
    free(table);
}

TablePair TableInsert(TableRef table, void* key, void* data)
{
    Node* node = NULL;
    TablePair pair = {NULL, NULL};
    if (!table->root) {
        node = table->root = nodeCreate(table, key, data, NULL, BLACK);
        table->size += 1;
    } else {
        node = nodeInsert(table->root, table, key, data);
    }
    pair.key = node->key;
    pair.data = node->data;
    return pair;
}

void TableErase(TableRef table, void* key)
{
    Node* node = nodeFind(table->root, table, key);
    if (node) {
        nodeErase(node, table);
        table->size--;
    }
}

TablePair TableFind(TableRef table, void* key)
{
    TablePair pair = {NULL, NULL};
    Node* node = nodeFind(table->root, table, key);
    if (node) {
        pair.key = node->key;
        pair.data = node->data;
    }
    return pair;
}

int TableSize(TableRef table)
{
    return table->size;
}

TablePair* TableToArray(TableRef table)
{
    int i = 0;
	TablePair* pairs;

    if (!table->size)
		return 0;
    pairs = (TablePair*)malloc(sizeof(TablePair) * table->size);
    nodeToArray(table->root, pairs, &i);
    return pairs;
}

TableRef TableClone(TableRef table)
{
    TablePair* pairs = TableToArray(table);
    TableRef clone = TableCreate(table->destroykey, table->destroydata,
            table->copykey, table->copydata, table->less);
    for (int i = 0; i < TableSize(table); ++i)
        TableInsert(clone, pairs[i].key, pairs[i].data);
    free(pairs);
    return clone;
}

TableRef TableMerge(TableRef tables[], int n)
{
    TableRef merge = TableCreate(tables[0]->destroykey, tables[0]->destroydata,
            tables[0]->copykey, tables[0]->copydata, tables[0]->less);
    for (int table_index = 0; table_index < n; ++table_index) {
        TableRef table = tables[table_index];
        TablePair* pairs = TableToArray(table);
        for (int i = 0; i < TableSize(table); ++i)
            TableInsert(merge, pairs[i].key, pairs[i].data);
        free(pairs);
    }
    return merge;
}

void TableDummyDestroy(void* p)
{
    (void)p;
}

void* TableDummyCopy(void* p)
{
    return p;
}

int TableDummyLess(void* a, void* b)
{
    return a < b;
}

void* TableStrCopy(void* p)
{
    size_t size = strlen((char*)p) + 1;
    char* out = (char*)malloc(size);
    strcpy(out, (char*)p);
    return out;
}

int TableStrLess(void* a, void* b)
{
    return strcmp((char*)a, (char*)b) < 0;
}

static Node* nodeCreate(TableRef table, void* key, void* data,
        Node* father, int color)
{
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = table->copykey(key);
    node->data = table->copydata(data);
    node->father = father;
    node->left = NULL;
    node->right = NULL;
    node->color = color;
    return node;
}

static void nodeDestroy(Node* node, TableRef table)
{
    if (node) {
        nodeDestroy(node->left, table);
        nodeDestroy(node->right, table);
        table->destroykey(node->key);
        table->destroydata(node->data);
        free(node);
    }
}

static void nodeSwap(Node* a, Node* b)
{
    void* akey = a->key;
    void* adata = a->data;
    a->key = b->key;
    a->data = b->data;
    b->key = akey;
    b->data = adata;
}

static Node* nodeGrandfather(Node* node)
{
    if (node && node->father)
        return node->father->father;
    return NULL;
}

static Node* nodeUncle(Node* node)
{
    Node* grandfather = nodeGrandfather(node);
    if (!grandfather)
        return NULL;
    if (node->father == grandfather->left)
        return grandfather->right;
    return grandfather->left;
}

static Node* nodeBrother(Node* node, Node* father) 
{
    if (father->left == node)
        return father->right;
    return father->left;
}

static void nodeRotateLeft(Node* father, TableRef table)
{
    Node* grandfather = father->father;
    Node* node = father->right;
    if (grandfather) {
        if (grandfather->left == father)
            grandfather->left = node;
        else
            grandfather->right = node;
        node->father = grandfather;
    } else {
        table->root = node;
        node->father = NULL;
    }
    father->right = node->left;
    if (father->right)
        father->right->father = father;
    node->left = father;
    father->father = node;
}

static void nodeRotateRight(Node* father, TableRef table)
{
    Node* grandfather = father->father;
    Node* node = father->left;
    if (grandfather) {
        if (grandfather->left == father)
            grandfather->left = node;
        else
            grandfather->right = node;
        node->father = grandfather;
    } else {
        table->root = node;
        node->father = NULL;
    }
    father->left = node->right;
    if (father->left)
        father->left->father = father;
    node->right = father;
    father->father = node;
}

static Node* nodeInsert(Node* root, TableRef table, void* key,
        void* data)
{
    Node* father = root;
    Node** insert_position = NULL;
	Node* node = NULL;

    while (!insert_position) {
        if (table->less(key, father->key)) {
            if (father->left) {
                father = father->left;
            } else {
                insert_position = &father->left;
            }
        } else if (table->less(father->key, key)) {
            if (father->right) {
                father = father->right;
            } else {
                insert_position = &father->right;
            }
        } else {
            return father;
        }
    }
    node = nodeCreate(table, key, data, father, RED);
    *insert_position = node;
    table->size += 1;
    nodeInsertRebalance(node, table);
    return node;
}

static void nodeInsertRebalance(Node* node, TableRef table)
{
    Node* uncle = NULL;
    Node* grandfather = NULL;
begin:
    /* Case 1 */
    if (!node->father) {
        node->color = BLACK;
        return;
    }

    /* Case 2 */
    if (node->father->color == BLACK)
        return;

    /* Case 3 */
    uncle = nodeUncle(node);
    grandfather = nodeGrandfather(node);
    if (uncle && uncle->color == RED) {
        node->father->color = BLACK;
        uncle->color = BLACK;
        grandfather->color = RED;
        node = grandfather;
        goto begin;
    }

    /* Case 4 */
    if (node == node->father->right && node->father == grandfather->left) {
        nodeRotateLeft(node->father, table);
        node = node->left;
    } else if (node == node->father->left &&
            node->father == grandfather->right) {
        nodeRotateRight(node->father, table);
        node = node->right;
    }

    /* Case 5 */
    grandfather = nodeGrandfather(node);
    node->father->color = BLACK;
    grandfather->color = RED;
    if (node == node->father->left)
        nodeRotateRight(grandfather, table);
    else
        nodeRotateLeft(grandfather, table);
}

static void nodeErase(Node* node, TableRef table)
{
    /* Swap the node with it predecessor */
    Node* removed = node;
    Node* child = node->right;
	Node* father = NULL;

    if (node->left) {
        Node* next = node->left;
        do {
            removed = next;
            child = next->left;
            next = removed->right;
        } while (next);
        nodeSwap(removed, node);
    }

    /* Swaps with child */
    father = removed->father;
    if (father) {
        if (father->left == removed)
            father->left = child;
        else
            father->right = child;
    } else {
        table->root = child;
    }
    if (child)
        child->father = father;

    /* Rebalance subtree */
    if (removed->color == BLACK) {
        if (child && child->color == RED)
            child->color = BLACK;
        else
            nodeEraseRebalance(child, father, table);
    }
    table->destroykey(removed->key);
    table->destroydata(removed->data);
    free(removed);
}

static void nodeEraseRebalance(Node* node, Node* father,
         TableRef table)
{
    Node* brother;
begin:
    /* Case 1 */
    if (father == NULL)
        return;

    /* Case 2 */
    brother = nodeBrother(node, father);
    if (brother->color == RED) {
        father->color = RED;
        brother->color = BLACK;
        if (node == father->left)
            nodeRotateLeft(father, table);
        else
            nodeRotateRight(father, table);
        brother = nodeBrother(node, father);
    }

    /* Case 3 */
    if ((father->color == BLACK) &&
        (brother->color == BLACK) &&
        (!brother->left || brother->left->color == BLACK) &&
        (!brother->right || brother->right->color == BLACK)) {
        brother->color = RED;
        node = father;
        father = node->father;
        goto begin;
    }

    /* Case 4 */
    if ((father->color == RED) &&
        (brother->color == BLACK) &&
        (!brother->left || brother->left->color == BLACK) &&
        (!brother->right || brother->right->color == BLACK)) {
        brother->color = RED;
        father->color = BLACK;
        return;
    }

    /* Case 5 */
    if ((node == father->left) &&
        (!brother->right || brother->right->color == BLACK)) {
        brother->color = RED;
        if (brother->left)
            brother->left->color = BLACK;
        nodeRotateRight(brother, table);
    } else if ((node == father->right) &&
               (!brother->left || brother->left->color == BLACK)) {
        brother->color = RED;
        if (brother->right)
            brother->right->color = BLACK;
        nodeRotateLeft(brother, table);
    }

    /* Case 6 */
    brother = nodeBrother(node, father);
    brother->color = father->color;
    father->color = BLACK;
    if (node == father->left) {
        if (brother->right)
            brother->right->color = BLACK;
        nodeRotateLeft(father, table);
    } else {
        if (brother->left)
            brother->left->color = BLACK;
        nodeRotateRight(father, table);
    }
}

static Node* nodeFind(Node* node, TableRef table, void* key)
{
    while (node) {
        if (table->less(key, node->key))
            node = node->left;
        else if (table->less(node->key, key))
            node = node->right;
        else
            break;
    }
    return node;
}

static void nodeToArray(Node* node, TablePair* pairs,
        int* index)
{
    if (!node) return;
    nodeToArray(node->left, pairs, index);
    pairs[*index].key = node->key;
    pairs[*index].data = node->data;
    (*index) += 1;
    nodeToArray(node->right, pairs, index);
}

