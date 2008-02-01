#include <stdlib.h>
#include <stdio.h>

#include "lucie.h"

#include "inireader.h"

inifile_t* inireader_open( const char* filename );
int inireader_close( inifile_t* inifile );
void inireader_free_inifile( inifile_t* inifile );
int inireader_parse( inifile_t* inifile );

inifile_t* inireader_open( const char* filename ) 
{
    inifile_t* inifile = null;

/*    if ( !file_exists( filename ) ) 
    {
        THROW_ERROR( "Inifile \"%s\" does not exist.", filename );
        return NULL;
    }
*/
    DEBUGLOG( "Allocating memory for inifile structure" );
    inifile = ( inifile_t* )smalloc( sizeof( inifile_t ) );
    inifile->first   = NULL;
    inifile->current = NULL;
    
    DEBUGLOG( "Opening inifile \"%s\" for reading", filename );
    if ( ( inifile->handle = fopen( filename, "r" ) ) == NULL ) 
    {
        return NULL;
    }

    if ( inireader_parse( inifile ) == false ) 
    {
        print_error( "Error parsing inifile" );
    }

    return inifile;
}

int inireader_close( inifile_t* inifile ) 
{
    DEBUGLOG( "Closing inifile" );
    if ( fclose( inifile->handle ) == EOF ) 
    {
        return false;
    }
    
    DEBUGLOG( "Freeing inifile structure" );
    inireader_free_inifile( inifile );
    return true;
}

void inireader_free_inifile( inifile_t* inifile ) 
{
    inifile_entry_t* current;    
    for( current = inifile->first; current != NULL; current = current->next ) 
    {
        free( current->name );
        free( current->data );
        free( current->key );
    }
    free( inifile );
}

int inireader_parse( inifile_t* inifile )
{
    enum { LINE_START = 1, IDENTIFIER, KEY, AROUND_EQUALSIGN, DATA, GROUP, IGNORE_LINE } state = 1;
    int c;
    char* group;
    int len = 0;
    int line = 1;
    int character = 1;
    
    // Allocate space for the first entry
    inifile_entry_t* current = inifile->first = ( inifile_entry_t* )smalloc( sizeof( inifile_entry_t ) );
    current->name    = NULL;
    current->data    = NULL;
    current->key     = NULL;
    current->group   = NULL;
    current->isArray = false;
    
    // Seeking to the beginning of the file
    fseek( inifile->handle, 0, SEEK_SET );

    // Read character by character
    for( c = fgetc( inifile->handle ); !feof( inifile->handle ); c = fgetc( inifile->handle ), character++ ) 
    {   
        int lookahead = fgetc( inifile->handle );

        if ( ( state == IDENTIFIER ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) ) ) 
        {

        }
        else if ( ( state == IDENTIFIER ) && ( c == '[' ) ) 
        {
            current->isArray = true;
            state = KEY;
        }
        else if ( ( ( state == IDENTIFIER ) || ( state == KEY ) ) && ( c == '=' ) ) 
        {
            state = AROUND_EQUALSIGN;
        }
        else if ( ( state == KEY ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) ) ) 
        {

        }
        else if ( ( ( state == KEY ) && ( c == ']' ) ) || ( ( state == IDENTIFIER ) && ( ( c == ' ' ) || ( c == '\t' ) ) ) ) 
        {
            state = AROUND_EQUALSIGN;
        }
        else if ( ( ( state == LINE_START ) || ( state == AROUND_EQUALSIGN ) ) && ( ( c == ' ' ) || c == '\t' ) ) 
        {
            // Just eat the whitespaces ;)
        }
        else if ( ( state == AROUND_EQUALSIGN ) && ( ( c != ' ' ) && ( c != '\t' ) ) ) 
        {
            ungetc( c, inifile->handle );
            state = DATA;
        }
        else if ( ( state == LINE_START ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) ) ) 
        {
            state = IDENTIFIER;
        }
        else if ( ( state == LINE_START ) && ( c == '[' ) ) 
        {
            state = GROUP;
        }
        else if ( ( state == GROUP ) && ( c == ']' ) ) 
        {
            state = IGNORE_LINE;
        }
        else if ( ( state == GROUP ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) ) ) 
        {
            
        }
        else if ( ( state == IGNORE_LINE ) ) 
        {
            // Just ignore the character
        }
        else if ( c == '/' && lookahead == '/' ) 
        {
            lookahead = fgetc( inifile->handle );
            state = IGNORE_LINE;
        }
        else if ( ( c == '\r' ) && ( lookahead == '\n' ) ) 
        {
            lookahead = fgetc( inifile->handle );
            character = 0;
            line++;
            state = LINE_START;
        }
        else if ( ( c == '\n' ) ) 
        {
            character = 0;
            line++;
            state = LINE_START;
        }
        else 
        {
            THROW_ERROR( "Unexpected character in line %d, at character position %d.",line, character );
            return 0;
        }

        if ( lookahead != EOF ) 
        {
            ungetc( lookahead, inifile->handle );
        }
    }

    return true;
}

