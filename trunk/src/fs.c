#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lucie.h"
#include "fs.h"

int L_realpath( lua_State *L ) 
{
    char rpath[PATH_MAX];
    const char* pathname = luaL_checkstring( L, 1 );
    if ( realpath( pathname, rpath ) == NULL ) 
    {
        luaL_error( L, "Could not retrieve realpath: %s", strerror( errno ) );
    }
    lua_pushstring( L, rpath );
    return 1;
}

int L_dirname( lua_State *L ) 
{
    const char* param = luaL_checkstring( L, 1 );
    char* filename = strdup( param );
    char* iter = filename;
    char* tmp = NULL;

    for( tmp = iter; *iter; iter++ ) 
    {
        if ( *iter == '/' ) 
        {
            tmp = iter;
        }
    }
    *tmp = 0;
    RETURN_STRING( filename );
    free( filename );
    return 1;
}

