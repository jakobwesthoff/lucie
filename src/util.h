#ifndef UTIL_H
#define UTIL_H

void* smalloc( size_t bytes );
void* srealloc( void* old, size_t bytes );
int file_exists( const char* filename ); 

#endif
