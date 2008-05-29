#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"
#include "string.h"

int L_split( lua_State *L ) 
{
    const char* delimiter    = luaL_checkstring( L, 1 );
    const char* input_string = luaL_checkstring( L, 2 );
    int tableIndex = 1;

    // Check for non empty delimiter
    if ( strlen( delimiter ) != 1 ) 
    {
        lua_pushnil( L );
        lua_pushstring( L,"The split delimiter must not be empty or bigger then one character" );

        return 2;
    }

    // Create our output table
    lua_newtable( L );

    // Output the original string, if there is no delimiter match
    if ( strlen( input_string ) == 0 || strchr( input_string, *delimiter ) == NULL  ) 
    {
        lua_pushinteger( L, 1 );
        lua_pushstring( L, input_string );
        lua_settable( L, -3 );
        return 1;
    }

    // Scan the string and split at every delimiter
    // @todo: Add support for multichar delimiter
    {        
        char* start;
        char* string_to_free = strdup( input_string );
        char* cur = string_to_free;

        // Split the string starting at the end
        for( start = cur; *cur != 0; cur++ ) 
        {
            if ( *cur == *delimiter ) 
            {
                DEBUGLOG( "Found delimiter at %i", cur - start );
                lua_pushinteger( L, tableIndex++ );
                lua_pushlstring( L, start, cur - start );
                lua_settable( L, -3 );
                start = cur + 1;
            }
        }
        // Add the last part
        lua_pushinteger( L, tableIndex++ );
        lua_pushlstring( L, start, cur - start );
        lua_settable( L, -3 );

        // Free the copied string
        free( string_to_free );
    }
    return 1;
}

void register_extension( lua_State *L ) 
{
    REGISTER_EXTENSION( "string", "Jakob Westhoff", "jakob@westhoffswelt.de" );
    NAMESPACE_BEGIN( "string" );
        REGISTER_NAMESPACE_FUNCTION( L_split, split );
        REGISTER_NAMESPACE_FUNCTION( L_split, explode );
    NAMESPACE_END( "string" );
}
