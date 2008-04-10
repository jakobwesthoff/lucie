#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <libgen.h>

#include "lucie.h"
#include "inireader.h"
#include "reader.h"
#include "superglobals.h"
#include "lucieinfo.h"
#include "util.h"


const char* config_file = NULL;

int extension_count;
extension_t **extensions;

char errorstring[4096];

void cleanup_registered_extensions()
{
    int i,j;

    DEBUGLOG( "Freeing memory of %i extensions.", extension_count );
    for( i = 0; i < extension_count; i++ )
    {
        DEBUGLOG( "Freeing extension %i.", i );
        for( j=0; j<extensions[i]->function_count; j++ ) 
        {
            DEBUGLOG( "Freeing function name %i.", j );
            free( extensions[i]->functions[j] );
        }
        free( extensions[i]->functions );
        free( extensions[i] );
    }
    DEBUGLOG( "Freeing extension array." );
    free( extensions );
    DEBUGLOG( "All extension memory freed." );
}

void register_extensions( lua_State* L ) 
{
    inifile_t* inifile         = NULL;
    inireader_iterator_t* iter = NULL;
    inireader_entry_t* current = NULL;
    
    const char* lucie_config_path_env = NULL;

    // Harcoded configpaths to check
    const char* hardcoded_path[] = {
        "/etc/lucie.conf",
        "/usr/local/etc/lucie.conf",        
    };

    // First try to read the configpath from the LUCIE_CONFIG_PATH environment
    // variable
    lucie_config_path_env = getenv( "LUCIE_CONFIG_FILE" );
    if ( lucie_config_path_env != NULL ) 
    {
        if ( file_exists( lucie_config_path_env ) ) 
        {
            config_file = lucie_config_path_env;
        }
    }

    // Check the working directory for a config file next
    if ( config_file == NULL && file_exists( "./lucie.conf" ) ) 
    {
        config_file = "./lucie.conf";
    }

    // Check the hardcoded paths for the configfile
    if ( config_file == NULL ) 
    {
        int i = 0;
        for( i=0; i<2; i++ ) 
        {
            if ( file_exists( hardcoded_path[i] ) ) 
            {
                config_file = hardcoded_path[i];
                break;
            }
        }
    }

    // Exit if we can not find a configuration file
    if ( config_file == NULL ) 
    {
        THROW_ERROR( "The configuration file could not be found anywhere." );
        print_error( "Configuration file read error" );
        exit( EXIT_FAILURE );
    }

    // Open the configfile and get an iterator for the extension list
    inifile = inireader_open( config_file );
    iter = inireader_get_iterator( inifile, "extensions", "extension", 0, 0 );

    // Iterate over all of the given extensions and try to load them.
    for( current = inireader_iterate( iter ); current != NULL; current = inireader_iterate( iter ) ) 
    {
        void* so_handle = NULL;
        typedef void ( *register_extension_t )( lua_State *L );
        register_extension_t register_extension;

        DEBUGLOG( "Trying to load extension: %s", current->data );
        
        if ( !file_exists( current->data ) ) 
        {
            THROW_ERROR( "The specified extension \"%s\" does not exist.", current->data );
            print_error( "Extension loading failed" );
            exit( EXIT_FAILURE );
        }

        // Try to open the extensions shared library file
        DEBUGLOG( "Opening %s", current->data );
        if ( ( so_handle = dlopen( current->data, RTLD_LAZY ) ) == NULL ) 
        {
            THROW_ERROR( "%s", dlerror() );
            print_error( "Extension loading failed" );
            exit( EXIT_FAILURE );
        }
        DEBUGLOG( "Extension opened handle=0x%x", (int)so_handle );

        // Try to load the register extension function
        DEBUGLOG( "Trying to get symbol of the register_extension function" );
        if ( ( register_extension = ( register_extension_t )dlsym( so_handle, "register_extension" ) ) == NULL )
        {
            THROW_ERROR( "%s", dlerror() );
            print_error( "Extension loading failed" );
            exit( EXIT_FAILURE );
        }
        DEBUGLOG( "Symbol register_extension retrieved. register_extension=0x%x", (int)register_extension );

        DEBUGLOG( "Calling register_extension" );
        register_extension( L );
        DEBUGLOG( "Extension registered" );
    }
}

int main( int argc, char** argv )
{    
    lua_State* L;

    // Check for commandline parameters ( We need at least the script filename
    // to execute )
    if ( argc < 2 ) 
    {
        printf( "%s - %s Version %s\nCopyright %s\n", APPLICATION_NAME, APPLICATION_FULLNAME, APPLICATION_VERSION, APPLICATION_AUTHOR );
        printf( "\nUsage: %s <lucie scriptfile>\n", basename( argv[0] ) );
        printf( "\nInformation:\n" );
        printf( "    This interpreter should be called as a cgi executable from\n    inside the webserver.\n" );
        printf( "\n    The configurationfile position can be specified by setting\n    the LUCIE_CONFIG_FILE environment variable.\n" );
        printf( "\n" );

        exit( EXIT_FAILURE );
    }

    // Init the lua engine
    DEBUGLOG( "Initializing lua state" );
    L = luaL_newstate();
    DEBUGLOG( "New state created L=0x%x", (int)L );
    luaL_openlibs( L ); 
    DEBUGLOG( "Lua lib opened" );

    DEBUGLOG( "Registering extensions" );
    register_extensions( L );
    DEBUGLOG( "All extensions registered" );

    DEBUGLOG( "Converting cgi environment to superglobals" );
    init_superglobals( L );

    DEBUGLOG( "Registering lucieinfo function" );
    lua_pushcfunction( L, L_lucieinfo );               
    lua_setglobal( L, "lucieinfo" );

    // Load the script for execution
    {
        FILE* f = fopen( argv[1], "r" );
        DEBUGLOG( "Trying to load lua file: %s", argv[1] );
        LUACHECK( lua_load( L, lucie_reader, f, argv[1] ) );
        fclose( f );
    }

    // Execute script
    DEBUGLOG( "Executing loaded script" );
    LUACHECK( lua_pcall( L, 0, LUA_MULTRET, 0 ) );

    // Cleanup memory from registered extensions
    cleanup_registered_extensions();

    // Destroy the lua environment
    DEBUGLOG( "Closing lua" );
    lua_close( L );

    return EXIT_SUCCESS;
}
