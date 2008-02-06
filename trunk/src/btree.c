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

btree_element_t* btree_find( btree_element_t* root, char* key ) 
{
    int cmp;
    
    // We reached the end of the tree without finding the element
    // Special case of empty root node is handled here as well 
    if ( root == NULL || root->key == NULL ) 
    {
        DEBUGLOG( "Element not found, returning NULL" );
        return NULL;
    }

    DEBUGLOG( "Comparing keys: %s, %s", root->key, key );
    cmp = strcmp( root->key, key );

    if( cmp == 0 ) 
    {
        // We found the searched element
        DEBUGLOG( "Found element, returning it" );
        return root;
    }

    // Recurse the tree to find the element
    if ( cmp < 0 ) 
    {
        DEBUGLOG( "Traversing left side" );
        return btree_find( root->left, key );
    }

    if ( cmp > 0 ) 
    {
        DEBUGLOG( "Traversing right side" );
        return btree_find( root->right, key );
    }
    
    DEBUGLOG( "We should never reach this statement" );
    return NULL;
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
