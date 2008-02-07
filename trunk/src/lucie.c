#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <libgen.h>
#include <ctype.h>

#include "lucie.h"
#include "inireader.h"

#include "lucieP.h"

void* smalloc( size_t bytes )
{
    void* allocated;
    if ( ( allocated = malloc( bytes ) ) == NULL ) 
    {
        THROW_ERROR( "Failed to allocate %d bytes.", bytes );
        print_error( "Allocation failed" );
        exit( EXIT_FAILURE );
    }
    return allocated;
}

void* srealloc( void* old, size_t bytes )
{
    void* allocated;
    if ( ( allocated = realloc( old, bytes ) ) == NULL ) 
    {
        THROW_ERROR( "Failed to allocate %d bytes.", bytes );
        print_error( "Reallocation failed" );
        exit( EXIT_FAILURE );
    }
    return allocated;
}

int file_exists( const char* filename ) 
{
	struct stat stats;
	
	if ( stat( filename, &stats ) == -1 )
	{
		if ( errno == ENOENT )
		{
			return false;
		}
		else
		{
			// An error occured during the stats call
	        print_error("An error occured during a file_exists check");
			exit( EXIT_FAILURE );
		}
	}
	return true;
}

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

void cleanup_registered_extensions()
{
    int i;

    DEBUGLOG( "Freeing memory of %i extensions.", extension_count );
    for( i = 0; i < extension_count; i++ )
    {
        DEBUGLOG( "Freeing extension %i.", i );
        free( extensions[i] );
    }
    DEBUGLOG( "Freeing extension array." );
    free( extensions );
    DEBUGLOG( "All extension memory freed." );
}

