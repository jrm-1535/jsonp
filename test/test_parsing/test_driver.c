

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "jsonvalue.h"
#include "jsonparse.h"
#include "jsonserial.h"

#define IN_MEMORY_PARSING 0

/*
    Test driver for feeding data to JSON parser
*/

void exit_print( char *name, const char *fmt, ... )
{
  va_list ap;

  printf( "%s: ", name );
  va_start(ap, fmt );
  vprintf( fmt, ap );
  va_end( ap );
  exit(1);
}

static void help_exit( const char *name,  char *err )
{
    if ( err )
        printf("%s: error: %s\n\n", name, err );
    printf("%s [options] name\n\n", name );
    printf("options:\n -c            accept c/c++ comments /* ... */ //...\n");
    printf(" -h            print this message\n");
    printf(" -v            verbose\n");
    printf("\nname is the json test file name to run\n");
    printf("\nBy default runs all conformance tests\n");
    exit( (err != NULL) );
}

typedef struct {
    const char *xname;
    const char *sname;
    FILE       *sfile;
    size_t     sfsize;
    int        verbose, comments;
} arg_struct_t;

static void parse_args( int argc, const char **argv, arg_struct_t *args )
{
    int index;

    assert( argv );
    assert( args );

    args->xname = argv[0];
    args->sname = NULL;
    args->sfile = NULL;
    args->verbose = 0;
    args->comments = 0;

    for (index = 1 ; index < argc; index++ ) {
        const char *arg = argv[index];

        if (*arg == '-' ) {  /* an option */
            switch (arg[1]) {
            default:
                help_exit( argv[0], "unrecognized option" );
            break;

            case 'c':
                args->comments = 1;
                break;

            case 'v':
                args->verbose = 1;
                break;

            case 'h':
                help_exit( argv[0], NULL );
            break;
            }
        } else {
            if( NULL == args->sname )
                args->sname = arg;
            else
                help_exit( argv[0], "too many files" );
        }
    }
}

static json_value_t *parse_json_file( arg_struct_t *args )
{
    args->sfile = fopen( args->sname, "r" );
    if ( NULL == args->sfile ) {
        printf("\n%s: can't open file %s\n", args->xname, args->sname);
        return NULL;
    }

#if IN_MEMORY_PARSING
    args->sfsize = st.st_size;
    assert( args->sfile );
    unsigned char *buffer = malloc( args->sfsize + 1 );
    assert( buffer );

    if ( args->sfsize != fread( buffer, 1, args->sfsize, args->sfile ) ) {
        printf( "\n%s: error reading file %s\n", args->xname, args->sname );
        fclose( args->sfile );
        return NULL;
    }
    fclose( args->sfile );

    buffer[ args->sfsize ] = 0;
    if ( strlen( (const char *)buffer ) != args->sfsize ) {
         printf("\n%s: file %s contains binary '\\0' in text\n", args->xname, args->sname );
         free( buffer );
        return NULL;
    }

    if ( args->verbose ) {
        printf( "\n%s: processing json file %s (%lu bytes)\n",
                args->xname, args->sname, args->sfsize );
        printf( "============================ INPUT TEXT ===============================\n");
        printf( "%s\n", buffer );
        printf( "============================= OUTCOME =================================\n");
    }

    json_error_report_t error;
    json_value_t *result = json_parse( buffer, (bool)args->comments, &error );
#else
    json_error_report_t error;
     if ( args->verbose ) {
        printf( "\n%s: processing json file %s (%lu bytes)\n",
                args->xname, args->sname, args->sfsize );
        printf( "============================= OUTCOME =================================\n");
    }
    json_value_t *result = json_parse_stream( args->sfile, (bool)args->comments, &error );
#endif
    if ( NULL == result ) {
        if ( args->verbose )
            printf("%s: %s\n", args->xname, error.error_string );
        free( error.error_string );
    } else if ( args->verbose ) {
        json_print( result, PRETTY_FORMAT, NULL, stdout );
    }
#if IN_MEMORY_PARSING
    free( buffer );
#else
    fclose( args->sfile );
#endif
    return result;
}

int main( int argc, const char **argv )
{
    arg_struct_t args;
    parse_args( argc, argv, &args );

    if ( args.sname ) {
        json_value_t *root = parse_json_file( &args );
        if ( NULL == root ) {
            printf( "%s: parse error\n", args.xname );
            return -1;
        }
        printf( "%s: parse successful\n", args.xname );
        json_free( root );
        return 0;
    }

    DIR *tdir;
    if ( 0 != chdir( "test" ) || NULL == ( tdir = opendir( "test_parsing" ) ) ) {
        printf("%s: Unable to open directory - exiting\n", args.xname );
        return 1;
    }

    if ( 0 != chdir( "test_parsing" ) ) {
        printf("%s: Unable to find files - exiting\n", args.xname );
        closedir( tdir );
        return 1;
    }

    unsigned int passed = 0, undecided = 0, total = 0;
    while ( 1 ) {
        struct dirent *entry = readdir( tdir );
        if ( NULL == entry ) break;

        if ( '.' == entry->d_name[0] ) continue;  // skip '.' and '..'
        char *extension = strrchr( entry->d_name, '.' );
        if ( NULL == extension || strcmp( extension, ".json" ) ) continue;

        args.sname = entry->d_name;
        int expected = entry->d_name[0];

        //printf( "Processing file %s\n", args.sname );

        json_value_t *root = parse_json_file( &args );
        if ( 'y' == expected ) {
            if ( NULL == root ) {
                printf( "\x1b[01;31m%s: Failed test \"%s\", which was expected to pass\x1b[00;39m\n",
                        args.xname, args.sname );
            } else {
                ++passed;
                json_free( root );
            }
        } else if ( 'n' == expected ) {
            if ( NULL != root ) {
                printf( "\x1b[01;31m%s: Passed test \"%s\", which was expected to fail\x1b[00;39m\n",
                        args.xname, args.sname );
                json_free( root );
            } else ++passed;
        } else {  // undecided
            if ( NULL == root ) {
                printf( "%s: Failed test \"%s\", which is undecided\n",
                        args.xname, args.sname );
            } else {
                printf( "%s: Passed test \"%s\", which is undecided\n",
                        args.xname, args.sname );
                json_free( root );
            }
            ++undecided;
        }
        ++total;
    }
    printf( "\x1b[01;39m%s: completed test suite (passed %u, undecided %u out of %u tests)\x1b[00;39m\n",
            args.xname, passed, undecided, total );
    closedir( tdir );
    return 0;
}
