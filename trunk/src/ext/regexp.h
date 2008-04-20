#ifndef EXT_REGEXP_H
#define EXT_REGEXP_H

#define REGEXP_REGEX_T "regex_t*"

void register_extension( lua_State *L ); 
int L_compile( lua_State *L );
int regexp_exec( lua_State *L );
int regexp_gc( lua_State *L );

#endif
