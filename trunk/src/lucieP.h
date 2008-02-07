#ifndef LUCIEP_H
#define LUCIEP_H

extern char **environ;

int extension_count;
extension_t **extensions;

char errorstring[4096];

#define DYNAMIC_STRING_INIT(ds) \
    if ( ds != NULL ) \
    { \
        free( ds ); \
    } \
    ds = NULL;

#define DYNAMIC_STRING_ADD(ds, data) \
    { \
        DEBUGLOG( "STRING_ADD( %s, \"%s\" )", #ds, data ); \
        int datalen = strlen( data ); \
        if ( ds != NULL ) \
        { \
            int len = strlen( ds ); \
            ds = (char*)srealloc( ds, sizeof( char ) * ( len + datalen + 1 ) ); \
            memset( ds + len, 0, datalen + 1 ); \
            memcpy( ds + len, (void*)data, datalen ); \
        } \
        else \
        { \
            ds = (char*)smalloc( sizeof( char ) * ( datalen + 1 ) ); \
            memset( ds, 0, datalen + 1 ); \
            memcpy( ds, (void*)data, datalen ); \
        } \
    }

#define DYNAMIC_STRING_ADD_CHAR(ds, data) \
    { \
        DEBUGLOG( "STRING_ADD_CHAR( %s, \"%c\" )", #ds, data ); \
        int datalen = 1; \
        if ( ds != NULL ) \
        { \
            int len = strlen( ds ); \
            DEBUGLOG( "REALLOC" ); \
            ds = (char*)srealloc( ds, sizeof( char ) * ( len + datalen + 1 ) ); \
            memset( ds + len, 0, datalen + 1 ); \
            ds[len] = data; \
        } \
        else \
        { \
            DEBUGLOG( "ALLOC" ); \
            ds = (char*)smalloc( sizeof( char ) * ( datalen + 1 ) ); \
            DEBUGLOG( "MEMSET" ); \
            memset( ds, 0, datalen + 1 ); \
            ds[0] = data; \
        } \
    }

#endif
