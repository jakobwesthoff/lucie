#include <stdlib.h>
#include <stdio.h>

#include "lucie.h"

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

