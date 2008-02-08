#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "lucie.h"

extern char **environ;

int urldecode( char* data ) 
{
    int i = 0;
    int j = 0;
    int len = 0;
    char* result; 

    len = strlen( data );
    // Our new string is smaler than the old one, therefore the same size is
    // definatly enough ( Null termination not needed )
    result = smalloc( ( len ) * sizeof( char ) );
    memset( result, 0, len );    
    while( i < len ) 
    {
        if ( data[i] == '%' )
        {
            char c1, c2;           
            i++;
            c1 = tolower( data[i++] );
            c2 = tolower( data[i++] );
            if ( !isxdigit( c1 ) || !isxdigit( c2 ) ) 
            {
                THROW_ERROR( "Invalid hexadecimal character after %% escape sign." );
                free( result );
                return false;
            }
            // Convert hexadecimal number to character index
            result[j++] = ( 16 * ( ( c1 <= '9' ) 
                                 ? ( c1 - '0' ) 
                                 : ( c1 - 'a' + 10 ) ) ) 
                        + ( ( c2 <= '9' ) 
                          ? ( c2 - '0' ) 
                          : ( c2 - 'a' + 10 ) );
        }
        else if ( data[i] == '+' )
        {
            i++;
            result[j++] = ' ';
        }
        else 
        {
            result[j++] = data[i++];
        }
    }
    memcpy( data, result, len );
    free( result );
    return true;
}

void add_env_variable( lua_State* L,  const char* key, const char* env ) 
{
    char* envdata;
    
    DEBUGLOG( "Registering env variable: %s", env );

    // Put the key onto the stack
    lua_pushstring( L, key );

    // Put the environment data onto the stack
    envdata = getenv( env );
    if ( envdata == NULL ) 
    {
        lua_pushstring( L, "" );
    }
    else 
    {
        lua_pushstring( L, envdata );
    }

    lua_settable( L, -3 );
}

void decode_url_parameter_string( lua_State* L, const char* data )
{
    int entries = 1;
    int len     = 0;

    if ( data != NULL && strcmp( data, "" ) ) 
    {           
        char* query_string;
        char* pointer_to_free;
        
        // Copy the data to work on it
        query_string = strdup( data );
        pointer_to_free = query_string;

        // Split the query string at every & sign
        int i;
        len = strlen( query_string );
        for ( i = 0; i < len; i++ ) 
        {
            if ( query_string[i] == '&' ) 
            {
                query_string[i] = 0;
                entries++;
            }
        }

        // Loop through all get key value pairs
        while( entries > 0 ) 
        {
            int found_equalsign = false;

            // Split at the = sign
            len = strlen( query_string );
            for( i = 0; i < len; i++ ) 
            {
                if ( query_string[i] == '=' ) 
                {
                    query_string[i] = 0;
                    found_equalsign = true;
                    break;
                }
            }
            
            // Decode and push the key
            {
                char* decoded = strdup( query_string );
                if ( urldecode( decoded ) ) 
                {
                    lua_pushstring( L, decoded );
                }
                free( decoded );
            }

            // Decode and push value or boolean true
            if ( found_equalsign ) 
            {
                query_string = query_string + strlen( query_string ) + 1;
                {
                    char* decoded = strdup( query_string );
                    if ( urldecode( decoded ) ) 
                    {
                        lua_pushlstring( L, decoded, strlen( decoded ) );
                    }
                    free( decoded );
                }
            }
            else 
            {
                lua_pushboolean( L, true );
            }
                            
            // Set the table entry
            lua_settable( L, -3 );

            // Advance to next entry
            query_string = query_string + strlen( query_string ) + 1;
            entries--;
        }
        
        free( pointer_to_free );
    }
}

void init_superglobals( lua_State* L ) 
{
    // Create and fillup the _SERVER table
    lua_newtable( L );
    {
        add_env_variable( L, "SOFTWARE", "SERVER_SOFTWARE" );
        add_env_variable( L, "NAME", "SERVER_NAME" );
        add_env_variable( L, "GATEWAY_INTERFACE", "GATEWAY_INTERFACE" );
        add_env_variable( L, "PROTOCOL", "SERVER_PROTOCOL" );
        add_env_variable( L, "PORT", "SERVER_PORT" );
        add_env_variable( L, "REQUEST_METHOD", "REQUEST_METHOD" );
        add_env_variable( L, "PATH_INFO", "PATH_INFO" );
        add_env_variable( L, "PATH_TRANSLATED", "PATH_TRANSLATED" );
        add_env_variable( L, "SCRIPT_NAME", "SCRIPT_NAME" );
        add_env_variable( L, "QUERY_STRING", "QUERY_STRING" );
        add_env_variable( L, "REMOTE_HOST", "REMOTE_HOST" );
        add_env_variable( L, "REMOTE_ADDR", "REMOTE_ADDR" );
        add_env_variable( L, "AUTH_TYPE", "AUTH_TYPE" );
        add_env_variable( L, "REMOTE_USER", "REMOTE_USER" );
        add_env_variable( L, "REMOTE_IDENT", "REMOTE_IDENT" );
        add_env_variable( L, "CONTENT_TYPE", "CONTENT_TYPE" );
        add_env_variable( L, "CONTENT_LENGTH", "CONTENT_LENGTH" );
    }
    lua_setglobal( L, "_SERVER" );

    // Create and fillup _GET table
    lua_newtable( L );
    DEBUGLOG( "Decoding query_string" );
    {
        decode_url_parameter_string( L, getenv( "QUERY_STRING" ) );
    }
    lua_setglobal( L, "_GET" );

    // Create and fillup _POST table
    lua_newtable( L );
    DEBUGLOG( "Decoding post data" );
    {        
        char* postdata       = NULL;
        int readBytes        = 0;
        int overallReadBytes = 0;
        char* contentType    = getenv( "CONTENT_TYPE" );

        if ( contentType != NULL && !strcmp( contentType, "application/x-www-form-urlencoded" ) ) 
        {
            postdata = smalloc( sizeof( char ) * 1024 );
            memset( postdata, 0, 1024 );
            
            while( ( readBytes = fread( postdata + overallReadBytes, sizeof( char ), 1023, stdin ) ) != 0 ) 
            {
                overallReadBytes += readBytes;
                postdata = srealloc( postdata, sizeof( char ) * ( overallReadBytes + 1024 ) );
                memset( postdata + overallReadBytes, 0, 1024 );
            }

            decode_url_parameter_string( L, postdata );
            free( postdata );
        }
    }
    lua_setglobal( L, "_POST" );

    // Create and fillup _HEADER table
    lua_newtable( L );
    DEBUGLOG( "Setting _HEADER" );
    {
       char** env;
       // Loop through all defined environment variables
       for( env = environ; *env; env++ ) 
       {      
           int i;
           int len;
           char* envdata;

           if( strstr( *env, "HTTP_" ) != *env ) 
           {
               // The variable name does not start with HTTP_
               continue;
           }
           
           // Split at the equal sign
           envdata = strdup( *env );
           len = strlen( envdata );
           for( i = 5; i < len; i++ ) 
           {
               if ( envdata[i] == '=' ) 
               {
                   envdata[i] = 0;
                   break;
               }
           }
           
           // Push the key, without the HTTP_
           lua_pushstring( L, envdata + 5 );
           // Push the data
           lua_pushstring( L, envdata + strlen(envdata) + 1 );
           // Add the table entry
           lua_settable( L, -3 );

           // Free the temporary string
           free( envdata );
       }
    }
    lua_setglobal( L, "_HEADER" );
}
