#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"

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

void register_extension( lua_State *L ) 
{
    REGISTER_EXTENSION( "core", "Jakob Westhoff", "jakob@westhoffswelt.de" );
    REGISTER_GLOBAL_FUNCTION( L_var_dump, var_dump );
}
