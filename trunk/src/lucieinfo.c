#include <stdlib.h>
#include <stdio.h>

#include "lucie.h"
#include "lucieinfo.h"

int L_lucieinfo( lua_State* L ) 
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
    
    //
    // Title and Version number
    //
    printf( " \
        <table class=\"title\"> \n \
            <thead> \n \
                <tr> \n \
                    <td>" APPLICATION_NAME " Version " APPLICATION_VERSION "</td> \n \
                    <td class=\"logo\"></td> \n \
                </tr> \n \
            </thead> \n \
        </table> \n \
    " );

    // 
    // Default information
    //
    printf( " \
        <table> \n \
            <tr> \n \
                <td class=\"key\">Build Date</td> \n \
                <td class=\"value\">" __DATE__ " " __TIME__ "</td> \n \
            </tr> \n \
            <tr> \n \
                <td class=\"key\">Configuration File</td> \n \
                <td class=\"value\">\
" );
    printf( config_file );
    printf( "</td> \n \
            </tr> \n \
            <tr> \n \
                <td class=\"key\">Debug Build</td> \n \
                <td class=\"value\">\
" );
    #ifdef DEBUG
        printf( "yes" );
    #else
        printf( "no" );
    #endif
    printf( "</td> \n \
            </tr> \n \
        </table> \n \
    " );

    //
    // Loaded module info
    //
    printf( "<h2>Loaded Modules</h2>\n" );
    {
        int i;
        for( i=0; i<extension_count; i++ ) 
        {
            printf( "\
                <h3>%s</h3>		 \n \
                <table>		 \n \
                    <thead> \n \
                        <tr> \n \
                            <td colspan=\"2\">Author information</td> \n \
                        </tr> \n \
                    </thead> \n \
                    <tr> \n \
                        <td class=\"key\">Name</td> \n \
                        <td class=\"value\">%s</td> \n \
                    </tr> \n \
                    <tr> \n \
                        <td class=\"key\">EMail</td> \n \
                        <td class=\"value\">%s</td> \n \
                    </tr> \n \
                </table> \n \
            ", extensions[i]->name, extensions[i]->author, extensions[i]->email );
        }
    }


    //
    // Environmental variables
    //
    printf( "<h2>Environment Variables</h2>\n" );
    printf( "\
        <table> \n \
            <thead> \n \
                <tr> \n \
                    <td>Variable Name</td> \n \
                    <td>Value</td> \n \
                </tr> \n \
            </thead> \n \
    " );
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
            printf( "<tr><td class=\"key\">%s</td><td class=\"value\">%s</td>\n", key, value );
            free( key );
        }    
    }
    printf( "</table>\n" );

    //
    // Superglobal variables
    //
    printf( "<h2>Superglobals</h2>\n" );
    printf( "\
        <table> \n \
            <thead> \n \
                <tr> \n \
                    <td>Variable Name</td> \n \
                    <td>Value</td> \n \
                </tr> \n \
            </thead> \n \
    " );
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
    printf( "</table>\n" );


    //
    // Html footer
    //
    printf( " \
</body> \n \
</html> \n \
    " );

    return 0;

}
