/*
 * PUC-Rio
 * INF1715 Compiladores
 * Gabriel de Quadros Ligneul 1212560
 *
 * table.c
 */

#include <stdlib.h>

#include "table.h"

struct node {
    void* key;
    void* data;
    struct node* father;
    struct node* left;
    struct node* right;
    int color;
};

struct table {
    table_destroy_f destroykey;
    table_destroy_f destroydata;
    table_copy_f copykey;
    table_copy_f copydata;
    table_less_f less;
    struct node* root;
    int size;
};

static const int BLACK = 0;
static const int RED = 1;

static struct node* node_create(table_t table, void* key, void* data,
        struct node* father, int color);
static void node_destroy(struct node* node, table_t table);
static void node_swap(struct node* a, struct node* b);
static struct node* node_grandfather(struct node* node);
static struct node* node_uncle(struct node* node);
static struct node* node_brother(struct node* node, struct node* father);
static void node_rotate_left(struct node* father, table_t table);
static void node_rotate_right(struct node* father, table_t table);
static struct node* node_insert(struct node* node, table_t table, void* key,
        void* data);
static void node_insert_rebalance(struct node* node, table_t table);
static void node_erase(struct node* node, table_t table);
static void node_erase_rebalance(struct node* node, struct node* father,
         table_t table);
static struct node* node_find(struct node* node, table_t table, void* key);
static void node_toarray(struct node* node, struct table_pair* pairs,
        int* index);

table_t table_create(table_destroy_f destroykey, table_destroy_f destroydata,
        table_copy_f copykey, table_copy_f copydata, table_less_f less)
{
    table_t table = (table_t)malloc(sizeof(struct table));
    table->destroykey = destroykey;
    table->destroydata = destroydata;
    table->copykey = copykey;
    table->copydata = copydata;
    table->less = less;
    table->root = NULL;
    table->size = 0;
    return table;
}

void table_destroy(table_t table)
{
    node_destroy(table->root, table);
    free(table);
}

struct table_pair table_insert(table_t table, void* key, void* data)
{
    struct node* node = NULL;
    struct table_pair pair = {NULL, NULL};
    if (!table->root) {
        node = table->root = node_create(table, key, data, NULL, BLACK);
        table->size += 1;
    } else {
        node = node_insert(table->root, table, key, data);
    }
    pair.key = node->key;
    pair.data = node->data;
    return pair;
}

void table_erase(table_t table, void* key)
{
    struct node* node = node_find(table->root, table, key);
    if (node) {
        node_erase(node, table);
        table->size--;
    }
}

struct table_pair table_find(table_t table, void* key)
{
    struct table_pair pair = {NULL, NULL};
    struct node* node = node_find(table->root, table, key);
    if (node) {
        pair.key = node->key;
        pair.data = node->data;
    }
    return pair;
}

int table_size(table_t table)
{
    return table->size;
}

struct table_pair* table_toarray(table_t table)
{
    int i = 0;
    if (!table->size) return 0;
    struct table_pair* pairs =
        (struct table_pair*)malloc(sizeof(struct table_pair) * table->size);
    node_toarray(table->root, pairs, &i);
    return pairs;
}

static struct node* node_create(table_t table, void* key, void* data,
        struct node* father, int color)
{
    struct node* node = (struct node*)malloc(sizeof(struct node));
    node->key = table->copykey(key);
    node->data = table->copydata(data);
    node->father = father;
    node->left = NULL;
    node->right = NULL;
    node->color = color;
    return node;
}

static void node_destroy(struct node* node, table_t table)
{
    if (node) {
        node_destroy(node->left, table);
        node_destroy(node->right, table);
        table->destroykey(node->key);
        table->destroydata(node->data);
        free(node);
    }
}

static void node_swap(struct node* a, struct node* b)
{
    void* akey = a->key;
    void* adata = a->data;
    a->key = b->key;
    a->data = b->data;
    b->key = akey;
    b->data = adata;
}

