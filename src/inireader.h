#ifndef INIREADER_H
#define INIREADER_H

typedef struct inireader_entry_struct
{
    char* identifier;
    char* data;
    char* key;
    char* group;
    struct inireader_entry_struct* next;
} inireader_entry_t, inireader_iterator_t;

typedef struct 
{
    FILE* handle;
    inireader_entry_t* first;
} inifile_t;

inifile_t* inireader_open( const char* filename );
int inireader_close( inifile_t* inifile );
inireader_iterator_t* inireader_get_iterator( inifile_t* inifile, char* group, char* identifier, char* key, char* data );
void inireader_reset( inifile_t* inifile, inireader_iterator_t* iter );
inireader_entry_t* inireader_iterate( inireader_iterator_t* iter );
void inireader_destroy_iterator( inireader_iterator_t* iter );

#endif
