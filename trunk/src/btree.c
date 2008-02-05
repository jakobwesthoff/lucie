#include <stdlib.h>
#include <stdio.h>

#include "lucie.h"
#include "btree.h"

btree_element_t* btree_create() 
{
    btree_element_t* head;
    head = smalloc( sizeof( btree_element_t ) );
    head->key = NULL;
    head->data = NULL;
    head->left = NULL;
    head->right = NULL;
    head->next = NULL;
    return head;
}

void btree_add( btree_element_t** root, char* key, void* data ) 
{
    // We found the insert position
    if ( *root == NULL ) 
    {
        // Allocate root element and init it
        *root = smalloc( sizeof( btree_element_t ) );
        ( *root )->left = ( *root )->right = ( *root )->next = NULL;
        ( *root )->key  = key;
        ( *root )->data = data;
        return;
    }

    // Handle the case that there are no elements in the tree
    if ( ( *root )->key == NULL  ) 
    {
        ( *root )->key  = key;
        ( *root )->data = data;
        return;
    }

    {
        int cmp = strcmp( ( *root )->key, key );
        if ( cmp < 0 ) 
        {
            btree_add( &( *root )->left, key, data );
        }
        else if ( cmp > 0 )
        {
            btree_add( &( *root )->right, key, data );
        }
        else 
        {
            btree_add( &( *root )->next, key, data );
        }
    }
}

void btree_free( btree_element_t* root ) 
{
    if ( root->left != NULL ) 
    {
        btree_free( root->left );
    }
    if ( root->next != NULL ) 
    {
        btree_free( root->next );
    }
    if ( root->right != NULL ) 
    {
        btree_free( root->right );
    }

    free( root );
}
