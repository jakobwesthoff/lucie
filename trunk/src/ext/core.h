#ifndef CORE_H
#define CORE_H

void print_padded_line( int padding, const char* fmt, ... );
void var_dump( lua_State* L, int depth );
int L_var_dump( lua_State* L );
int L_urldecode( lua_State* L );
void readini_array_traverse( lua_State* L, btree_element_t* root, int arrayindex );
void readini_group_tree_traverse( lua_State* L, btree_element_t* root );
void readini_tree_traverse( lua_State* L, btree_element_t* root );
void readini_tree_free( btree_element_t* root );
int L_readini( lua_State* L );
void register_extension( lua_State *L );

#endif
