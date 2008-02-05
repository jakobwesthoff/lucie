#ifndef BTREE_H
#define BTREE_H

typedef struct btree_element_struct
{
    char* key;
    void* data;
    struct btree_element_struct* left;
    struct btree_element_struct* right;
    struct btree_element_struct* next;
} btree_element_t;

btree_element_t* btree_create();
void btree_add( btree_element_t** root, char* key, void* data );
void btree_free( btree_element_t* root );

#endif
