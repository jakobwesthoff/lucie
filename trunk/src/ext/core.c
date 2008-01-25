#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"

void test() 
{
    lua_State *L;

    // Init the lua engine
    DEBUGLOG( "Initializing lua state" );
    L = luaL_newstate();
    DEBUGLOG( "New state created L=0x%x", (int)L );
    luaL_openlibs( L ); 
    DEBUGLOG( "Lua lib opened" );

    // Load the script for execution
    DEBUGLOG( "Trying to load lua file: %s", "" );
    LUACHECK( luaL_loadfile( L, 0 ) );

    // Execute script
    DEBUGLOG( "Executing loaded script" );
    LUACHECK( lua_pcall( L, 0, LUA_MULTRET, 0 ) );

    // Destroy the lua environment
    DEBUGLOG( "Closing lua" );
    lua_close( L );
}
