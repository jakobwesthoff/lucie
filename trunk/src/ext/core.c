#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"

int Lfoobar( lua_State *L ) 
{
    int bool;
    const char *ch;
    int len = 0;
    double number;
    PARAM_DOUBLE( number );
    PARAM_BINARY_STRING( ch, len );
    PARAM_BOOLEAN( bool );

    printf( "core.foobar( %s, \"%s\" (%i), %f ) call\n", ( bool == 0 )?"false":"true", ch, len, number );
    return 0;
}

int Lfoobar2( lua_State *L ) 
{
    const char* retval[] = { "foobar1", "foobar2", "foobar3" };
    printf( "core.foobar2 call\n" );
    RETURN_ARRAY( STRING, retval, 3 );
    return 1;
}

int Lglobal_foobar( lua_State *L ) 
{
    printf( "global foobar call\n" );
    return 0;
}

void register_extension( lua_State *L ) 
{
    REGISTER_EXTENSION( "core", "Jakob Westhoff", "jakob@westhoffswelt.de" );
    NAMESPACE_BEGIN( "core" );
        REGISTER_NAMESPACE_FUNCTION( Lfoobar, foobar );
        REGISTER_NAMESPACE_FUNCTION( Lfoobar2, foobar2 );
    NAMESPACE_END();
    REGISTER_GLOBAL_FUNCTION( Lglobal_foobar, foobar );
}