static struct node* node_grandfather(struct node* node)
{
    if (node && node->father)
        return node->father->father;
    return NULL;
}

static struct node* node_uncle(struct node* node)
{
    struct node* grandfather = node_grandfather(node);
    if (!grandfather)
        return NULL;
    if (node->father == grandfather->left)
        return grandfather->right;
    return grandfather->left;
}

static struct node* node_brother(struct node* node, struct node* father) 
{
    if (father->left == node)
        return father->right;
    return father->left;
}

static void node_rotate_left(struct node* father, table_t table) {
    struct node* grandfather = father->father;
    struct node* node = father->right;
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

static void node_rotate_right(struct node* father, table_t table)
{
    struct node* grandfather = father->father;
    struct node* node = father->left;
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

static struct node* node_insert(struct node* root, table_t table, void* key,
        void* data)
{
    struct node* father = root;
    struct node** insert_position = NULL;
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
    struct node* node = node_create(table, key, data, father, RED);
    *insert_position = node;
    table->size += 1;
    node_insert_rebalance(node, table);
    return node;
}

static void node_insert_rebalance(struct node* node, table_t table)
{
    struct node* uncle = NULL;
    struct node* grandfather = NULL;
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
    uncle = node_uncle(node);
    grandfather = node_grandfather(node);
    if (uncle && uncle->color == RED) {
        node->father->color = BLACK;
        uncle->color = BLACK;
        grandfather->color = RED;
        node = grandfather;
        goto begin;
    }

    /* Case 4 */
    if (node == node->father->right && node->father == grandfather->left) {
        node_rotate_left(node->father, table);
        node = node->left;
    } else if (node == node->father->left &&
            node->father == grandfather->right) {
        node_rotate_right(node->father, table);
        node = node->right;
    }

    /* Case 5 */
    grandfather = node_grandfather(node);
    node->father->color = BLACK;
    grandfather->color = RED;
    if (node == node->father->left)
        node_rotate_right(grandfather, table);
    else
        node_rotate_left(grandfather, table);
}

static void node_erase(struct node* node, table_t table)
{
    /* Swap the node with it predecessor */
    struct node* removed = node;
    struct node* child = node->right;
    if (node->left) {
        struct node* next = node->left;
        do {
            removed = next;
            child = next->left;
            next = removed->right;
        } while (next);
        node_swap(removed, node);
    }

    /* Swaps with child */
    struct node* father = removed->father;
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
            node_erase_rebalance(child, father, table);
    }
    table->destroykey(removed->key);
    table->destroydata(removed->data);
    free(removed);
}

static void node_erase_rebalance(struct node* node, struct node* father,
         table_t table)
{
    struct node* brother;
begin:
    /* Case 1 */
    if (father == NULL)
        return;

    /* Case 2 */
    brother = node_brother(node, father);
    if (brother->color == RED) {
        father->color = RED;
        brother->color = BLACK;
        if (node == father->left)
            node_rotate_left(father, table);
        else
            node_rotate_right(father, table);
        brother = node_brother(node, father);
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
        node_rotate_right(brother, table);
    } else if ((node == father->right) &&
               (!brother->left || brother->left->color == BLACK)) {
        brother->color = RED;
        if (brother->right)
            brother->right->color = BLACK;
        node_rotate_left(brother, table);
    }

    /* Case 6 */
    brother = node_brother(node, father);
    brother->color = father->color;
    father->color = BLACK;
    if (node == father->left) {
        if (brother->right)
            brother->right->color = BLACK;
        node_rotate_left(father, table);
    } else {
        if (brother->left)
            brother->left->color = BLACK;
        node_rotate_right(father, table);
    }
}

static struct node* node_find(struct node* node, table_t table, void* key)
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

static void node_toarray(struct node* node, struct table_pair* pairs,
        int* index)
{
    if (!node) return;
    node_toarray(node->left, pairs, index);
    pairs[*index].key = node->key;
    pairs[*index].data = node->data;
    (*index) += 1;
    node_toarray(node->right, pairs, index);
}

