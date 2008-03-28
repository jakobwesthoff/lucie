#ifndef LUCIE_H
#define LUCIE_H

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define APPLICATION_NAME "LuCIE"
#define APPLICATION_FULLNAME "Lua common internet environment"
#define APPLICATION_AUTHOR "Jakob Westhoff <jakob@westhoffswelt.de>"
#define APPLICATION_VERSION "0.0.1"

typedef struct 
    {
        char* name;
        char* author;
        char* email;        
    } extension_t;

extern const char* config_file;

extern int extension_count;
extern extension_t **extensions;

extern char errorstring[4096];

extern void* smalloc( size_t bytes );
extern void* srealloc( void* old, size_t bytes );
extern int file_exists( const char* filename );

#ifndef FALSE 
    #define FALSE 0
#endif
#ifndef false
    #define false 0
#endif
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef true
    #define true 1
#endif
#ifndef NULL  
    #define NULL 0
#endif
#ifndef null
    #define null 0
#endif

#define THROW_ERROR(fmt, ...) \
    errno = 0;                                      \
    sprintf( errorstring, fmt, ##__VA_ARGS__ ); 

#define print_error(desc) \
    if ( errno != 0 )                                                                               \
    {                                                                                               \
        fprintf( stderr, "%s <%d>: %s\n\t%s\n", __FILE__, __LINE__, desc, strerror( errno ) );      \
    }                                                                                               \
    else                                                                                            \
    {                                                                                               \
        fprintf( stderr, "%s <%d>: %s\n\t%s\n", __FILE__, __LINE__, desc, errorstring );            \
    }

#define return_error() \
    {               \
        RETURN_NIL(); \
        if ( errno != 0 )                                                                               \
        {                                                                                               \
            RETURN_STRING( strerror( errno ) );      \
        }                                            \
        else                                         \
        {                                            \
            RETURN_STRING( errorstring );            \
        }                                            \
        return 2;                                    \
    }

#define REGISTER_EXTENSION(nameVal, authorVal, emailVal) \
    {                                                                                                                   \
        if ( extension_count == 0 )                                                                                     \
        {                                                                                                               \
            extensions = ( extension_t** ) smalloc( 4 * sizeof( extension_t* ) );                                       \
        }                                                                                                               \
        else if ( extension_count % 4 == 0 )                                                                            \
        {                                                                                                               \
            extensions = ( extension_t** ) srealloc( extensions, ( extension_count + 4 ) * sizeof( extension_t* ) );    \
        }                                                                                                               \
        extensions[extension_count] = ( extension_t* ) smalloc( sizeof( extension_t ) );                                 \
        extensions[extension_count]->name = nameVal;                                                                    \
        extensions[extension_count]->author = authorVal;                                                                \
        extensions[extension_count++]->email = emailVal;                                                                \
    }

#define NAMESPACE_BEGIN(namespaceVal) \
    {                                 \
        char* namespace=namespaceVal; \
        lua_newtable( L ); 

#define NAMESPACE_END(...) \
        lua_setglobal( L, namespace );  \
    }


#define REGISTER_NAMESPACE_FUNCTION(func, luafunc)  \
    lua_pushstring( L, #luafunc );                  \
    lua_pushcfunction( L, func );                   \
    lua_settable( L, -3 );              

#define REGISTER_GLOBAL_FUNCTION(func, luafunc) \
    lua_pushcfunction( L, func );               \
    lua_setglobal( L, #luafunc );

#define PARAM_BOOLEAN(bool) \
    {                                                                                                       \
        if ( !lua_isboolean( L, -1 ) )                                                                      \
        {                                                                                                   \
            return luaL_error( L, "Tried to retrieve a boolean parameter which is not of type boolean" );   \
        }                                                                                                   \
        bool = lua_toboolean( L, -1 );                                                                      \
        lua_pop( L, 1 );                                                                                    \
    }

#define PARAM_DOUBLE(double) \
    {                                                                                                       \
        if ( !lua_isnumber( L, -1 ) )                                                                       \
        {                                                                                                   \
            return luaL_error( L, "Tried to retrieve a double parameter which is not of type number" );     \
        }                                                                                                   \
        double = lua_tonumber( L, -1 );                                                                     \
        lua_pop( L, 1 );                                                                                    \
    }

#define PARAM_INTEGER(integer) \
    {                                                                                                       \
        if ( !lua_isnumber( L, -1 ) )                                                                       \
        {                                                                                                   \
            return luaL_error( L, "Tried to retrieve a integer parameter which is not of type number" );    \
        }                                                                                                   \
integer = lua_tointeger( L, -1 );                                                                   \
        lua_pop( L, 1 );                                                                                    \
    }

#define PARAM_STRLEN(len) \
    {                                                                                                       \
        len = lua_strlen( L, -1 );                                                                          \
    }

#define PARAM_STRING(string) \
    {                                                                                                       \
        if ( ( string = lua_tostring( L, -1 ) ) == 0 )                                                      \
        {                                                                                                   \
            return luaL_error( L, "Tried to retrieve a string parameter which is not of type string" );     \
        }                                                                                                   \
        lua_pop( L, 1 );                                                                                    \
    }

#define PARAM_BINARY_STRING(string, len) \
    {                                                                                                       \
        if ( ( string = lua_tostring( L, -1 ) ) == 0 )                                                      \
        {                                                                                                   \
            return luaL_error( L, "Tried to retrieve a string parameter which is not of type string" );     \
        }                                                                                                   \
        len = lua_strlen( L, -1 );                                                                          \
        lua_pop( L, 1 );                                                                                    \
    }

#define RETURN_NIL() \
    lua_pushnil( L );

#define RETURN_BOOLEAN(value) \
    lua_pushboolean( L, value );

#define RETURN_DOUBLE(value) \
    lua_pushnumber( L, value );

#define RETURN_INTEGER(value) \
    lua_pushinteger( L, value );

#define RETURN_STRING(value) \
    lua_pushlstring( L, value, strlen( value ) );

#define RETURN_BINARY_STRING(value, len) \
    lua_pushlstring( L, value, len );

#define RETURN_ARRAY(type, datasrc, count)  \
    {                                       \
        int i=1;                            \
        lua_newtable( L );                  \
        for( i=0; i<count; i++ )            \
        {                                   \
            lua_pushinteger( L, i + 1 );    \
            RETURN_ ## type(datasrc[i])     \
            lua_settable( L, -3 );          \
        }                                   \
    }

#define RETURN_BOOLEAN_ARRAY( datasrc, count ) \
    RETURN_ARRAY( BOOLEAN, datasrc, count )
    
#define RETURN_DOUBLE_ARRAY( datasrc, count ) \
    RETURN_ARRAY( DOUBLE, datasrc, count )
    
#define RETURN_INTEGER_ARRAY( datasrc, count ) \
    RETURN_ARRAY( INTEGER, datasrc, count )
    
#define RETURN_STRING_ARRAY( datasrc, count ) \
    RETURN_ARRAY( STRING, datasrc, count )
    
#define LUACHECK(x) \
    {                                                                                                     \
        int error = x;                                                                                    \
        if ( error )                                                                                      \
        {                                                                                                 \
             fprintf( stderr, "%s <%d>: %s\n\t%s\n", __FILE__, __LINE__, #x, lua_tostring( L, -1 ) );     \
             lua_pop( L, 1 );                                                                             \
             exit( EXIT_FAILURE );                                                                        \
        }                                                                                                 \
    }

/* Debug functions are only declared if the DEBUG flag is set */
#ifdef DEBUG
    #define DEBUGLOG(fmt, ...) \
        fprintf( stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__ )
    
#else
    #define DEBUGLOG(...)
#endif

#endif
