#ifdef LUCIEINFO_H
#define LUCIEINFO_H

int L_lucieinfo( lua_State* L );
void lucieinfo_header();
void lucieinfo_footer(); 
void lucieinfo_table_begin( char* class );
void lucieinfo_table_end();
void lucieinfo_table_header( int cols, ... );
void lucieinfo_table_row( int cols, ... );
void lucieinfo_headline( int type, char* headline );

#endif
