#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lucie.h"

int main( int argc, char** argv )
{    
    typedef void ( *test_t )();
    test_t test;
    lua_State *L;
    void* dlh;

    // Init the lua engine
    DEBUGLOG( "Initializing lua state" );
    L = luaL_newstate();
    DEBUGLOG( "New state created L=0x%x", (int)L );
    luaL_openlibs( L ); 
    DEBUGLOG( "Lua lib opened" );

    // Open the core shared lib for testing
    DEBUGLOG( "Opening ext/core.so" );
    dlh = dlopen( "./src/ext/core.so", RTLD_LAZY );
    DEBUGLOG( "Extension opened handle=0x%x", (int)dlh );

    // Try to load the test function
    test = ( test_t )dlsym( dlh, "test" );
    
    DEBUGLOG( "Executing test function" );
    test();

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
