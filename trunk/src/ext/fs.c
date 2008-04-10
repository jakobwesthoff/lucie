#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"

void register_extension( lua_State *L ) 
{
    REGISTER_EXTENSION( "core", "Jakob Westhoff", "jakob@westhoffswelt.de" );
    NAMESPACE_BEGIN( "fs" );
        REGISTER_NAMESPACE_FUNCTION( L_file_exists, file_exists );
        REGISTER_NAMESPACE_FUNCTION( L_is_file, is_file );
        REGISTER_NAMESPACE_FUNCTION( L_is_dir, is_dir );
        REGISTER_NAMESPACE_FUNCTION( L_is_readable, is_readable );
        REGISTER_NAMESPACE_FUNCTION( L_is_writable, is_writable );
        REGISTER_NAMESPACE_FUNCTION( L_is_executable, is_executable );
        REGISTER_NAMESPACE_FUNCTION( L_is_link, is_link );
        REGISTER_NAMESPACE_FUNCTION( L_file_mtime, file_mtime );
        REGISTER_NAMESPACE_FUNCTION( L_file_atime, file_atime );
        REGISTER_NAMESPACE_FUNCTION( L_file_ctime, file_ctime );
        REGISTER_NAMESPACE_FUNCTION( L_file_owner, file_owner );
        REGISTER_NAMESPACE_FUNCTION( L_file_group, file_group );
        REGISTER_NAMESPACE_FUNCTION( L_file_size, file_size );
        REGISTER_NAMESPACE_FUNCTION( L_basename, basename );
        REGISTER_NAMESPACE_FUNCTION( L_dirname, dirname );
        REGISTER_NAMESPACE_FUNCTION( L_ )
    NAMESPACE_END( "fs" );
}
