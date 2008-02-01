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

inireader_iterator_t* inireader_get_iterator( inifile_t* inifile, char* group, char* identifier, char* key, char* data ) 
{
    inireader_iterator_t* iter = ( inireader_iterator_t* )smalloc( sizeof( inireader_iterator_t ) );
    iter->group = group;
    iter->identifier = identifier;
    iter->key = key;
    iter->data = data;
    iter->next = inifile->first;

    return iter;
}

void inireader_reset( inifile_t* inifile, inireader_iterator_t* iter ) 
{
    iter->next = inifile->first;
}

inireader_entry_t* inireader_iterate( inireader_iterator_t* iter ) 
{
    inireader_entry_t* current = iter->next;
    while( current != NULL ) 
    {
        if (
                ( ( iter->group == NULL ) || !strcasecmp( current->group, iter->group ) )
             && ( ( iter->identifier == NULL ) || !strcmp( current->identifier, iter->identifier ) )
             && ( ( iter->key == NULL ) || !strcmp( current->key, iter->key ) )
             && ( ( iter->data == NULL ) || !strcmp( current->data, iter->data ) )
           )
        {
            iter->next = current->next;
            return current;
        }
        current = current->next;
    }
    iter->next = NULL;
    return NULL;
}

void inireader_destroy_iterator( inireader_iterator_t* iter ) 
{
    free( iter );
}

void inireader_free_inifile( inifile_t* inifile ) 
{
    inireader_entry_t* current;    
    for( current = inifile->first; current != NULL; current = current->next ) 
    {
        free( current->identifier );
        free( current->data );
        free( current->key );
        free( current->group );
    }
    free( inifile );
}

