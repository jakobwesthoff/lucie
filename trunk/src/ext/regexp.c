#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <regex.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"

#include "regexp.h"

static const luaL_Reg regexp_metatable[] = {
    { "exec", regexp_exec },
    { "__gc", regexp_gc },
    { NULL, NULL }
};

void register_extension( lua_State *L ) 
{
    REGISTER_EXTENSION( "regexp", "Jakob Westhoff", "jakob@westhoffswelt.de" );
    NAMESPACE_BEGIN( "re" );
        REGISTER_NAMESPACE_FUNCTION( L_compile, compile );
    NAMESPACE_END( "re" );

    // We need to create our needed metatable for the regexp "object"
    luaL_newmetatable( L, REGEXP_REGEX_T );
    // Copy the metatable
    lua_pushvalue( L, -1 );
    // Set the metatable as index table. Using this trick we can define all the
    // needed functions for our userdata inside the metatable
    lua_setfield( L, -2, "__index" );
    // Register the needed functions
    luaL_register( L, NULL, regexp_metatable );
}

int L_compile( lua_State *L ) 
{
    regex_t* regexp = smalloc( sizeof( regex_t ) );
    const char* regexp_string = luaL_checkstring( L, 1 );
    char* pattern             = strdup( regexp_string );
    char  delimiter           = regexp_string[0];
    char* cur;
    int cflags = 0;
    int error = 0;

    // Apply all flags until the delimiter is found
    for( cur = ( pattern + strlen( pattern ) - 1 ); *cur != delimiter; cur-- )
    {
        switch( *cur ) 
        {
            case 'x':
                cflags |= REG_EXTENDED;
            break;
            case 'i':
                cflags |= REG_ICASE;
            break;
            case 'm':
                cflags |= REG_NEWLINE;
            break;
            default:
                luaL_error( L, "Invalid regexp modifier '%c'", *cur );
            break;
        }
    }

    // Set the last delimiter to a null-byte
    *cur = 0;
    // Move the start of the pattern to beyond the first delimiter
    pattern++;

    // Compile the regex and check for possible errors
    if ( ( error = regcomp( regexp, pattern, cflags ) ) != 0 ) 
    {
        int length   = regerror( error, regexp, NULL, 0 );
        char* buffer = smalloc ( length * sizeof( char ) );
        regerror ( error, regexp, buffer, length );
        lua_pushstring( L, buffer );
        free( buffer );
        free( pattern - 1 );
        free( regexp );
        lua_error( L );
    }
    free( pattern - 1 );
    
    // Create a new userdata object and bind the needed metatable to it
    {
        regex_t** udata = ( regex_t** )lua_newuserdata( L, sizeof( regex_t* ) );
        // Set the compiled regex address
        *udata = regexp;
        // Set the correct metatable
        luaL_getmetatable( L, REGEXP_REGEX_T );
        lua_setmetatable( L, -2 );
    }

    // Return the udata object
    return 1;
}

int regexp_exec( lua_State *L ) 
{
    regex_t* regexp = *( regex_t** )luaL_checkudata( L, 1, REGEXP_REGEX_T );
    const char* data = luaL_checkstring( L, 2 );

    if ( regexec( regexp, data, 0, NULL, 0 ) == 0 ) 
    {
        lua_pushboolean( L, true );
    }
    else 
    {
        lua_pushboolean( L, false );
    }
    return 1;
}

int regexp_gc( lua_State *L ) 
{
    regex_t* regexp = *( regex_t** )luaL_checkudata( L, 1, REGEXP_REGEX_T );

    regfree( regexp );
    free( regexp );

    return 0;
}
