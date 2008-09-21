#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"
#include "string.h"

static int L_split( lua_State *L ) 
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
    // @todo: Maybe add multichar delimiter support in the future
    {        
        const char* start;
        const char* cur = input_string;

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
    }
    return 1;
}

static int L_join( lua_State *L ) 
{
    char* joinedString  = NULL;
    int joinedStringLen = 0;
    unsigned int delimiterLen = 0;

    const char* delimiter = luaL_checklstring( L, 1, &delimiterLen );
    luaL_checktype( L, 2, LUA_TTABLE );

    // Init the output string. It needs to be at least the \0 terminating character.
    joinedString = (char*)smalloc( sizeof(char) );
    *joinedString = 0;
    joinedStringLen++;

    // Get the table to the top
    lua_pushvalue( L, 2 );
    // First key
    lua_pushnil( L );
    while( lua_next( L, -2 ) != 0 ) 
    {
        unsigned int len = 0;
        const char* str = NULL;
        if( ( str = lua_tolstring( L, -1, &len ) ) == NULL ) 
        {            
            return luaL_error( L, "Found table entry which could not be casted to type string." );
        }
        DEBUGLOG( "New string found in table: string(%i) \"%s\"", len, str );
        // Allocate needed memory
        joinedString = (char*)srealloc( joinedString, ( joinedStringLen + len + delimiterLen ) * sizeof( char ) );
        // Initialize needed memory
        memset( joinedString + joinedStringLen, 0, len + delimiterLen );
        // Remember our new string length
        joinedStringLen += len + delimiterLen;
        // Concat the new string and delimiter
        strcat( joinedString, str );
        strcat( joinedString, delimiter );
        // Remove the retrieved value from the lua stack for the next call to lua_next
        lua_pop( L, 1 );
    }

    // Remove the table from the stack
    lua_pop( L, 1 );

    // Push the generated string to the stack cutting off the last delimiter
    lua_pushlstring( L, joinedString, joinedStringLen - delimiterLen - 1 );

    // Free the occupied memory
    free( joinedString );

    return 1;
}

static int L_tolower( lua_State *L ) 
{
    const char* input = luaL_checkstring( L, 1 );
    char* output = strdup( input );
    char* cur = output;

    while( *cur != 0 ) 
    {
        *cur = ( *cur >= 'A' && *cur <= 'Z' ) ? (*cur) + 32 : *cur;
        cur++;
    }
    lua_pushstring( L, output );
    free( output );
    return 1;
}

static int L_toupper( lua_State *L ) 
{
    const char* input = luaL_checkstring( L, 1 );
    char* output = strdup( input );
    char* cur = output;

    while( *cur != 0 ) 
    {
        *cur = ( *cur >= 'a' && *cur <= 'z' ) ? (*cur) - 32 : *cur;
        cur++;
    }
    lua_pushstring( L, output );
    free( output );
    return 1;
}

static int L_ucfirst( lua_State *L ) 
{
    unsigned int len;
    const char* input = luaL_checklstring( L, 1, &len );
    char* output = strdup( input );
    char* cur = output;

    if ( len == 0 ) 
    {
        lua_pushstring( L, cur );
        free( output );
        return 1;
    }

    *cur = ( *cur >= 'a' && *cur <= 'z' ) ? (*cur) - 32 : *cur;
    cur++;

    while( *cur != 0 ) 
    {
        *cur = ( *cur >= 'A' && *cur <= 'Z' ) ? (*cur) + 32 : *cur;
        cur++;
    }
    lua_pushstring( L,  output );
    free( output );
    return 1;
}

void register_extension( lua_State *L ) 
{
    REGISTER_EXTENSION( "string", "Jakob Westhoff", "jakob@westhoffswelt.de" );
    NAMESPACE_BEGIN( "string" );
        REGISTER_NAMESPACE_FUNCTION( L_split, split );
        REGISTER_NAMESPACE_FUNCTION( L_split, explode );
        REGISTER_NAMESPACE_FUNCTION( L_join, join );
        REGISTER_NAMESPACE_FUNCTION( L_join, implode );
        REGISTER_NAMESPACE_FUNCTION( L_tolower, tolower );
        REGISTER_NAMESPACE_FUNCTION( L_toupper, toupper );
        REGISTER_NAMESPACE_FUNCTION( L_ucfirst, ucfirst );
    NAMESPACE_END( "string" );
}
