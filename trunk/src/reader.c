#include <stdlib.h>
#include <stdio.h>

#include "lucie.h"

void dynamic_string_init( char** buffer ) 
{
    if ( *buffer != NULL ) 
    { 
        free( *buffer ); 
    }
    
    *buffer = NULL;
}

void dynamic_string_add( char** buffer, char* data ) 
{
    DEBUGLOG( "STRING_ADD( \"%s\" )", data ); 
    int datalen = strlen( data ); 
    if ( *buffer != NULL ) 
    { 
        int len = strlen( *buffer ); 
        *buffer = (char*)srealloc( *buffer, sizeof( char ) * ( len + datalen + 1 ) ); 
        memset( (*buffer) + len, 0, datalen + 1 ); 
        memcpy( (*buffer) + len, (void*)data, datalen ); 
    } 
    else 
    { 
        *buffer = (char*)smalloc( sizeof( char ) * ( datalen + 1 ) ); 
        memset( *buffer, 0, datalen + 1 ); 
        memcpy( *buffer, (void*)data, datalen ); 
    } 
}

void dynamic_string_add_char( char** buffer, char data ) 
{
    DEBUGLOG( "STRING_ADD_CHAR( \"%c\" )", data ); 
    int datalen = 1; 
    if ( *buffer != NULL ) 
    { 
        int len = strlen( *buffer ); 
        *buffer = (char*)srealloc( *buffer, sizeof( char ) * ( len + datalen + 1 ) ); 
        memset( (*buffer) + len, 0, datalen + 1 ); 
        (*buffer)[len] = data; 
    } 
    else 
    { 
        *buffer = (char*)smalloc( sizeof( char ) * ( datalen + 1 ) ); 
        memset( *buffer, 0, datalen + 1 ); 
        (*buffer)[0] = data; 
    } 
}