void register_extensions( lua_State* L ) 
{
    inifile_t* inifile         = NULL;
    inireader_iterator_t* iter = NULL;
    inireader_entry_t* current = NULL;
    
    const char* lucie_config_path_env = NULL;
    const char* config_file           = NULL;

    // Harcoded configpaths to check
    const char* hardcoded_path[] = {
        "/etc/lucie.conf",
        "/usr/local/etc/lucie.conf",        
    };

    // First try to read the configpath from the LUCIE_CONFIG_PATH environment
    // variable
    lucie_config_path_env = getenv( "LUCIE_CONFIG_FILE" );
    if ( lucie_config_path_env != NULL ) 
    {
        if ( file_exists( lucie_config_path_env ) ) 
        {
            config_file = lucie_config_path_env;
        }
    }

    // Check the working directory for a config file next
    if ( config_file == NULL && file_exists( "./lucie.conf" ) ) 
    {
        config_file = "./lucie.conf";
    }

    // Check the hardcoded paths for the configfile
    if ( config_file == NULL ) 
    {
        int i = 0;
        for( i=0; i<2; i++ ) 
        {
            if ( file_exists( hardcoded_path[i] ) ) 
            {
                config_file = hardcoded_path[i];
                break;
            }
        }
    }

    // Exit if we can not find a configuration file
    if ( config_file == NULL ) 
    {
        THROW_ERROR( "The configuration file could not be found anywhere." );
        print_error( "Configuration file read error" );
        exit( EXIT_FAILURE );
    }

    // Open the configfile and get an iterator for the extension list
    inifile = inireader_open( config_file );
    iter = inireader_get_iterator( inifile, "extensions", "extension", 0, 0 );

    // Iterate over all of the given extensions and try to load them.
    for( current = inireader_iterate( iter ); current != NULL; current = inireader_iterate( iter ) ) 
    {
        void* so_handle = NULL;
        typedef void ( *register_extension_t )( lua_State *L );
        register_extension_t register_extension;

        DEBUGLOG( "Trying to load extension: %s", current->data );
        
        if ( !file_exists( current->data ) ) 
        {
            THROW_ERROR( "The specified extension \"%s\" does not exist.", current->data );
            print_error( "Extension loading failed" );
            exit( EXIT_FAILURE );
        }

        // Try to open the extensions shared library file
        DEBUGLOG( "Opening %s", current->data );
        if ( ( so_handle = dlopen( current->data, RTLD_LAZY ) ) == NULL ) 
        {
            THROW_ERROR( "%s", dlerror() );
            print_error( "Extension loading failed" );
            exit( EXIT_FAILURE );
        }
        DEBUGLOG( "Extension opened handle=0x%x", (int)so_handle );

        // Try to load the register extension function
        DEBUGLOG( "Trying to get symbol of the register_extension function" );
        if ( ( register_extension = ( register_extension_t )dlsym( so_handle, "register_extension" ) ) == NULL )
        {
            THROW_ERROR( "%s", dlerror() );
            print_error( "Extension loading failed" );
            exit( EXIT_FAILURE );
        }
        DEBUGLOG( "Symbol register_extension retrieved. register_extension=0x%x", (int)register_extension );

        DEBUGLOG( "Calling register_extension" );
        register_extension( L );
        DEBUGLOG( "Extension registered" );
    }
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

const char* lucie_reader( lua_State* L, void* data, size_t* size ) 
{
    static char* output_buffer;    

    // Check if we have already read the whole file.
    if ( feof( (FILE*)data ) ) 
    {
        // Free the buffer and return null
        free( output_buffer );
        return NULL;
    }

    {
        char* reading_buffer = NULL;
        char* working_buffer = NULL;
        int filesize       = 0;

        // Get size of the script to load;
        fseek( (FILE*)data, 0, SEEK_END );
        filesize = ftell( (FILE*)data );
        fseek( (FILE*)data, 0, SEEK_SET );

        // Allocate space for it
        reading_buffer = smalloc( filesize + 1 );
        memset( reading_buffer, 0, filesize + 1 );

        // Read the file into memory
        fread( reading_buffer, sizeof( char ), filesize, (FILE*)data );
        {
            // We need to remember certain values during the processing
            enum { HTML=1,
                   HTML_LONG_BRACKET = 2,
                   CHUNK = 3,
                   LINE_COMMENT = 4,
                   POSSIBLE_COMMENT = 5,
                   COMMENT = 6,
                   POSSIBLE_COMMENT_END = 7,
                   LONG_BRACKET_STRING = 8,
                   POSSIBLE_LONG_BRACKET_STRING = 9,
                   POSSIBLE_LONG_BRACKET_STRING_END = 10,                 
                   SINGLE_QUOTED_STRING = 11,
                   DOUBLE_QUOTED_STRING = 12,
                   LONG_BRACKET = 13
            } state = 1;
            
            int htmllevel          = 0;
            int possiblehtmllevel  = 0;
            int chunklevel         = 0;
            int possiblechunklevel = 0;
            char* html_buffer = NULL;
            int i = 0;

            DYNAMIC_STRING_INIT( html_buffer );
            DYNAMIC_STRING_INIT( working_buffer );

            while( i < filesize ) 
            {
                if ( state == HTML && reading_buffer[i] == '<' && reading_buffer[i+1] == '?' && reading_buffer[i+2] == 'l' && reading_buffer[i+3] == 'u' && reading_buffer[i+4] == 'c' && reading_buffer[i+5] == 'i' && reading_buffer[i+6] == 'e' ) 
                {                                        
                    // Lucie starttag found
                    int j = 0;
                    state = CHUNK;
                    DYNAMIC_STRING_ADD( working_buffer, "io.write([" );
                    for( j = 0 ; j <= htmllevel; j++ ) 
                    {
                        DYNAMIC_STRING_ADD( working_buffer, "=" );
                    }
                    DYNAMIC_STRING_ADD( working_buffer, "[\n" );
                    DYNAMIC_STRING_ADD( working_buffer, html_buffer );
                    DYNAMIC_STRING_ADD( working_buffer, "]" );
                    for( j = 0 ; j <= htmllevel; j++ ) 
                    {
                        DYNAMIC_STRING_ADD( working_buffer, "=" );
                    }
                    DYNAMIC_STRING_ADD( working_buffer, "]);\n" );                    
                    DYNAMIC_STRING_INIT( html_buffer );
                    i += 7;
                }
                else if ( state == HTML && reading_buffer[i] == ']' && reading_buffer[i+1] == '=' ) 
                {
                    // Possible HTML_LONG_BRACKET
                    state = HTML_LONG_BRACKET;
                    possiblehtmllevel = 0;                    
                    DYNAMIC_STRING_ADD_CHAR( html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == HTML ) 
                {
                    // HTML content
                    DYNAMIC_STRING_ADD_CHAR( html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == HTML_LONG_BRACKET && reading_buffer[i] == '=' ) 
                {
                    // Possible next long bracket level
                    possiblehtmllevel++;
                    DYNAMIC_STRING_ADD_CHAR( html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == HTML_LONG_BRACKET && reading_buffer[i] == ']' ) 
                {
                    // possible long bracket is new bracket level
                    state = HTML;
                    htmllevel = ( possiblehtmllevel > htmllevel ) ? possiblehtmllevel : htmllevel;
                    DYNAMIC_STRING_ADD_CHAR( html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == HTML_LONG_BRACKET ) 
                {
                    // We were wrong there was no long bracket
                    state = HTML;
                    DYNAMIC_STRING_ADD_CHAR( html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == CHUNK && reading_buffer[i] == '?' && reading_buffer[i+1] == '>' ) 
                {
                    // Lucie endtag
                    state = HTML;
                    DYNAMIC_STRING_INIT( html_buffer );
                    htmllevel = 0;
                    i += 2;
                }
                else if ( state == CHUNK && reading_buffer[i] == '-' && reading_buffer[i+1] == '-' && reading_buffer[i+2] == '[' ) 
                {
                    // Possible comment area
                    state = POSSIBLE_COMMENT;
                    DYNAMIC_STRING_ADD( working_buffer, "--[" );
                    i += 3;
                }
                else if ( state == CHUNK && reading_buffer[i] == '-' && reading_buffer[i+1] == '-' ) 
                {
                    // Line comment
                    state = LINE_COMMENT;
                    i += 2;
                }
                else if ( state == CHUNK && reading_buffer[i] == '[' && reading_buffer[i+1] == '[' ) 
                {
                    // Zero level long bracket found
                    state = LONG_BRACKET_STRING;
                    chunklevel = 0;
                    DYNAMIC_STRING_ADD( working_buffer, "[[" );
                    i += 2;
                }
                else if ( state == CHUNK && reading_buffer[i] == '[' && reading_buffer[i+1] == '=' ) 
                {                    
                    // Possible long bracket
                    state = POSSIBLE_LONG_BRACKET_STRING;
                    possiblechunklevel = 0;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING && reading_buffer[i] == '=' ) 
                {
                    // Possible new string level
                    possiblechunklevel++;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING && reading_buffer[i] == '[' ) 
                {
                    // We have a new string level.
                    state = LONG_BRACKET_STRING;
                    chunklevel = possiblechunklevel;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING ) 
                {
                    // We have a new string level.
                    state = CHUNK;
                    possiblechunklevel = 0;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == CHUNK && reading_buffer[i] == '"' ) 
                {
                    // Double quoted string
                    state = DOUBLE_QUOTED_STRING;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == CHUNK && reading_buffer[i] == '\'' ) 
                {
                    // Single quoted string
                    state = SINGLE_QUOTED_STRING;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT && reading_buffer[i] == '=' ) 
                {
                    // Possible new comment level
                    possiblechunklevel++;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT && reading_buffer[i] == ']' ) 
                {
                    // New comment level is affirmative
                    state = COMMENT;
                    chunklevel = possiblechunklevel;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT && ( reading_buffer[i] == '\n' || ( reading_buffer[i] == '\r' && reading_buffer[i+1] == '\n' ) ) )
                {
                    // It was just a line comment and is ended now
                    state = CHUNK;
                    DYNAMIC_STRING_ADD( working_buffer, "\n" );
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT ) 
                {
                    // We were wrong about the comment level
                    // We are just inside a simple line comment
                    state = LINE_COMMENT;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == LINE_COMMENT && ( reading_buffer[i] == '\n' || ( reading_buffer[i] == '\r' && reading_buffer[i+1] == '\n' ) ) )
                {
                    // Line comment end
                    state = CHUNK;
                    DYNAMIC_STRING_ADD( working_buffer, "\n" );
                    i++;
                }
                else if ( state == COMMENT && reading_buffer[i] == ']' ) 
                {
                    // Possible comment end
                    state = POSSIBLE_COMMENT_END;
                    possiblechunklevel = 0;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT_END && reading_buffer[i] == '=' ) 
                {
                    // Possible new comment end level
                    possiblechunklevel++;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT_END && reading_buffer[i] == ']' ) 
                {
                    // We have a new comment end level. We need to check if it is the end of comment though
                    if ( possiblechunklevel == chunklevel ) 
                    {
                        state = CHUNK;                        
                    }
                    else 
                    {
                        possiblechunklevel = 0;
                    }
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == LONG_BRACKET_STRING && reading_buffer[i] == ']' ) 
                {
                    // Possible string end
                    state = POSSIBLE_LONG_BRACKET_STRING_END;
                    possiblechunklevel = 0;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING_END && reading_buffer[i] == '=' ) 
                {
                    // Possible new string end level
                    possiblechunklevel++;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING_END && reading_buffer[i] == ']' ) 
                {
                    // We have a new string end level. We need to check if it is the end of the string though
                    DEBUGLOG( "POSSIBLE_LONG_BRACKET_STRING_END: %d == %d", possiblechunklevel, chunklevel );
                    if ( possiblechunklevel == chunklevel ) 
                    {
                        state = CHUNK;                        
                    }
                    else 
                    {
                        state = LONG_BRACKET_STRING;
                        possiblechunklevel = 0;
                    }
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == SINGLE_QUOTED_STRING && reading_buffer[i] == '\\' && reading_buffer[i+1] == '\'' ) 
                {
                    // No string end 
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i+1] );
                    i += 2;
                }
                else if ( state == SINGLE_QUOTED_STRING && reading_buffer[i] == '\'' ) 
                {
                    // Single quoted string ends here
                    state = CHUNK;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == DOUBLE_QUOTED_STRING && reading_buffer[i] == '\\' && reading_buffer[i+1] == '"' ) 
                {
                    // No string end 
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i+1] );
                    i += 2;
                }
                else if ( state == DOUBLE_QUOTED_STRING && reading_buffer[i] == '"' ) 
                {
                    // Double quoted string ends here
                    state = CHUNK;
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                else
                {
                    // Nothing special just copy
                    DYNAMIC_STRING_ADD_CHAR( working_buffer, reading_buffer[i] );
                    i++;
                }
                DEBUGLOG( "State: %d", state );
            }
                    
            if ( html_buffer != NULL )
            {
                int j = 0;
                DYNAMIC_STRING_ADD( working_buffer, "io.write([" );
                for( j = 0 ; j <= htmllevel; j++ ) 
                {
                    DYNAMIC_STRING_ADD( working_buffer, "=" );
                }
                DYNAMIC_STRING_ADD( working_buffer, "[\n" );
                DYNAMIC_STRING_ADD( working_buffer, html_buffer );
                DYNAMIC_STRING_ADD( working_buffer, "]" );
                for( j = 0 ; j <= htmllevel; j++ ) 
                {
                    DYNAMIC_STRING_ADD( working_buffer, "=" );
                }
                DYNAMIC_STRING_ADD( working_buffer, "]);\n" );                    
            }
            free( html_buffer );
        }
        free( reading_buffer );
        DEBUGLOG( "%s", working_buffer );
    }
}

int main( int argc, char** argv )
{    
    const char* name    = "LUCIE - Lua common internet environment";
    const char* author  = "Jakob Westhoff <jakob@westhoffswelt.de>";
    const char* version = "Version 0.1";

    lua_State *L;

    // Check for commandline parameters ( We need at least the script filename
    // to execute )
    if ( argc < 2 ) 
    {
        printf( "%s %s\nCopyright %s\n", name, version, author );
        printf( "\nUsage: %s <lucie scriptfile>\n", basename( argv[0] ) );
        printf( "\nInformation:\n" );
        printf( "    This interpreter should be called as a cgi executable from\n    inside the webserver.\n" );
        printf( "\n    The configurationfile position can be specified by setting\n    the LUCIE_CONFIG_FILE environment variable.\n" );
        printf( "\n" );

        exit( EXIT_FAILURE );
    }

    // Init the lua engine
    DEBUGLOG( "Initializing lua state" );
    L = luaL_newstate();
    DEBUGLOG( "New state created L=0x%x", (int)L );
    luaL_openlibs( L ); 
    DEBUGLOG( "Lua lib opened" );

    DEBUGLOG( "Registering extensions" );
    register_extensions( L );
    DEBUGLOG( "All extensions registered" );

    DEBUGLOG( "Converting cgi environment to superglobals" );
    init_superglobals( L );

    {
        FILE* f = fopen( argv[1], "r" );
        size_t size;
        lucie_reader( L, f, &size );
    }

    // Load the script for execution
    DEBUGLOG( "Trying to load lua file: %s", argv[1] );
    LUACHECK( luaL_loadfile( L, argv[1] ) );

    // Execute script
    DEBUGLOG( "Executing loaded script" );
//    LUACHECK( lua_pcall( L, 0, LUA_MULTRET, 0 ) );

    // Cleanup memory from registered extensions
    cleanup_registered_extensions();

    // Destroy the lua environment
    DEBUGLOG( "Closing lua" );
    lua_close( L );

    return 0;
}
