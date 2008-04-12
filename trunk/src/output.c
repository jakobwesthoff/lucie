#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lucie.h"
#include "output.h"

struct httpheader 
{
    char* key;
    char* value;
    struct httpheader* next;
};

struct httpheader* httpheader;
int headersent;

int L_f_write( lua_State *L );
int L_io_write( lua_State *L );
int L_print( lua_State *L );
int L_header( lua_State *L );

//
// This part has been copied from the lua 5.1.3 source code
//

#define IO_INPUT    1
#define IO_OUTPUT   2
#define tofilep(L)	((FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE))

static const char *const fnames[] = {"input", "output"};
static int pushresult (lua_State *L, int i, const char *filename);
static FILE *tofile (lua_State *L);
static FILE *getiofile (lua_State *L, int findex);
static int g_write (lua_State *L, FILE *f, int arg);
static int luaB_print (lua_State *L);

static int pushresult (lua_State *L, int i, const char *filename) {
  int en = errno;  /* calls to Lua API may change this value */
  if (i) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    if (filename)
      lua_pushfstring(L, "%s: %s", filename, strerror(en));
    else
      lua_pushfstring(L, "%s", strerror(en));
    lua_pushinteger(L, en);
    return 3;
  }
}

static FILE *tofile (lua_State *L) {
  FILE **f = tofilep(L);
  if (*f == NULL)
    luaL_error(L, "attempt to use a closed file");
  return *f;
}

static FILE *getiofile (lua_State *L, int findex) {
  FILE *f;
  lua_rawgeti(L, LUA_ENVIRONINDEX, findex);
  f = *(FILE **)lua_touserdata(L, -1); 
  if (f == NULL)
    luaL_error(L, "standard %s file is closed", fnames[findex - 1]); 
  return f;
}

static int g_write (lua_State *L, FILE *f, int arg) {
  int nargs = lua_gettop(L) - 1;
  int status = 1;
  for (; nargs--; arg++) {
    if (lua_type(L, arg) == LUA_TNUMBER) {
      /* optimization: could be done exactly as for strings */
      status = status &&
          fprintf(f, LUA_NUMBER_FMT, lua_tonumber(L, arg)) > 0;
    }
    else {
      size_t l;
      const char *s = luaL_checklstring(L, arg, &l);
      status = status && (fwrite(s, sizeof(char), l, f) == l);
    }
  }
  return pushresult(L, status, NULL);
}

static int luaB_print (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  lua_getglobal(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);  /* get result */
    if (s == NULL)
      return luaL_error(L, LUA_QL("tostring") " must return a string to "
                           LUA_QL("print"));
    if (i>1) fputs("\t", stdout);
    fputs(s, stdout);
    lua_pop(L, 1);  /* pop result */
  }
  fputs("\n", stdout);
  return 0;
}



//
// These are the needed overrides using the lua functions
//

void init_output_override( lua_State *L ) 
{    
    DEBUGLOG( "Installing file:write override" );    
    luaL_getmetatable( L, LUA_FILEHANDLE );
    lua_getfield( L, -1, "write" );
    lua_pushcfunction( L, L_f_write );
    // We need to transfer the old function environment to the new one
    lua_getfenv( L, -2 );
    lua_setfenv( L, -2 );
    lua_setfield( L, -3, "write" );

    DEBUGLOG( "Installing io.write override" );
    lua_getglobal( L, "io" );
    lua_getfield( L, -1, "write" );
    lua_pushcfunction( L, L_io_write );
    // We need to transfer the old function environment to the new one
    lua_getfenv( L, -2 );
    lua_setfenv( L, -2 );
    lua_setfield( L, -3, "write" );

    DEBUGLOG( "Installing print override" );
    lua_pushcfunction( L, L_print );
    lua_setglobal( L, "print" );

    // Init the httpheader struct with a default content-type
    httpheader = ( struct httpheader* )smalloc( sizeof( struct httpheader ) );
    headersent = false;
    httpheader->key = strdup( "Content-Type" );
    httpheader->value = strdup( "text/html" );
    httpheader->next = NULL;

    // Register the header function
    lua_pushcfunction( L, L_header );
    lua_setglobal( L, "header" );
}

int L_f_write( lua_State *L ) 
{
    FILE* f = tofile( L );
    if ( f == stdout ) 
    {
        header_output();   
    }
    // Let lua handle the output procedure
    return g_write( L, f, 2 ); 
}

int L_io_write( lua_State *L )
{
    FILE* f = getiofile( L, IO_OUTPUT );
    if ( f == stdout ) 
    {
        header_output();   
    }
    return g_write( L, f, 1 );
}

int L_print( lua_State *L ) 
{
    header_output();   
    return luaB_print( L );
}

//
// Handle the header output and cleanup
//

int L_header( lua_State *L ) 
{
    struct httpheader* cur;
    struct httpheader* before = NULL;
    const char* key   = luaL_checkstring( L, 1 );
    const char* value = luaL_checkstring( L, 2 );

    for( cur = httpheader; cur; before = cur, cur = cur->next ) 
    {
        if ( !strcmp( cur->key, key ) ) 
        {
            // We already have a header with this key just update its value
            free( cur->value );
            cur->value = strdup( value );
            return 0;
        }
    }
    
    // Add the new header to the end of the linked list
    before->next = ( struct httpheader* )smalloc( sizeof( struct httpheader ) );
    cur = before->next;
    cur->key = strdup( key );
    cur->value = strdup( value );
    cur->next = NULL;

    return 0;
}

void cleanup_headerdata() 
{
    struct httpheader* cur = httpheader;   
    while ( cur )
    {
        DEBUGLOG( "Freeing header entry: %s: %s", cur->key, cur->value );
        // store the next item pointer before freeing the current one
        struct httpheader* tmp = cur->next;
        free( cur->key );
        free( cur->value );
        free( cur );
        cur = tmp;
    }
}

void header_output()
{
    struct httpheader* cur;

    if ( headersent == true ) 
    {
        return;
    }

    for( cur = httpheader; cur; cur = cur->next ) 
    {
        printf( "%s: %s\n", cur->key, cur->value );
    }
    printf( "\n" );
    
    headersent = true;    
}

