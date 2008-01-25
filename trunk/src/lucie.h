#ifndef LUCIE_H
#define LUCIE_H

#define LUACHECK(x) \
    {                                                                                                     \
        int error = x;                                                                                    \
        if ( error )                                                                                      \
        {                                                                                                 \
             fprintf( stderr, "%s <%d>: %s\n\t%s\n", __FILE__, __LINE__, #x, lua_tostring( L, -1 ) );     \
             lua_pop( L, 1 );                                                                             \
             exit( EXIT_FAILURE );                                                                        \
        }                                                                                                 \
    }

/* Debug functions are only declared if the DEBUG flag is set */
#ifdef DEBUG
    #define DEBUGLOG(fmt, ...) \
        fprintf( stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__ )
    
#else
    #define DEBUGLOG(...)
#endif

#endif
