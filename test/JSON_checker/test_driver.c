

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include "jsonvalue.h"
#include "jsonparse.h"
#include "jsonserial.h"

/*
    Test driver for feeding JSON_checker data to JSON parser
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
    printf("\nBy default runs all tests except fail1.json and fail18.json,\nwhich are not valid tests\n");
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

    struct stat st;
    if ( 0 != fstat( fileno(args->sfile), &st ) ) {
        printf("\n%s: empty or unredable file %s\n", args->xname, args->sname);
        return NULL;
    }

    args->sfsize = st.st_size;
    assert( args->sfile && args->sfsize );
    unsigned char *buffer = malloc( args->sfsize + 1 );
    assert( buffer );

    if ( args->sfsize != fread( buffer, 1, args->sfsize, args->sfile ) ) {
        printf( "\n%s: error reading file %s\n", args->xname, args->sname );
        return NULL;
    }
    fclose( args->sfile );

    buffer[ args->sfsize ] = 0;
    if ( args->verbose ) {
        printf( "\n%s: processing json file %s (%lu bytes)\n",
                args->xname, args->sname, args->sfsize );
        printf( "============================ INPUT TEXT ===============================\n");
        printf( "%s\n", buffer );
        printf( "============================= OUTCOME =================================\n");
    }

    json_error_report_t error;
    json_value_t *result = json_parse_buffer( buffer, (bool)args->comments, &error );
    if ( NULL == result ) {
        if ( args->verbose )
            printf("%s: %s\n", args->xname, error.error_string );
        free( error.error_string );
    } else if ( args->verbose ) {
        json_print( result, PRETTY_FORMAT, NULL, stdout );
    }
    free( buffer );

    return result;
}

typedef struct {
    const char *filename;
    bool expected;
} test_t;

int main( int argc, const char **argv )
{
    arg_struct_t args;
    parse_args( argc, argv, &args );

    test_t tests[] = { { "fail2.json", false }, { "fail3.json", false },
                       { "fail4.json", false }, { "fail5.json", false },
                       { "fail6.json", false }, { "fail7.json", false },
                       { "fail8.json", false }, { "fail9.json", false },
                       { "fail10.json", false }, { "fail11.json", false },
                       { "fail12.json", false }, { "fail13.json", false },
                       { "fail14.json", false }, { "fail15.json", false },
                       { "fail16.json", false }, { "fail17.json", false },
                       { "fail19.json", false }, { "fail20.json", false },
                       { "fail21.json", false }, { "fail22.json", false },
                       { "fail23.json", false }, { "fail24.json", false },
                       { "fail25.json", false }, { "fail26.json", false },
                       { "fail27.json", false }, { "fail28.json", false },
                       { "fail29.json", false }, { "fail30.json", false },
                       { "fail31.json", false }, { "fail32.json", false },
                       { "fail33.json", false }, { "pass1.json", true },
                       { "pass2.json", true }, { "pass3.json", true } };

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

    if ( 0 != chdir( "test/JSON_checker" ) ) {
        printf("%s: Unable to find JSON_checker files - exiting\n",
                args.xname );
        return 1;
    }

    unsigned int passed = 0;
    for ( unsigned int i = 0; i < sizeof( tests ) / sizeof( test_t ); ++i ) {
        args.sname = tests[i].filename;
        json_value_t *root = parse_json_file( &args );
        if ( true == tests[i].expected ) {
            if ( NULL == root ) {
                printf( "%s: Failed test \"%s\", which was expected to pass\n",
                        args.xname, tests[i].filename );
            } else {
                ++passed;
                json_free( root );
            }
        } else {
            if ( NULL != root ) {
                printf( "%s: Passed test \"%s\", which was expected to fail\n",
                        args.xname, tests[i].filename );
                json_free( root );
            } else ++passed;
        }
    }
    printf( "%s: completed test suite (passed %u out of %lu tests)\n",
            args.xname, passed, sizeof( tests ) / sizeof( test_t ) );
    return 0;
}
