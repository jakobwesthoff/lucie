#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
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

void print_padded_line( int padding, const char* fmt, ... ) 
{
    const char* spacer = "  ";
    va_list args;
    
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
    // Check for data on the stack
    if ( lua_isnone( L, -1 ) )
    {
        return;
    }

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
    
    lua_pop( L, 1 );
}

int L_var_dump( lua_State* L ) 
{
    var_dump( L, 0 );
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
    REGISTER_GLOBAL_FUNCTION( L_var_dump, var_dump );
}
