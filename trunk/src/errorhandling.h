#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#define LUACHECK(x...) \
    {                                                                                                     \
        int error = x;                                                                                    \
        if ( error )                                                                                      \
        {                                                                                                 \
             fprintf( stderr, "%s <%d>: %s\n\t%s\n", __FILE__, __LINE__, #x, lua_tostring( L, -1 ) );     \
             lua_pop( L, 1 );                                                                             \
             exit( EXIT_FAILURE );                                                                        \
        }                                                                                                 \
    }

#endif
