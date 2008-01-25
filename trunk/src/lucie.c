#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lucie.h"

int main( int argc, char** argv )
{    
    lua_State *L;

    // Init the lua engine
    DEBUGLOG( "Initializing lua state" );
    L = luaL_newstate();
    luaL_openlibs( L ); 
    DEBUGLOG( "Lua lib opened L=0x%x", (int)L );

    // Load the script for execution
    DEBUGLOG( "Trying to load lua file: %s", argv[1] );
    LUACHECK( luaL_loadfile( L, argv[1] ) );

    // Execute script
    DEBUGLOG( "Executing loaded script" );
    LUACHECK( lua_pcall( L, 0, LUA_MULTRET, 0 ) );

    // Destroy the lua environment
    DEBUGLOG( "Closing lua" );
    lua_close( L );

    return 0;
}
