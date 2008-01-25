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
    L = luaL_newstate();
    luaL_openlibs( L ); 

    // Load the script for execution
    LUACHECK( luaL_loadfile( L, argv[1] ) );

    // Execute script
    LUACHECK( lua_pcall( L, 0, LUA_MULTRET, 0 ) );

    // Destroy the lua environment
    lua_close( L );

    return 0;
}
