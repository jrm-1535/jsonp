

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include <malloc.h>
#include <string.h>

#include "jsonvalue.h"
#include "jsonparse.h"
#include "jsonedit.h"
#include "jsonserial.h"

/*
    Driver for JSON parser
*/

static int verbose = 0;

void warning_print( const char *fmt, ... )
{
    if (verbose) {
        va_list ap;
        printf( "jsonc: " );
        va_start(ap, fmt );
        vprintf( fmt, ap );
        va_end( ap );
    }
}

void exit_print( const char *fmt, ... )
{
  va_list ap;

  printf( "jsonc: " );
  va_start(ap, fmt );
  vprintf( fmt, ap );
  va_end( ap );
  exit(1);
}

static void help_exit( char *err )
{
    if ( err )
        printf("jsonc: error: %s\n\n", err );
    printf("jsonc [options] name\n\n");
    printf("options:\n -c            accept c/c++ comments /* ... */ //...\n");
    printf(" -h            print this message\n");
    printf(" -v            verbose\n");
    printf("\nname is the json file name\n");
    exit( (err != NULL) );
}

typedef struct {
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

    args->sname = NULL;
    args->sfile = NULL;
    args->verbose = 0;
    args->comments = 0;

    for (index = 1 ; index < argc; index++ ) {
        const char *arg = argv[index];

        if (*arg == '-' ) {  /* an option */
            switch (arg[1]) {
            default:
                help_exit( "unrecognized option" );
            break;

            case 'c':
                args->comments = 1;
                break;

            case 'v':
                args->verbose = 1;
                break;

            case 'h':
                help_exit( NULL );
            break;
            }
        } else {
            if( NULL == args->sname )
                args->sname = arg;
            else
                help_exit( "too many files" );
        }
    }

    if ( NULL == args->sname ) {
        help_exit( "no file name specified" );
    }
}

static json_value_t *parse_json_file( arg_struct_t *args )
{
    assert( args->sfile && args->sfsize );
    unsigned char *buffer = malloc( args->sfsize + 1 );
    assert( buffer );

    if ( args->sfsize != fread( buffer, 1, args->sfsize, args->sfile ) ) {
        printf( "jsonc: error reading file %s\n", args->sname );
        return NULL;
    }

    buffer[ args->sfsize ] = 0;
    if ( args->verbose ) {
        printf("jsonc: processing json file %s (%lu bytes)\n", args->sname, args->sfsize );
        printf( "Buffer <%s>\n", buffer );
    }
    return json_parse_buffer( buffer, (bool)args->comments, NULL );
}

#ifdef _JSON_FAST_ACCESS_LARGER_CODE
static void serialize( json_serialize_t format, json_value_t *root )
{
    const char *fs;
    switch ( format ) {
    case SYNTAX_HIGHLIGHT:
        fs = "Highlight";
        break;
    case PRETTY_FORMAT:
        fs = "Pretty";
        break;
    case PACKED_FORMAT:
        fs = "Packed";
        break;
    }

    size_t length = json_get_serialization_length( root, format );
    printf( "\n%s Serialization length %lu\n", fs, length );
    char *buffer = malloc( 1 + length ); // + terminating 0

    size_t actual = json_serialize( root, format, 1 + length, buffer );
    printf( "Serialized JSON:\n%s\n\n", buffer );
    printf( "Lengths: initial %lu, actual %lu, length %lu\n", length, actual, strlen( buffer ) );
    free( buffer );
}

