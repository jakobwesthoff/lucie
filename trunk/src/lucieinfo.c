#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "lucie.h"
#include "lucieinfo.h"

void lucieinfo_header() 
{
    //
    // Html header and stylesheet
    // 
    printf( " \
<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \n \
    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"> \n \
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\"> \n \
<head> \n \
	<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /> \n \
 \n \
	<meta name=\"language\" content=\"en\" /> \n \
	<meta name=\"description\" content=\"Lucie information page\" /> \n \
	<meta name=\"author\" content=\"Jakob Westhoff\" /> \n \
	 \n \
	<style> \n \
        body { \n \
            margin: 0; \n \
            font-family: verdana, sans-serif;         \n \
            font-size: 14px; \n \
            width: 100%%; \n \
        } \n \
 \n \
         \n \
        h1, h2, h3, table { \n \
            margin: 0 auto; \n \
            width: 600px; \n \
        } \n \
 \n \
        h2, h3 { \n \
            text-align: center; \n \
            margin-bottom: 10px; \n \
            margin-top: 20px; \n \
        } \n \
         \n \
        table.title { \n \
            margin-bottom: 20px; \n \
            margin-top: 20px; \n \
        } \n \
 \n \
        table.title td { \n \
            border-right: 0; \n \
            padding: 10px; \n \
            font-size: 24px; \n \
            height: 100px; \n \
        } \n \
 \n \
        table.title td.logo { \n \
            border: 0; \n \
            background: #fcaf3e url('lucie.png') left center no-repeat; \n \
            width: 80px; \n \
        } \n \
         \n \
        table { \n \
            border-collapse: collapse; \n \
            margin-bottom: 10px; \n \
        }         \n \
 \n \
        table, td { \n \
            border: 1px solid #2d1300;             \n \
            background: #eeeeec; \n \
        } \n \
 \n \
        td { \n \
            padding: 2px 3px 2px 5px; \n \
        } \n \
         \n \
        thead td { \n \
            font-weight: bold; \n \
            background: #fcaf3e; \n \
        } \n \
         \n \
 \n \
        td.key { \n \
            font-weight: bold; \n \
            background: #fed69c; \n \
        } \n \
 \n \
        td.value { \n \
            background: #eeeeec; \n \
        } \n \
		 \n \
	</style> \n \
 \n \
	<title>Lucie information page</title> \n \
</head> \n \
<body> \n \
     \n \
   " );
}

void lucieinfo_footer() 
{
    printf( " \
</body> \n \
</html> \n \
    " );
}

void lucieinfo_table_begin( char* class ) 
{
    printf( "<table class=\"%s\">\n", class );
}

void lucieinfo_table_end() 
{
    printf( "</table>\n" );
}

void lucieinfo_table_header( int cols, ... ) 
{
    va_list va;
    int i; 

    printf( "<thead><tr>\n" );

    va_start( va, cols );
    for( i = 0; i < cols; i++ ) 
    {
        char* col_class = va_arg( va, char* );
        char* col_data = va_arg( va, char* );
        printf( "<td class=\"%s\">%s</td>\n", col_class, col_data );
    }
    printf( "</tr></thead>\n" );
}

void lucieinfo_table_row( int cols, ... ) 
{
    va_list va;
    int i; 
    
    printf( "<tr>\n" );
    va_start( va, cols );
    for( i = 0; i < cols; i++ ) 
    {
        char* col_class = va_arg( va, char* );
        char* col_data = va_arg( va, char* );
        printf( "<td class=\"%s\">%s</td>\n", col_class, col_data );
    }
    printf( "</tr>\n" );
}

void lucieinfo_headline( int type, char* headline ) 
{
    printf( "<h%i>%s</h%i>\n", type, headline, type );
}


