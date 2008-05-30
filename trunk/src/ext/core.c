#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"
#include "../inireader.h"
#include "../btree.h"
#include "../output.h"

#include "core.h"

extern int urldecode( char* data ); 

void print_padded_line( int padding, const char* fmt, ... ) 
{
    const char* spacer = "  ";
    va_list args;

    header_output();
    
    va_start( args, fmt );

    for( ; padding > 0; padding-- ) 
    {
        printf( spacer );
    }
    vprintf( fmt, args );
    printf( "\n" );

    va_end( args );
}

void var_dump( lua_State* L, int depth ) 
{
    // All the different datatypes need to be handled differently
    if ( lua_isnil( L, -1 ) ) 
    {
        print_padded_line( depth, "NIL" );
    }
    else if ( lua_isfunction( L, -1 ) ) 
    {
        print_padded_line( depth, "FUNCTION" );
    }
    else if ( lua_isuserdata( L, -1 ) ) 
    {
        print_padded_line( depth, "USERDATA" );
    }
    else if ( lua_isthread( L, -1 ) ) 
    {
        print_padded_line( depth, "THREAD" );
    }
    else if ( lua_isboolean( L, -1 ) ) 
    {        
        print_padded_line( depth, "boolean(%s)", lua_toboolean( L, -1 ) == true ? "true" : "false" );
    }
    else if ( lua_isnumber( L, -1 ) )
    {
        double number = lua_tonumber( L, -1 );

        if ( (double)(int)number == number ) 
        {
            print_padded_line( depth, "integer(%i)", (int)number );
        }
        else
        {
            print_padded_line( depth, "float(%f)", number );
        }
    }
    else if( lua_isstring( L, -1 ) ) 
    {
        print_padded_line( depth, "string(%d) \"%s\"", lua_strlen( L, -1 ), lua_tostring( L, -1 ) );
    }
    else if( lua_istable( L, -1 ) ) 
    {
        print_padded_line( depth, "table(%i) {", lua_objlen( L, -1 ) );

        // Push nil as first key before calling next
        lua_pushnil( L );
        while ( lua_next(L, -2 ) != 0 ) {            
            if ( lua_isnumber( L, -2 ) ) 
            {
                print_padded_line( depth + 1, "[%i] =>", lua_tointeger( L, -2 ) );
            }
            else
            {
                print_padded_line( depth + 1, "[\"%s\"] =>", lua_tostring( L, -2 ) );                
            }
            var_dump( L, depth + 1 );
        }
        print_padded_line( depth, "}" );
    }

    lua_pop( L, 1 );
}

int L_var_dump( lua_State* L ) 
{
    int i;
    int elements = lua_gettop( L );
    for( i = 1; i <= elements ; i++ ) 
    {
        // We need to push the last value first, because the parameters are in
        // inverted order on the stack
        lua_pushvalue( L, i );
        var_dump( L, 0 );
    }
    return 0;
}

int L_urldecode( lua_State* L ) 
{
    const char* param;
    char* data;
    PARAM_STRING( param );
    data = strdup( param );
    urldecode( data );
    RETURN_STRING( data );
    free( data );
    return 1;
}

void readini_array_traverse( lua_State* L, btree_element_t* root, int arrayindex ) 
{
    if ( root->next != NULL ) 
    {
        readini_array_traverse( L, root->next, ( ((inireader_entry_t*)root->next->data)->key != NULL ) ? ( arrayindex ) : ( arrayindex + 1 ) );
    }

    if ( ((inireader_entry_t*)root->data)->key != NULL ) 
    {
        DEBUGLOG( "Pushing user defined sub index: %s", ((inireader_entry_t*)root->data)->key );
        lua_pushstring( L, ((inireader_entry_t*)root->data)->key );
    }
    else 
    {
        DEBUGLOG( "Pushing default sub index: %i", arrayindex );
        lua_pushnumber( L, arrayindex );
    }

    // Push data value
    lua_pushstring( L, ((inireader_entry_t*)root->data)->data );

    DEBUGLOG( "Setting sub entry" );
    lua_settable( L, -3 );
}

