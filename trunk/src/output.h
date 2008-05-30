#ifndef OUTPUT_H
#define OUTPUT_H

void init_output_override( lua_State *L );
int L_f_write( lua_State *L );
int L_io_write( lua_State *L );
int L_print( lua_State *L );
void cleanup_headerdata();
void header_output();
int L_ob_start( lua_State *L ); 
int L_ob_end( lua_State *L );
void appendToOutputBuffer( const char* data, int len );

#endif