int inireader_parse( inifile_t* inifile )
{
    enum { LINE_START = 1, IDENTIFIER, KEY, BEFORE_EQUALSIGN, AFTER_EQUALSIGN, DATA, GROUP, IGNORE_LINE } state = 1;
    int c;
    char* group = NULL;
    int len = 0;
    int line = 1;
    int character = 1;
    inireader_entry_t* current = NULL;
    
    // Seeking to the beginning of the file
    fseek( inifile->handle, 0, SEEK_SET );

    // Read character by character
    for( c = fgetc( inifile->handle ); !feof( inifile->handle ); c = fgetc( inifile->handle ), character++ ) 
    {   
        if ( ( state == IDENTIFIER ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) ) ) 
        {
            // Allocate or reallocate the identifier string
            if ( current->identifier == NULL ) 
            {
                DEBUGLOG( "Initial allocation for identifier" );
                current->identifier = ( char* )smalloc( 16 * sizeof( char ) );                
                memset( current->identifier, 0, 16 );
                len = 0;
            }
            else if ( ( len + 1 ) % 16 == 0 ) 
            {
                DEBUGLOG( "Reallocation for identifier" );
                current->identifier = ( char* )srealloc( current->identifier, ( len + 16 + 1 ) * sizeof( char ) );
                memset( current->identifier + len + 1, 0, 16 );
            }
            DEBUGLOG( "Adding character to identifier at pos %d", len );
            current->identifier[len++] = c;
        }
        else if ( ( state == IDENTIFIER ) && ( c == '[' ) ) 
        {
            state = KEY;
        }
        else if ( ( ( state == IDENTIFIER ) || ( state == KEY ) || ( state == BEFORE_EQUALSIGN ) ) && ( c == '=' ) ) 
        {
            // Copy the group string or set it to null. We are doing this at
            // the equal sign, because this token is only read once in this
            // state
            if ( group != NULL ) 
            {
                DEBUGLOG( "Setting group: %s", group );
                current->group = strdup( group );
            }

            state = AFTER_EQUALSIGN;
        }
        else if ( ( state == KEY ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) ) ) 
        {
            if ( current->key == NULL ) 
            {
                DEBUGLOG( "Initial allocation for key" );
                current->key = ( char* )smalloc( 16 * sizeof( char ) );                
                memset( current->key, 0, 16 );
                len = 0;
            }
            else if ( ( len + 1 ) % 16 == 0 ) 
            {
                DEBUGLOG( "Reallocation for key" );
                current->key = ( char* )srealloc( current->key, ( len + 16 + 1 ) * sizeof( char ) );
                memset( current->key + len + 1, 0, 16 );
            }
            DEBUGLOG( "Adding character to key at pos %d", len );
            current->key[len++] = c;
        }
        else if ( ( ( state == KEY ) && ( c == ']' ) ) || ( ( state == IDENTIFIER ) && ( ( c == ' ' ) || ( c == '\t' ) ) ) ) 
        {
            state = BEFORE_EQUALSIGN;
        }
        else if ( ( ( state == LINE_START ) || ( state == BEFORE_EQUALSIGN ) || ( state == AFTER_EQUALSIGN ) ) && ( ( c == ' ' ) || c == '\t' ) ) 
        {
            // Just eat the whitespaces ;)
        }
        else if ( ( state == AFTER_EQUALSIGN ) && ( ( c != ' ' ) && ( c != '\t' ) ) ) 
        {
            ungetc( c, inifile->handle );
            state = DATA;
        }
        else if ( ( state == LINE_START ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) ) ) 
        {
            // Allocate memory for the new entry
            inireader_entry_t* next = ( inireader_entry_t* )smalloc( sizeof( inireader_entry_t ) );
            next->identifier = NULL;
            next->data       = NULL;
            next->key        = NULL;
            next->group      = NULL;
            
            // Maybe this is our first entry, therefore it does not have any parents
            if ( inifile->first == NULL ) 
            {
                inifile->first = next;
            }
            // Just add a new element to the list
            else 
            {
                current->next = next;
            }
            current = next;

            state = IDENTIFIER;
            ungetc( c, inifile->handle );
        }
        else if ( ( state == LINE_START ) && ( c == '[' ) ) 
        {
            // Free the old group string
            free( group );
            group = NULL;

            state = GROUP;
        }
        else if ( ( state == GROUP ) && ( c == ']' ) ) 
        {
            state = IGNORE_LINE;
        }
        else if ( ( state == GROUP ) && ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) ) ) 
        {
            if ( group == NULL ) 
            {
                DEBUGLOG( "Initial allocation for group" );
                group = ( char* )smalloc( 16 * sizeof( char ) );                
                memset( group, 0, 16 );
                len = 0;
            }
            else if ( ( len + 1 ) % 16 == 0 ) 
            {
                DEBUGLOG( "Reallocation for group" );
                group = ( char* )srealloc( group, ( len + 16 + 1 ) * sizeof( char ) );
                memset( group + len + 1, 0, 16 );
            }
            DEBUGLOG( "Adding character to group at pos %d", len );
            group[len++] = c;
        }
        else if ( ( state == IGNORE_LINE ) && ( ( c != '\r' ) && ( c != '\n' ) ) ) 
        {
            // Just ignore the character
        }
        else if ( ( state == LINE_START ) && ( ( c == ';' ) || ( c == '#' ) ) ) 
        {
            state = IGNORE_LINE;
        }
        else if ( ( state == DATA ) && ( ( c != '\r' ) && ( c != '\n' ) ) ) 
        {
            if ( current->data == NULL ) 
            {
                DEBUGLOG( "Initial allocation for data" );
                current->data = ( char* )smalloc( 32 * sizeof( char ) );                
                memset( current->data, 0, 32 );
                len = 0;
            }
            else if ( ( len + 1 ) % 32 == 0 ) 
            {
                DEBUGLOG( "Reallocation for data" );
                current->data = ( char* )srealloc( current->data, ( len + 1 + 32 ) * sizeof( char ) );
                memset( current->data + len + 1, 0, 32 );
            }
            DEBUGLOG( "Adding character to data at pos %d", len );
            current->data[len++] = c;
        }
        else if ( ( c == '\n' ) || ( c == '\r' ) ) 
        {
            int la = fgetc( inifile->handle );
            if ( la != '\n' ) 
            {
                ungetc( la, inifile->handle );
            }

            character = 0;
            line++;
            state = LINE_START;
        }
        else 
        {
            THROW_ERROR( "Unexpected character in line %d, at character position %d.",line, character );
            return 0;
        }

        DEBUGLOG( "Line: %d, Character: %d, Read character: '%c', State: %d, len: %d", line, character, c, state, len );
    }

    // Free the group it is not needed any longer
    if ( group != NULL ) 
    {
        free( group );
    }
    
    return true;
}

