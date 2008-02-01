#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "lucie.h"
#include "inireader.h"

int extension_count;
extension_t **extensions;

char errorstring[4096];

void* smalloc( size_t bytes )
{
    void* allocated;
    if ( ( allocated = malloc( bytes ) ) == NULL ) 
    {
        THROW_ERROR( "Failed to allocate %d bytes.", bytes );
        print_error( "Allocation failed" );
        exit( EXIT_FAILURE );
    }
    return allocated;
}

void* srealloc( void* old, size_t bytes )
{
    void* allocated;
    if ( ( allocated = realloc( old, bytes ) ) == NULL ) 
    {
        THROW_ERROR( "Failed to allocate %d bytes.", bytes );
        print_error( "Reallocation failed" );
        exit( EXIT_FAILURE );
    }
    return allocated;
}

int file_exists( const char* filename ) 
{
	struct stat stats;
	
	if ( stat( filename, &stats ) == -1 )
	{
		if ( errno == ENOENT )
		{
			return false;
		}
		else
		{
			// An error occured during the stats call
	        print_error("An error occured during a file_exists check");
			exit( EXIT_FAILURE );
		}
	}
	return true;
}

void cleanup_registered_extensions()
{
    int i;

    DEBUGLOG( "Freeing memory of %i extensions.", extension_count );
    for( i = 0; i < extension_count; i++ )
    {
        DEBUGLOG( "Freeing extension %i.", i );
        free( extensions[i] );
    }
    DEBUGLOG( "Freeing extension array." );
    free( extensions );
    DEBUGLOG( "All extension memory freed." );
}

int main( int argc, char** argv )
{    
    typedef void ( *test_t )( lua_State *L );
    test_t test;
    lua_State *L;
    void* dlh;
    inifile_t* inifile;

    // Init the lua engine
    DEBUGLOG( "Initializing lua state" );
    L = luaL_newstate();
    DEBUGLOG( "New state created L=0x%x", (int)L );
    luaL_openlibs( L ); 
    DEBUGLOG( "Lua lib opened" );

    // Test parse inifile
    DEBUGLOG( "Opening inifile" );
    inifile = inireader_open( "./test.ini" );
    DEBUGLOG( "Inifile opened" );
    // Just output the read information for testing purpose
    {
        inireader_iterator_t* iter;
        inireader_entry_t* current;
        DEBUGLOG( "Iterate all" );
        for( iter = inireader_get_iterator( inifile, 0, 0, 0, 0 ), current = inireader_iterate( iter ); current != NULL; current = inireader_iterate( iter ) ) 
        {
            printf( "Group: %s, Identifier: %s, Key: %s, Data: %s\n", current->group, current->identifier, current->key, current->data );
        }
        inireader_destroy_iterator( iter );
        DEBUGLOG( "Iterate group \"array_group\"" );
        for( iter = inireader_get_iterator( inifile, "array_group", 0, 0, 0 ), current = inireader_iterate( iter ); current != NULL; current = inireader_iterate( iter ) ) 
        {
            printf( "Group: %s, Identifier: %s, Key: %s, Data: %s\n", current->group, current->identifier, current->key, current->data );
        }
        inireader_destroy_iterator( iter );

    }
    DEBUGLOG( "Closing inifile" );
    inireader_close( inifile );
    DEBUGLOG( "Inifile closed" );

    // Open the core shared lib for testing
    DEBUGLOG( "Opening ext/core.so" );
    dlh = dlopen( "./src/ext/core.so", RTLD_LAZY );
    DEBUGLOG( "Extension opened handle=0x%x", (int)dlh );

    // Try to load the test function
    test = ( test_t )dlsym( dlh, "register_extension" );
    
    DEBUGLOG( "Executing test function" );
    test(L);

    DEBUGLOG( "Listing registered extensions:" );
    {
        int i;
        for( i=0; i<extension_count; i++ )
        {
            printf( "Ext name: \"%s\", Author: \"%s\", EMail: \"%s\"\n", extensions[i]->name, extensions[i]->author, extensions[i]->email );
        }
    }

    // Load the script for execution
    DEBUGLOG( "Trying to load lua file: %s", argv[1] );
    LUACHECK( luaL_loadfile( L, argv[1] ) );

    // Execute script
    DEBUGLOG( "Executing loaded script" );
    LUACHECK( lua_pcall( L, 0, LUA_MULTRET, 0 ) );

    // Cleanup memory from registered extensions
    cleanup_registered_extensions();

    // Destroy the lua environment
    DEBUGLOG( "Closing lua" );
    lua_close( L );

    return 0;
}
