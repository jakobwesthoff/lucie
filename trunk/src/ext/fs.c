#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <sys/stat.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../lucie.h"
#include "fs.h"

int stat_file( lua_State *L, enum statquestion question ) 
{
    const char* filename = luaL_checkstring( L, 1 );
    struct stat buffer;
    uid_t uid = getuid();
    gid_t gid = getgid();
    
    if ( stat( filename, &buffer ) != 0 ) 
    {
        if ( errno == ENOENT 
            && ( question == QUESTION_EXISTS 
              || question == QUESTION_DIR 
              || question == QUESTION_FILE 
              || question == QUESTION_READABLE 
              || question == QUESTION_WRITABLE 
              || question == QUESTION_EXECUTABLE 
            )
           ) 
        {
            RETURN_BOOLEAN( false );
            return 1;
        }

        print_error( "File stats could not be retrieved" );
        return 0; 
    }

    switch( question ) 
    {
        case QUESTION_EXISTS:
            RETURN_BOOLEAN( true );
        break;
        case QUESTION_DIR:
            RETURN_BOOLEAN( S_ISDIR( buffer.st_mode ) != 0 )            
        break;
        case QUESTION_LINK:
            RETURN_BOOLEAN( S_ISLNK( buffer.st_mode ) != 0 )            
        break;
        case QUESTION_FILE:
            RETURN_BOOLEAN( 
                S_ISDIR( buffer.st_mode ) == 0
             && S_ISLNK( buffer.st_mode ) == 0
            );
        break;
        case QUESTION_READABLE:
            RETURN_BOOLEAN( 
                ( ( buffer.st_mode & S_IROTH ) != 0 )
             || ( ( buffer.st_mode & S_IRGRP ) != 0 && buffer.st_gid == gid )
             || ( ( buffer.st_mode & S_IRUSR ) != 0 && buffer.st_uid == uid )
            );
        break;
        case QUESTION_WRITABLE:
            RETURN_BOOLEAN( 
                ( ( buffer.st_mode & S_IWOTH ) != 0 )
             || ( ( buffer.st_mode & S_IWGRP ) != 0 && buffer.st_gid == gid )
             || ( ( buffer.st_mode & S_IWUSR ) != 0 && buffer.st_uid == uid )
            );
        break;
        case QUESTION_EXECUTABLE:
            RETURN_BOOLEAN( 
                ( ( buffer.st_mode & S_IXOTH ) != 0 )
             || ( ( buffer.st_mode & S_IXGRP ) != 0 && buffer.st_gid == gid )
             || ( ( buffer.st_mode & S_IXUSR ) != 0 && buffer.st_uid == uid )
            );
        break;       
        case QUESTION_MTIME:
            RETURN_INTEGER( buffer.st_mtime );
        break;
        case QUESTION_ATIME:
            RETURN_INTEGER( buffer.st_atime );
        break;
        case QUESTION_CTIME:
            RETURN_INTEGER( buffer.st_ctime );
        break;
        case QUESTION_OWNER:
            RETURN_INTEGER( buffer.st_uid );
        break;
        case QUESTION_GROUP:
            RETURN_INTEGER( buffer.st_gid );
        break;
        case QUESTION_SIZE:
            RETURN_INTEGER( buffer.st_size );
        break;
    }
    return 1;
}

int L_basename( lua_State *L ) 
{
    const char* filename = luaL_checkstring( L, 1 );
    const char* basename = NULL;

    for( basename = filename; *filename; filename++ ) 
    {
        if ( *filename == '/' ) 
        {
            basename = filename + 1;
        }
    }
    RETURN_STRING( basename );
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

int L_file_exists( lua_State *L ) 
{
    return stat_file( L, QUESTION_EXISTS );
}

int L_is_file( lua_State *L ) 
{
    return stat_file( L, QUESTION_FILE );
}

int L_is_dir( lua_State *L ) 
{
    return stat_file( L, QUESTION_DIR );
}

int L_is_readable( lua_State *L ) 
{
    return stat_file( L, QUESTION_READABLE );
}

int L_is_writable( lua_State *L ) 
{
    return stat_file( L, QUESTION_WRITABLE );
}

int L_is_executable( lua_State *L ) 
{
    return stat_file( L, QUESTION_EXECUTABLE );
}

int L_is_link( lua_State *L ) 
{
    return stat_file( L, QUESTION_LINK );
}

int L_file_mtime( lua_State *L ) 
{
    return stat_file( L, QUESTION_MTIME );
}

int L_file_atime( lua_State *L ) 
{
    return stat_file( L, QUESTION_ATIME );
}

int L_file_ctime( lua_State *L ) 
{
    return stat_file( L, QUESTION_CTIME );
}

int L_file_owner( lua_State *L ) 
{
    return stat_file( L, QUESTION_OWNER );
}

int L_file_group( lua_State *L ) 
{
    return stat_file( L, QUESTION_GROUP );
}

int L_file_size( lua_State *L ) 
{
    return stat_file( L, QUESTION_SIZE );
}

void register_extension( lua_State *L ) 
{
    REGISTER_EXTENSION( "fs", "Jakob Westhoff", "jakob@westhoffswelt.de" );
    NAMESPACE_BEGIN( "file" );
        REGISTER_NAMESPACE_FUNCTION( L_file_exists, exists );
        REGISTER_NAMESPACE_FUNCTION( L_is_file, is_file );
        REGISTER_NAMESPACE_FUNCTION( L_is_dir, is_dir );
        REGISTER_NAMESPACE_FUNCTION( L_is_link, is_link );
        REGISTER_NAMESPACE_FUNCTION( L_is_readable, is_readable );
        REGISTER_NAMESPACE_FUNCTION( L_is_writable, is_writable );
        REGISTER_NAMESPACE_FUNCTION( L_is_executable, is_executable );
        REGISTER_NAMESPACE_FUNCTION( L_file_mtime, mtime );
        REGISTER_NAMESPACE_FUNCTION( L_file_atime, atime );
        REGISTER_NAMESPACE_FUNCTION( L_file_ctime, ctime );
        REGISTER_NAMESPACE_FUNCTION( L_file_owner, owner );
        REGISTER_NAMESPACE_FUNCTION( L_file_group, group );
        REGISTER_NAMESPACE_FUNCTION( L_file_size, size );
        REGISTER_NAMESPACE_FUNCTION( L_basename, basename );
        REGISTER_NAMESPACE_FUNCTION( L_dirname, dirname );
    NAMESPACE_END( "file" );
}

