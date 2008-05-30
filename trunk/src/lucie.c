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
#include "output.h"
#include "fs.h"


const char* config_file = NULL;

int extension_count;
extension_t **extensions;

char errorstring[4096];

extern char* outputbuffer;

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

int L_dofile( lua_State *L ) 
{
    const char* filename = luaL_checkstring( L, 1 );
    FILE* f;
    char* oldFILE;
    char*  oldDIR;
    const char* tmp;
    
    // Save old file dependend variables
    lua_getglobal( L, "__FILE__" );
    tmp = lua_tostring( L, -1 );
    if ( tmp == NULL ) 
    {
        oldFILE = NULL;
    }
    else 
    {
        oldFILE = strdup( tmp );
    }
    lua_getglobal( L, "__DIR__" );
    tmp = lua_tostring( L, -1 );
    if ( tmp == NULL ) 
    {
        oldDIR = NULL;
    }
    else 
    {
        oldDIR = strdup( tmp );
    }

    // Call out realpath function on the given filename
    lua_getfield( L, LUA_GLOBALSINDEX, "file" );
    lua_getfield( L, -1, "realpath" );
    lua_pushstring( L, filename );
    lua_call( L, 1, 1 );
    // Set the __FILE__ variable to the function return value
    lua_setglobal( L, "__FILE__" );

    // Call the dirname function on the __FILE__ var
    lua_getfield( L, LUA_GLOBALSINDEX, "file" );
    lua_getfield( L, -1, "dirname" );
    lua_getglobal( L, "__FILE__" );
    lua_call( L, 1, 1 );
    // Set the __DIR__ variable to the function return value
    lua_setglobal( L, "__DIR__" );

    DEBUGLOG( "Trying to open file: %s", filename );
    if ( ( f = fopen( filename, "r" ) ) == NULL )
    {
        luaL_error( L, "Could not open file \"%s\" for inclusion", filename );
    }
    DEBUGLOG( "Trying to include file: %s", filename );
    LUACHECK( lua_load( L, lucie_reader, f, filename ) );
    fclose( f );
    LUACHECK( lua_pcall( L, 0, LUA_MULTRET, 0 ) );

    // Restore env
    DEBUGLOG( "Restoring env" );
    lua_pushstring( L, oldFILE );
    lua_setglobal( L, "__FILE__" );
    free( oldFILE );

    lua_pushstring( L, oldDIR );
    lua_setglobal( L, "__DIR__" );
    free( oldDIR );

    return 0;
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

    DEBUGLOG( "Converting cgi environment to superglobals" );
    init_superglobals( L );

    DEBUGLOG( "Registering lucieinfo function" );
    lua_pushcfunction( L, L_lucieinfo );               
    lua_setglobal( L, "lucieinfo" );

    DEBUGLOG( "Installing output overrides" );
    init_output_functionality( L );

    // Register include and dofile functions
    lua_pushcfunction( L, L_dofile );
    lua_setglobal( L, "dofile" );
    lua_pushcfunction( L, L_dofile );
    lua_setglobal( L, "include" );

    // Register dirname, realpath functions
    lua_newtable( L );
    lua_pushstring( L, "dirname" );
    lua_pushcfunction( L, L_dirname );
    lua_settable( L, -3 );
    lua_pushstring( L, "realpath" );
    lua_pushcfunction( L, L_realpath );
    lua_settable( L, -3 );
    lua_setglobal( L, "file" );

    // Register all needed extensions
    DEBUGLOG( "Registering extensions" );
    register_extensions( L );
    DEBUGLOG( "All extensions registered" );


    // Execute the main script
    lua_getfield( L, LUA_GLOBALSINDEX, "dofile" );
    lua_pushstring( L, argv[1] );
    lua_call( L, 1, 0 );

    // At least output the default header
    header_output();

    // Cleanup header data
    cleanup_headerdata();

    // If the output buffer has not been terminated do it now and output its contents
    if ( outputbuffer != NULL ) 
    {
        printf( outputbuffer );
        free( outputbuffer );
    }

    // Cleanup memory from registered extensions
    cleanup_registered_extensions();

    // Destroy the lua environment
    DEBUGLOG( "Closing lua" );
    lua_close( L );

    return EXIT_SUCCESS;
}
