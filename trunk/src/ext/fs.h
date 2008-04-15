#ifdef FS_H
#define FS_H

enum statquestion { 
    QUESTION_EXISTS = 1,
    QUESTION_FILE,
    QUESTION_DIR,
    QUESTION_LINK,
    QUESTION_READABLE,
    QUESTION_WRITABLE,
    QUESTION_EXECUTABLE,
    QUESTION_MTIME,
    QUESTION_ATIME,
    QUESTION_CTIME,
    QUESTION_OWNER,
    QUESTION_GROUP,
    QUESTION_SIZE
};

int stat_file( lua_State *L, enum statquestion question );
int L_basename( lua_State *L );
int L_dirname( lua_State *L );
int L_file_exists( lua_State *L );
int L_is_file( lua_State *L );
int L_is_dir( lua_State *L );
int L_is_readable( lua_State *L );
int L_is_writable( lua_State *L );
int L_is_executable( lua_State *L );
int L_is_link( lua_State *L );
int L_file_mtime( lua_State *L );
int L_file_atime( lua_State *L );
int L_file_ctime( lua_State *L );
int L_file_owner( lua_State *L );
int L_file_group( lua_State *L );
int L_file_size( lua_State *L );
void register_extension( lua_State *L );

#endif