const char* lucie_reader( lua_State* L, void* data, size_t* size ) 
{
    static char* output_buffer;    

    // Check if we have already read the whole file.
    if ( feof( (FILE*)data ) ) 
    {
        // Free the buffer and return null
        DEBUGLOG( "END OF FILE" );
        free( output_buffer );
        *size = 0;
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
        fread( reading_buffer, sizeof( char ), filesize + 1, (FILE*)data );
        {
            // We need to remember certain values during the processing
            enum { INIT = 1,
                   SHEBANG = 2,
                   HTML = 3,
                   HTML_LONG_BRACKET = 4,
                   CHUNK = 5,
                   LINE_COMMENT = 6,
                   POSSIBLE_COMMENT = 7,
                   COMMENT = 8,
                   POSSIBLE_COMMENT_END = 9,
                   LONG_BRACKET_STRING = 10,
                   POSSIBLE_LONG_BRACKET_STRING = 11,
                   POSSIBLE_LONG_BRACKET_STRING_END = 12,                 
                   SINGLE_QUOTED_STRING = 13,
                   DOUBLE_QUOTED_STRING = 14,
                   LONG_BRACKET = 15
            } state = 1;
            
            int htmllevel          = 0;
            int possiblehtmllevel  = 0;
            int chunklevel         = 0;
            int possiblechunklevel = 0;
            char* html_buffer = NULL;
            int i = 0;

            dynamic_string_init( &html_buffer );
            dynamic_string_init( &working_buffer );

            while( i < filesize ) 
            {
                if ( state == INIT && reading_buffer[i] == '#' ) 
                {
                    state = SHEBANG;
                } 
                else if ( state == INIT ) 
                {
                    state = HTML;
                }
                else if ( state == SHEBANG && ( reading_buffer[i] == '\r' && reading_buffer[i+1] == '\n' ) ) 
                {
                    state = HTML;
                    i += 2;
                }
                else if ( state == SHEBANG && ( reading_buffer[i] == '\n' ) ) 
                {
                    state = HTML;
                    i++;
                }
                else if ( state == SHEBANG ) 
                {
                    i++;
                }
                else if ( state == HTML && reading_buffer[i] == '<' && reading_buffer[i+1] == '?' && reading_buffer[i+2] == 'l' && reading_buffer[i+3] == 'u' && reading_buffer[i+4] == 'c' && reading_buffer[i+5] == 'i' && reading_buffer[i+6] == 'e' ) 
                {                                        
                    // Lucie starttag found
                    if ( html_buffer != NULL ) 
                    {
                        int j = 0;
                        dynamic_string_add( &working_buffer, "io.stdout:write([" );
                        for( j = 0 ; j <= htmllevel; j++ ) 
                        {
                            dynamic_string_add( &working_buffer, "=" );
                        }
                        dynamic_string_add( &working_buffer, "[\n" );
                        dynamic_string_add( &working_buffer, html_buffer );
                        dynamic_string_add( &working_buffer, "]" );
                        for( j = 0 ; j <= htmllevel; j++ ) 
                        {
                            dynamic_string_add( &working_buffer, "=" );
                        }
                        dynamic_string_add( &working_buffer, "]);\n" );                    
                        dynamic_string_init( &html_buffer );
                    }

                    state = CHUNK;
                    i += 7;
                }
                else if ( state == HTML && reading_buffer[i] == ']' && reading_buffer[i+1] == '=' ) 
                {
                    // Possible HTML_LONG_BRACKET
                    state = HTML_LONG_BRACKET;
                    possiblehtmllevel = 0;                    
                    dynamic_string_add_char( &html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == HTML ) 
                {
                    // HTML content
                    // Maybe we are at the last char and it is a newline. In this case it is ignored
                    if ( i < filesize - 1 || reading_buffer[i] != '\n' ) 
                    {
                        dynamic_string_add_char( &html_buffer, reading_buffer[i] );                    
                    }
                    i++;
                }
                else if ( state == HTML_LONG_BRACKET && reading_buffer[i] == '=' ) 
                {
                    // Possible next long bracket level
                    possiblehtmllevel++;
                    dynamic_string_add_char( &html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == HTML_LONG_BRACKET && reading_buffer[i] == ']' ) 
                {
                    // possible long bracket is new bracket level
                    state = HTML;
                    htmllevel = ( possiblehtmllevel > htmllevel ) ? possiblehtmllevel : htmllevel;
                    dynamic_string_add_char( &html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == HTML_LONG_BRACKET ) 
                {
                    // We were wrong there was no long bracket
                    state = HTML;
                    dynamic_string_add_char( &html_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == CHUNK && reading_buffer[i] == '?' && reading_buffer[i+1] == '>' ) 
                {
                    // Lucie endtag
                    state = HTML;
                    dynamic_string_init( &html_buffer );
                    htmllevel = 0;
                    i += 2;
                }
                else if ( state == CHUNK && reading_buffer[i] == '-' && reading_buffer[i+1] == '-' && reading_buffer[i+2] == '[' ) 
                {
                    // Possible comment area
                    state = POSSIBLE_COMMENT;
                    dynamic_string_add( &working_buffer, "--[" );
                    i += 3;
                }
                else if ( state == CHUNK && reading_buffer[i] == '-' && reading_buffer[i+1] == '-' ) 
                {
                    // Line comment
                    state = LINE_COMMENT;
                    dynamic_string_add( &working_buffer, "--" );
                    i += 2;
                }
                else if ( state == CHUNK && reading_buffer[i] == '[' && reading_buffer[i+1] == '[' ) 
                {
                    // Zero level long bracket found
                    state = LONG_BRACKET_STRING;
                    chunklevel = 0;
                    dynamic_string_add( &working_buffer, "[[" );
                    i += 2;
                }
                else if ( state == CHUNK && reading_buffer[i] == '[' && reading_buffer[i+1] == '=' ) 
                {                    
                    // Possible long bracket
                    state = POSSIBLE_LONG_BRACKET_STRING;
                    possiblechunklevel = 0;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING && reading_buffer[i] == '=' ) 
                {
                    // Possible new string level
                    possiblechunklevel++;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING && reading_buffer[i] == '[' ) 
                {
                    // We have a new string level.
                    state = LONG_BRACKET_STRING;
                    chunklevel = possiblechunklevel;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING ) 
                {
                    // We have a new string level.
                    state = CHUNK;
                    possiblechunklevel = 0;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == CHUNK && reading_buffer[i] == '"' ) 
                {
                    // Double quoted string
                    state = DOUBLE_QUOTED_STRING;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == CHUNK && reading_buffer[i] == '\'' ) 
                {
                    // Single quoted string
                    state = SINGLE_QUOTED_STRING;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT && reading_buffer[i] == '=' ) 
                {
                    // Possible new comment level
                    possiblechunklevel++;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT && reading_buffer[i] == ']' ) 
                {
                    // New comment level is affirmative
                    state = COMMENT;
                    chunklevel = possiblechunklevel;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT && ( reading_buffer[i] == '\n' || ( reading_buffer[i] == '\r' && reading_buffer[i+1] == '\n' ) ) )
                {
                    // It was just a line comment and is ended now
                    state = CHUNK;
                    dynamic_string_add( &working_buffer, "\n" );
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT ) 
                {
                    // We were wrong about the comment level
                    // We are just inside a simple line comment
                    state = LINE_COMMENT;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );                    
                    i++;
                }
                else if ( state == LINE_COMMENT && ( reading_buffer[i] == '\n' || ( reading_buffer[i] == '\r' && reading_buffer[i+1] == '\n' ) ) )
                {
                    // Line comment end
                    state = CHUNK;
                    dynamic_string_add( &working_buffer, "\n" );
                    i++;
                }
                else if ( state == COMMENT && reading_buffer[i] == ']' ) 
                {
                    // Possible comment end
                    state = POSSIBLE_COMMENT_END;
                    possiblechunklevel = 0;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == POSSIBLE_COMMENT_END && reading_buffer[i] == '=' ) 
                {
                    // Possible new comment end level
                    possiblechunklevel++;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
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
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == LONG_BRACKET_STRING && reading_buffer[i] == ']' ) 
                {
                    // Possible string end
                    state = POSSIBLE_LONG_BRACKET_STRING_END;
                    possiblechunklevel = 0;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == POSSIBLE_LONG_BRACKET_STRING_END && reading_buffer[i] == '=' ) 
                {
                    // Possible new string end level
                    possiblechunklevel++;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
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
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == SINGLE_QUOTED_STRING && reading_buffer[i] == '\\' && reading_buffer[i+1] == '\'' ) 
                {
                    // No string end 
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    dynamic_string_add_char( &working_buffer, reading_buffer[i+1] );
                    i += 2;
                }
                else if ( state == SINGLE_QUOTED_STRING && reading_buffer[i] == '\'' ) 
                {
                    // Single quoted string ends here
                    state = CHUNK;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    i++;
                }
                else if ( state == DOUBLE_QUOTED_STRING && reading_buffer[i] == '\\' && reading_buffer[i+1] == '"' ) 
                {
                    // No string end 
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    dynamic_string_add_char( &working_buffer, reading_buffer[i+1] );
                    i += 2;
                }
                else if ( state == DOUBLE_QUOTED_STRING && reading_buffer[i] == '"' ) 
                {
                    // Double quoted string ends here
                    state = CHUNK;
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    i++;
                }
                else
                {
                    // Nothing special just copy
                    dynamic_string_add_char( &working_buffer, reading_buffer[i] );
                    i++;
                }
                DEBUGLOG( "State: %d", state );
            }
                    
            if ( html_buffer != NULL )
            {
                int j = 0;
                dynamic_string_add( &working_buffer, "io.write([" );
                for( j = 0 ; j <= htmllevel; j++ ) 
                {
                    dynamic_string_add( &working_buffer, "=" );
                }
                dynamic_string_add( &working_buffer, "[\n" );
                dynamic_string_add( &working_buffer, html_buffer );
                dynamic_string_add( &working_buffer, "]" );
                for( j = 0 ; j <= htmllevel; j++ ) 
                {
                    dynamic_string_add( &working_buffer, "=" );
                }
                dynamic_string_add( &working_buffer, "]);\n" );                    
            }
            free( html_buffer );
        }
        free( reading_buffer );        
        output_buffer = working_buffer;
    }

    if ( output_buffer == NULL ) 
    {
        DEBUGLOG( "Empty script file detected" );
        *size = 0;
        return NULL;
    }

    *size = strlen( output_buffer );
    DEBUGLOG( "PARSED DATA: \n%s\n-- PARSED DATA", output_buffer );
    return output_buffer;
}