void readini_group_tree_traverse( lua_State* L, btree_element_t* root ) 
{
    if ( root->left != NULL ) 
    {
        readini_group_tree_traverse( L, root->left );
    }
    if ( root->right != NULL ) 
    {
        readini_group_tree_traverse( L, root->right );
    }

    // Add the current element
    // Push the index to the stack
    DEBUGLOG( "Pushing main index: %s", ((inireader_entry_t* )root->data)->identifier );
    lua_pushstring( L, ((inireader_entry_t* )root->data)->identifier );

    // Push the data to the stack
    if ( root->next != NULL || ((inireader_entry_t* )root->data)->key != NULL ) 
    {
        // The data is a table
        DEBUGLOG( "Creating sub table" );
        lua_newtable( L );
        DEBUGLOG( "Traversing array list" );
        readini_array_traverse( L, root, 1 );
    }
    else
    {
        // Simply push the data
        DEBUGLOG( "Pusing simple value: %s", ((inireader_entry_t*)root->data)->data );
        lua_pushstring( L, ((inireader_entry_t*)root->data)->data );
    }

    // Add entry to table
    DEBUGLOG( "Adding main entry" );
    lua_settable( L, -3 );
}

void readini_tree_traverse( lua_State* L, btree_element_t* root )
{
    if ( root->left != NULL ) 
    {
        readini_tree_traverse( L, root->left );
    }
    if ( root->right != NULL ) 
    {
        readini_tree_traverse( L, root->right );
    }

    // Add a new array index for the current group
    // Push the index to the stack
    DEBUGLOG( "Pushing group index: %s", root->key );
    lua_pushstring( L, root->key );

    // Push the data to the stack
    DEBUGLOG( "Creating group table" );
    lua_newtable( L );
    DEBUGLOG( "Traversing group tree" );
    readini_group_tree_traverse( L, ( btree_element_t* )root->data );

    // Add entry to table
    DEBUGLOG( "Adding group entry" );
    lua_settable( L, -3 );
}


void readini_tree_free( btree_element_t* root ) 
{
    // Traverse every node and free the subtree
    if ( root->left != NULL ) 
    {
        readini_tree_free( root->left );
    }
    if( root->right != NULL ) 
    {
        readini_tree_free( root->right );
    }

    // Free the subtree
    btree_free( ( btree_element_t* )root->data );
}

int L_readini( lua_State* L ) 
{
    const char* filename;
    inifile_t* inifile            = NULL;
    inireader_iterator_t* iter    = NULL;
    inireader_entry_t* current    = NULL;
    btree_element_t* root         = NULL;
    btree_element_t* groupelement = NULL;
    btree_element_t* grouproot    = NULL;

    PARAM_STRING( filename );

    if ( ( inifile = inireader_open( filename ) ) == NULL ) 
    {
        return_error();
    }

    // Add every entry into a btree to get the arrays and groups in one piece later
    DEBUGLOG( "Creating btree" );
    root = btree_create();
    iter = inireader_get_iterator( inifile, 0, 0, 0, 0 );
    DEBUGLOG( "Filling up btree" );
    for ( current = inireader_iterate( iter ); current != NULL; current = inireader_iterate( iter ) )         
    {
        DEBUGLOG( "Searching for or adding group: %s", current->group );
        // Find group element
        groupelement = btree_find( root, current->group );        
        if ( groupelement == NULL ) 
        {
            // A new grouproot needs to be created
            DEBUGLOG( "Creating new grouproot" );
            grouproot = btree_create();
            btree_add( &root, current->group, grouproot );
        }
        else 
        {
            // Retrieve the already added grouproot
            DEBUGLOG( "Setting grouproot" );
            grouproot = ( btree_element_t* )groupelement->data;
        }

        // Add the new element to the grouptree
        btree_add( &grouproot, current->identifier, current );
    }
    
    // Traverse the tree and put our inivalues onto the lua stack
    DEBUGLOG( "Creating initial lua table" );
    lua_newtable( L );    
    readini_tree_traverse( L, root );
    DEBUGLOG( "Freeing the btree data" );
    // Free the group trees
    readini_tree_free( root );
    // Free the main tree
    btree_free( root );
    // Close the inifile
    inireader_close( inifile );
    
    return 1;
}

int L_eval( lua_State *L ) 
{
    const char* command = luaL_checkstring( L, 1 );
    int stackTop = lua_gettop( L );
    int nresult  = 0;
    luaL_loadstring( L, command );
    lua_call( L, 0, LUA_MULTRET );
    nresult = lua_gettop( L ) - stackTop;
    return nresult;
}

void register_extension( lua_State *L ) 
{
    REGISTER_EXTENSION( "core", "Jakob Westhoff", "jakob@westhoffswelt.de" );
    REGISTER_GLOBAL_FUNCTION( L_var_dump, var_dump );
    REGISTER_GLOBAL_FUNCTION( L_urldecode, urldecode );
    REGISTER_GLOBAL_FUNCTION( L_readini, readini );
    REGISTER_GLOBAL_FUNCTION( L_eval, eval );
}