int L_lucieinfo( lua_State* L ) 
{
    lucieinfo_header();

    //
    // Title and Version number
    //
    lucieinfo_table_begin( "title" );
    lucieinfo_table_header( 2, "", APPLICATION_NAME " Version " APPLICATION_VERSION, "logo", "" );
    lucieinfo_table_end();

    // 
    // Default information
    //
    lucieinfo_table_begin( "" );
    lucieinfo_table_row( 2, "key", "Build Date", "value", __DATE__ " " __TIME__ );
    lucieinfo_table_row( 2, "key", "Configuration File", "value", config_file );
    #ifdef DEBUG
        lucieinfo_table_row( 2, "key", "Debug Build", "value", "yes" );
    #else
        lucieinfo_table_row( 2, "key", "Debug Build", "value", "no" );
    #endif
    lucieinfo_table_end();

    //
    // Loaded module info
    //
    lucieinfo_headline( 2, "Loaded Modules" );
    {
        int i,j;
        for( i=0; i<extension_count; i++ ) 
        {           
            lucieinfo_headline( 3, extensions[i]->name );
            lucieinfo_table_begin( "" );
            
            // Header with colspan=2
            printf( "\
                    <thead> \n \
                        <tr> \n \
                            <td colspan=\"2\">Author information</td> \n \
                        </tr> \n \
                    </thead> \n"
            );
            
            lucieinfo_table_row( 2, "key", "Name", "value", extensions[i]->author );
            lucieinfo_table_row( 2, "key", "EMail", "value", extensions[i]->email );
            lucieinfo_table_end();

            lucieinfo_table_begin( "" );
            lucieinfo_table_header( 1, "", "Registered functions" );
            for( j=0; j<extensions[i]->function_count; j++ ) 
            {
                lucieinfo_table_row( 1, "value", extensions[i]->functions[j] );
            }
            lucieinfo_table_end();
        }
    }


    //
    // Environmental variables
    //
    lucieinfo_headline( 2, "Environment Variables" );
    lucieinfo_table_begin( "" );
    lucieinfo_table_header( 2, "", "Variable Name", "", "Value" );
    {
        extern char** environ;
        char** env;
        for( env = environ; *env; env++ ) 
        {
            char* key = strdup( *env );
            char* value = "";
            int len = strlen( key );
            int i;

            for( i=0; i<len; i++ ) 
            {
                if ( key[i] == '=' ) 
                {
                    key[i] = 0;
                    value = key + i + 1;
                    break;
                }
            }

            // @todo: There should be a html entities encode be done here
            lucieinfo_table_row( 2, "key", key, "value", value );
            free( key );
        }    
    }
    lucieinfo_table_end();

    //
    // Superglobal variables
    //
    lucieinfo_headline( 2, "Superglobals" );
    lucieinfo_table_begin( "" );
    lucieinfo_table_header( 2, "", "Variable Name", "", "Value" );
    // _SERVER superglobal
    {
        lua_getglobal( L, "_SERVER" );
        lua_pushnil( L );
        while( lua_next( L, -2 ) != 0 ) 
        {
            // @todo: There should be a html entities encode be done here
            printf( "<tr><td class=\"key\">_SERVER[\"%s\"]</td><td class=\"value\">%s</td>\n", lua_tostring( L, -2 ), lua_tostring( L, -1  ) );
            lua_pop( L, 1 );
        }
        lua_pop( L, 1 );
    }
    // _HEADER superglobal
    {
        lua_getglobal( L, "_HEADER" );
        lua_pushnil( L );
        while( lua_next( L, -2 ) != 0 ) 
        {
            // @todo: There should be a html entities encode be done here
            printf( "<tr><td class=\"key\">_HEADER[\"%s\"]</td><td class=\"value\">%s</td>\n", lua_tostring( L, -2 ), lua_tostring( L, -1  ) );
            lua_pop( L, 1 );
        }
        lua_pop( L, 1 );
    }
    // _GET superglobal
    {
        lua_getglobal( L, "_GET" );
        lua_pushnil( L );
        while( lua_next( L, -2 ) != 0 ) 
        {
            // @todo: There should be a html entities encode be done here
            printf( "<tr><td class=\"key\">_GET[\"%s\"]</td><td class=\"value\">%s</td>\n", lua_tostring( L, -2 ), lua_tostring( L, -1  ) );
            lua_pop( L, 1 );
        }
        lua_pop( L, 1 );
    }
    // _POST superglobal
    {
        lua_getglobal( L, "_POST" );
        lua_pushnil( L );
        while( lua_next( L, -2 ) != 0 ) 
        {
            // @todo: There should be a html entities encode be done here
            printf( "<tr><td class=\"key\">_POST[\"%s\"]</td><td class=\"value\">%s</td>\n", lua_tostring( L, -2 ), lua_tostring( L, -1  ) );
            lua_pop( L, 1 );
        }
        lua_pop( L, 1 );
    }
    lucieinfo_table_end();

    return 0;

}
