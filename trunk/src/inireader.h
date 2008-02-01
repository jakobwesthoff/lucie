#ifndef INIREADER_H
#define INIREADER_H

typedef struct inireader_entry_struct
{
    char* identifier;
    char* data;
    char* key;
    char* group;
    struct inireader_entry_struct* next;
} inireader_entry_t;

typedef struct 
{
    FILE* handle;
    inireader_entry_t* first;
} inifile_t;

inifile_t* inireader_open( const char* filename );
int inireader_close( inifile_t* inifile );

#endif
