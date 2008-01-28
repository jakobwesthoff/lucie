#ifndef LUCIE_H
#define LUCIE_H

typedef struct 
    {
        char* name;
        char* author;
        char* email;        
    } extension_t;

extern int extension_count;
extern extension_t **extensions;

#define REGISTER_EXTENSION(nameVal, authorVal, emailVal) \
    {                                                                                                               \
        if ( extension_count == 0 )                                                                                 \
        {                                                                                                           \
            extensions = ( extension_t** ) malloc( 4 * sizeof( extension_t* ) );                                    \
        }                                                                                                           \
        else if ( extension_count % 4 == 0 )                                                                        \
        {                                                                                                           \
            extensions = ( extension_t** ) realloc( extensions, ( extension_count + 4 ) * sizeof( extension_t* ) );     \
        }                                                                                                           \
        extensions[extension_count] = ( extension_t* ) malloc( sizeof( extension_t ) );                             \
        extensions[extension_count]->name = nameVal;                                                                \
        extensions[extension_count]->author = authorVal;                                                            \
        extensions[extension_count++]->email = emailVal;                                                            \
    }

#define NAMESPACE_BEGIN(namespaceVal) \
    {                                   \
        char* namespace=namespaceVal;   \
        lua_newtable( L );               

#define NAMESPACE_END(...) \
        lua_setglobal( L, namespace );  \
    }


#define REGISTER_NAMESPACE_FUNCTION(func, luafunc) \
    lua_pushstring( L, #luafunc );      \
    lua_pushcfunction( L, func );       \
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