static void edit( json_value_t *root )
{
    const json_value_t *items = json_search_for_object_member_by_name(
                                            root, (unsigned char *)"items" );
    if ( NULL == items )
        printf( "Cannot find \"items\" in root\n" );
    else {
        printf( "Found \"items\" array: " );
        json_print( items, PRETTY_FORMAT, NULL, stdout );
    }

    json_value_t *previous_value =
                json_remove_element_from_array( (json_value_t *)items, 3 );
    if ( NULL == previous_value ) {
        printf( "Cannot remove entry [3] in items\n" );
    } else {
        printf( "Removed entry [3] in items - previous value :\n" );
        json_print( previous_value, PRETTY_FORMAT, NULL, stdout );
        json_free_value( previous_value );
        printf( "New array definition:\n" );
        json_print( items, PRETTY_FORMAT, NULL, stdout );
    }

    json_value_t *new_value = json_new_value( JSON_STRING, "A new string" );
    if ( NULL == new_value ) {
        printf(" Unable to create a new JSON_STRING value\n" );
    } else {
        printf( "Created new_value:\n" );
        json_print( new_value, PRETTY_FORMAT, NULL, stdout );
    }

    previous_value =
        json_replace_element_in_array( (json_value_t *)items, 2, new_value );
    if ( NULL == previous_value ) {
        printf("Failed to replace entry [2] in items\n");
    } else {
        printf("Replaced entry [2] in items: previous_value\n" );
        json_print( previous_value, PRETTY_FORMAT, NULL, stdout );
        json_free_value( previous_value );
        printf( "New array definition:\n" );
        json_print( items, PRETTY_FORMAT, NULL, stdout );
    }

    previous_value =
        json_remove_element_from_array( (json_value_t *)items, 2 );
    if ( NULL == previous_value ) {
        printf( "Cannot remove entry [2] in items\n" );
    } else {
        printf( "Removed entry [2] in items - previous value:\n" );
        json_print( previous_value, PRETTY_FORMAT, NULL, stdout );
        json_free_value( previous_value );
        printf( "New array definition:\n" );
        json_print( items, PRETTY_FORMAT, NULL, stdout );
    }

    new_value = json_new_value( JSON_ARRAY );
    if ( NULL == new_value ) {
        printf(" Unable to create a new JSON_ARRAY value\n" );
    } else {
        printf( "Created new array value:\n" );
        json_print( new_value, PRETTY_FORMAT, NULL, stdout );

        json_value_t *new_item = json_new_value( JSON_NUMBER,
                                                 JSON_INTEGER_NUMBER, 17 );
        if ( NULL == new_item ) {
            printf(" Unable to create a new JSON_INTEGER_NUMBER value\n" );
            json_free_value( new_value );
        } else {
            printf( "Created new integer value:\n" );
            json_print( new_item, PRETTY_FORMAT, NULL, stdout );
            json_status_t status =
                    json_insert_element_into_array( new_value, 0, new_item );
            if ( JSON_STATUS_SUCCESS != status ) {
                printf("Failed to insert into new array at index 0\n");
                json_free_value( new_item );
                json_free_value( new_value );
            } else {
                printf( "Inserted new integer value into new array at index 0 :\n" );
                json_print( new_value, PRETTY_FORMAT, NULL, stdout );
                new_item = json_new_value( JSON_BOOLEAN, true );
                if ( NULL == new_item ) {
                    printf(" Unable to create a new JSON_BOOLEAN value\n" );
                    json_free_value( new_value );
                } else {
                    printf( "Created new boolean value:\n" );
                    json_print( new_item, PRETTY_FORMAT, NULL, stdout );
                    json_status_t status =
                        json_insert_element_into_array( new_value, 1, new_item );
                    if ( JSON_STATUS_SUCCESS != status ) {
                        printf("Failed to insert into new array at index 1\n");
                        json_free_value( new_item );
                        json_free_value( new_value );
                    } else {
                        printf( "Inserted new boolean value into new array at index 1 :\n" );
                        json_print( new_value, PRETTY_FORMAT, NULL, stdout );
                        new_item = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, 23.1 );
                        if ( NULL == new_item ) {
                            printf(" Unable to create a new JSON_REAL_NUMBER value\n" );
                            json_free_value( new_value );
                        } else {
                            printf( "Created new real numner value:\n" );
                            json_print( new_item, PRETTY_FORMAT, NULL, stdout );
                            json_status_t status =
                                json_insert_element_into_array( new_value, 0, new_item );
                            if ( JSON_STATUS_SUCCESS != status ) {
                                printf("Failed to insert into new array at index 0\n");
                                json_free_value( new_item );
                                json_free_value( new_value );
                            } else {
                                printf( "Inserted new real number value into new array at index 0 :\n" );
                                json_print( new_value, PRETTY_FORMAT, NULL, stdout );

                                previous_value =
                                    json_remove_element_from_array( (json_value_t *)new_value, 0 );
                                if ( NULL == previous_value ) {
                                  printf( "Cannot remove entry [0] in new_value\n" );
                                } else {
                                    printf( "Removed entry [0] in new_value - previous value :\n" );
                                    json_print( previous_value, PRETTY_FORMAT, NULL, stdout );
                                    json_free_value( previous_value );
                                    printf( "New new_value array definition:\n" );
                                    json_print( new_value, PRETTY_FORMAT, NULL, stdout );
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    new_value = json_new_value( JSON_OBJECT );
    if ( NULL == new_value ) {
        printf(" Unable to create a new JSON_OBJECT value\n" );
    } else {
        printf( "Created new object value:\n" );
        json_print( new_value, PRETTY_FORMAT, NULL, stdout );
        json_value_t *new_item = json_new_value( JSON_NUMBER,
                                                 JSON_INTEGER_NUMBER, 17 );
        if ( NULL == new_item ) {
            printf(" Unable to create a new JSON_INTEGER_NUMBER value\n" );
            json_free_value( new_value );
        } else {
            printf( "Created new integer value:\n" );
            json_print( new_item, PRETTY_FORMAT, NULL, stdout );
            json_status_t status = json_insert_member_into_object(
                                new_value, (unsigned char *)"foo", new_item );
            if ( JSON_STATUS_SUCCESS != status ) {
                printf("Failed to insert memeber 'foo' into new object\n");
                json_free_value( new_item );
                json_free_value( new_value );
            } else {
                printf( "Inserted new integer member 'foo' into new object :\n" );
                json_print( new_value, PRETTY_FORMAT, NULL, stdout );
            }
        }
    }
}
#endif
int main( int argc, const char **argv )
{
    arg_struct_t args;
    parse_args( argc, argv, &args );

    args.sfile = fopen( args.sname, "r" );
    if ( NULL == args.sfile ) {
        printf("jsonc: can't open file %s\n", args.sname);
        exit(1);
    }

    struct stat st;
    if ( 0 != fstat( fileno(args.sfile), &st ) ) {
        printf("jsonc: empty or unredable file %s\n", args.sname);
        exit(1);
    }

    args.sfsize = st.st_size;
    json_value_t *root = parse_json_file( &args );
    if ( NULL == root ) {
        printf("jsonc: parse error\n");
        exit(1);
    }
    json_print( root, PRETTY_FORMAT, NULL, stdout );
    const json_value_t *gid = json_search_for_object_member_by_name(
                                            root, (unsigned char *)"groupId" );
    if ( NULL == gid )
        printf( "Cannot find \"groupId\" in root\n" );
    else {
        printf( "Found \"groupId\": " );
        json_print( gid, PRETTY_FORMAT, NULL, stdout );
    }

    const json_value_t *items = json_search_for_object_member_by_name(
                                            root, (unsigned char *)"items" );
    if ( NULL == items )
        printf( "Cannot find \"items\" in root\n" );
    else {
        printf( "Found \"items\": " );
        json_print( items, PRETTY_FORMAT, NULL, stdout );
    }

    if ( JSON_ARRAY != json_get_value_type( items ) ) {
        printf("Member \"items\" is not an array\n");
    } else {
        const json_value_t *third = json_get_array_element( items, 3 );
        if ( NULL == third ) {
            printf( "Cannot find entry [3] in items\n" );
        } else {
            printf( "Found entry [3]: " );
            json_print( third, PRETTY_FORMAT, NULL, stdout );
        }
    }
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    edit( root );

    serialize( PRETTY_FORMAT, root );
    serialize( PACKED_FORMAT, root );
#endif
    json_free( root );

    fclose( args.sfile );

    return 0;
}
