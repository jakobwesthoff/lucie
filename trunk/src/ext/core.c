#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"

int Lfoobar( lua_State *L ) 
{
    printf( "core.foobar call\n" );
    return 0;
}

int Lfoobar2( lua_State *L ) 
{
    printf( "core.foobar2 call\n" );
    return 0;
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
