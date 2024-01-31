
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "jsonserial.h"

/*  -----------------------------------------------------------------
    serialize json values
    -----------------------------------------------------------------  */

typedef struct {
    FILE *              fd;
    char *              ptr;
    size_t              remaining_size, size;
    json_serialize_t    format;
    unsigned int        indent;
    bool                follows;
} serialize_context_t;


/* Default syntax highlighting color definitions.
   Definitions are given in an array ordered by json types (
    NOT_A_JSON_VALUE (used for assignment signe ':'), JSON_NULL,
    JSON_BOOLEAN, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT).

    Modes:  NORMAL=0, BOLD=1, FAINT=2, ITALIC=3, UNDERLINE=4
    Colors: BLACK=0, RED=1, GREEN=2, YELLOW=3, BLUE=4, MAGENTA=5, CYAN=6, WHITE=7

    NOT_A_JSON_VALUE  BOLD   GREEN
    JSON_NULL         NORMAL BLUE
    JSON_BOOLEAN      NORMAL GREEN
    JSON_NUMBER       NORMAL RED
    JSON_STRING       BOLD   MAGENTA
    JSON_ARRAY        BOLD   CYAN
    JSON_OBJECT       BOLD   WHITE

   Those definitions are compiled into an ANSI SGR string that can be used
   directly to set/reset the actual highlight.
 */

static const char * reset_highlight = "\033[0m";

static char *current_highlight[ JSON_OBJECT + 1 ];

#define MAX_SGR_SEQUENCE_LEN 8
static void set_highlight( json_highlight_t *def )
{
    static const char * default_highlight[ ] = {
        "\33[1;32m" /*   BOLD,   GREEN */, "\33[0;34m" /* NORMAL,    BLUE */,
        "\33[0;32m" /* NORMAL,   GREEN */, "\33[0;31m" /* NORMAL,     RED */,
        "\33[1;35m" /*   BOLD, MAGENTA */, "\33[1;36m" /*   BOLD,    CYAN */,
        "\33[1;37m" /*   BOLD,   WHITE */ };
    // longuest string is \33[x;3ym\0 (8 characters including terminating 0)
    static char highlight_strings[ 8 ][ sizeof(default_highlight)/sizeof(char *) ];
    memcpy ( current_highlight, default_highlight, sizeof( current_highlight ) );

    if ( NULL == def ) return;

    for ( unsigned int i = 0; i < def->nb_entries; ++i ) {
        json_highlight_definition_t *hd = &def->definitions[ i ];
        if ( hd->vtype < NOT_A_JSON_VALUE || hd->vtype > JSON_OBJECT )
            continue; // ignore invalid entries

        int mode = hd->mode, color = hd->color;
        snprintf( highlight_strings[ i ], MAX_SGR_SEQUENCE_LEN, "\33[%d;%dm",
                   mode, 30 + color );
        current_highlight[ i ] = highlight_strings[ i ];
    }
}

static void write_indent( serialize_context_t *sctxt )
{
    assert( sctxt );

    if ( 0 == ( sctxt->format & PRETTY_FORMAT ) ) return;

    sctxt->size += sctxt->indent;           // size is the actual size
    if ( sctxt->fd ) {                      // write to file
        for ( int i = sctxt->indent; i; --i )  putc( ' ', sctxt->fd );
    } else if ( sctxt->ptr ) {              // storage is active
        if ( sctxt->remaining_size > sctxt->indent ) {
            for ( int i = sctxt->indent; i; --i ) *(sctxt->ptr)++ = ' ';
            sctxt->remaining_size -= sctxt->indent;
        } else {
            sctxt->ptr = NULL;              // no room, stop storing here.
        }
    }
}

static void write_char( serialize_context_t *sctxt, char c )
{
    assert( sctxt );

    sctxt->size += 1;                       // size is always updated first
    if ( sctxt->fd ) {                      // write to file
        putc( c, sctxt->fd );
    } else if ( sctxt->ptr ) {              // storage is active
        if ( sctxt->remaining_size > 1 ) {
            *(sctxt->ptr)++ = c;
            --sctxt->remaining_size;
        } else {
            sctxt->ptr = NULL;
        }
    }
}

static inline void write_n_chars( serialize_context_t *sctxt,
                                  const char *chars )
{
    while ( *chars )
        write_char( sctxt, *chars++ );
}

static void write_string( serialize_context_t *sctxt,
                          const char *string, bool name )
{
    assert( sctxt );

    if ( ! name && SYNTAX_HIGHLIGHT == sctxt->format )
        fputs( current_highlight[ JSON_STRING ], sctxt->fd );
    write_char( sctxt, '\"' );

    for ( const unsigned char *p = (const unsigned char *)string; *p; ++p ) {
        unsigned char c;
        switch( ( c = *p ) ) {
        case '"':
            write_n_chars( sctxt, "\\\"" );
            break;
        case '\b':
            write_n_chars( sctxt, "\\b" );
            break;
        case '\f':
            write_n_chars( sctxt, "\\f" );
            break;
        case '\n':
            write_n_chars( sctxt, "\\n" );
            break;
        case '\r':
            write_n_chars( sctxt, "\\r" );
            break;
        case '\t':
            write_n_chars( sctxt, "\\t" );
            break;
        case '\\':
            write_n_chars( sctxt, "\\\\" );
            break;
        default:
            if ( c < ' ' ) {
                write_n_chars( sctxt, "\\u00" );
                write_char( sctxt, (c > 0x0f) ? '1' : '0' );
                c &= 0x0f;
                write_char( sctxt, (c < 10) ? '0' + c : 'a'-10 + c );
            } else {
                write_char( sctxt, c );
            }
            break;
        }
    }

    write_char( sctxt, '\"' );
    if ( ! name && SYNTAX_HIGHLIGHT == sctxt->format )
        fputs( reset_highlight, sctxt->fd );
}

static void write_boolean( serialize_context_t *sctxt,
                           const json_value_t *value )
{
    const char *s = (json_get_boolean_value( value ) == 1) ? "true" : "false";
    if ( sctxt->fd && SYNTAX_HIGHLIGHT == sctxt->format ) {
        fputs( current_highlight[ JSON_BOOLEAN], sctxt->fd );
        write_n_chars( sctxt, s );
        fputs( reset_highlight, sctxt->fd );
    } else {
        write_n_chars( sctxt, s );
    }
}

static void write_number( serialize_context_t *sctxt,
                          const json_value_t *value )
{
    assert( sctxt );

    bool is_integer = JSON_INTEGER_NUMBER == json_get_value_number_type( value );
    assert ( is_integer || JSON_REAL_NUMBER == json_get_value_number_type( value ) );

    long long int integer_value;
    double real_value;
    size_t size;

    if ( is_integer ) {
        integer_value = json_get_integer_value( value );
        size = snprintf( sctxt->ptr, 0, "%lld", integer_value );
    } else {
        real_value = json_get_real_value( value );
        size = snprintf( sctxt->ptr, 0, "%g", real_value );
    }

    sctxt->size += size;                    // size is always updated first

    if ( sctxt->fd ) {                      // write to file
        if ( SYNTAX_HIGHLIGHT == sctxt->format )
            fputs( current_highlight[ JSON_NUMBER ], sctxt->fd );
        if ( is_integer ) {
            fprintf( sctxt->fd, "%lld", integer_value );
        } else {
            fprintf( sctxt->fd, "%g", real_value );
        }
        if ( SYNTAX_HIGHLIGHT == sctxt->format )
            fputs( reset_highlight, sctxt->fd );
    } else if ( sctxt->ptr ) {              // storage is active
        if ( sctxt->remaining_size > size ) {
            if ( is_integer ) {
                snprintf( sctxt->ptr, sctxt->remaining_size, "%lld", integer_value );
            } else {
                snprintf( sctxt->ptr, sctxt->remaining_size, "%g", real_value );
            }
            sctxt->ptr += size;
            sctxt->remaining_size -= size;
        } else {
            sctxt->ptr = NULL;
        }
    }
}

static void enclose_object( serialize_context_t *sctxt, char c )
{
    if ( SYNTAX_HIGHLIGHT == sctxt->format ) {
        fputs( current_highlight[ JSON_OBJECT ], sctxt->fd );
        write_char( sctxt, c );
        fputs( reset_highlight, sctxt->fd );
    } else {
        write_char( sctxt, c );
    }
}

static void write_assignment( serialize_context_t *sctxt )
{
    if ( SYNTAX_HIGHLIGHT == sctxt->format ) {
        fputs( current_highlight[ NOT_A_JSON_VALUE ], sctxt->fd );
        write_n_chars( sctxt, ": " );
        fputs( reset_highlight, sctxt->fd );
    } else if ( PRETTY_FORMAT == sctxt->format ){
        write_n_chars( sctxt, ": " );
    } else {
        write_n_chars( sctxt, ":" );
    }
}

static void json_serialize_value( serialize_context_t *sctxt,
                                  const json_value_t *value );

static void json_serialize_object( serialize_context_t *sctxt,
                                   const json_value_t *value )
{
    assert( sctxt );
    assert( value );

    json_object_iterator_t iter = json_new_object_iterator( value );
    assert( iter );

    bool first = true;
    while ( true ) {
        const unsigned char *mname;
        const json_value_t *mval = json_iterate_object_member( &iter, &mname );
        if ( NULL == mval )
            break;

        if ( ! first ) {
            write_char( sctxt, ',' );
            if ( PRETTY_FORMAT & sctxt->format ) write_char( sctxt, '\n' );
        }
        first = false;

        write_indent( sctxt );
        write_string( sctxt, (const char *)mname, true );
        write_assignment( sctxt );
        if ( PRETTY_FORMAT == sctxt->format ) write_char( sctxt, ' ' );
        sctxt->follows = true;
        json_serialize_value( sctxt, mval );
    }
    if ( PRETTY_FORMAT & sctxt->format ) write_char( sctxt, '\n' );
    json_free_object_iterator( iter );
}

static void enclose_array( serialize_context_t *sctxt, char c )
{
    if ( SYNTAX_HIGHLIGHT == sctxt->format ) {
        fputs( current_highlight[ JSON_ARRAY ], sctxt->fd );
        write_char( sctxt, c );
        fputs( reset_highlight, sctxt->fd );
    } else {
        write_char( sctxt, c );
    }
}

static void json_serialize_array( serialize_context_t *sctxt,
                                  const json_value_t *value )
{
    assert( value );

    json_array_iterator_t iter = json_new_array_iterator( value );
    assert( iter );

    bool first = true;
    while( true ) {
        const json_value_t *aval = json_iterate_array_element( &iter );
        if ( NULL == aval )
            break;

        if ( ! first ) {
            write_char( sctxt, ',' );
            if ( PRETTY_FORMAT & sctxt->format ) write_char( sctxt, '\n' );
        }
        first = false;

        json_serialize_value( sctxt, aval );
    }
    if ( PRETTY_FORMAT & sctxt->format ) write_char( sctxt, '\n' );
    json_free_array_iterator( iter );
}

static void write_null( serialize_context_t *sctxt )
{
    if ( SYNTAX_HIGHLIGHT == sctxt->format ) {
        fputs( current_highlight[ JSON_NULL ], sctxt->fd );
        write_n_chars( sctxt, "null" );
        fputs( reset_highlight, sctxt->fd );
    } else {
        write_n_chars( sctxt, "null" );
    }
}

static void json_serialize_value( serialize_context_t *sctxt,
                                  const json_value_t *value )
{
    assert( sctxt );
    assert( value );

    if ( ! sctxt->follows) write_indent( sctxt );
    sctxt->follows = false;

    switch( json_get_value_type( value ) ) {
    default:
        assert(0);
    case JSON_OBJECT:
        enclose_object( sctxt, '{' );
        if ( PRETTY_FORMAT & sctxt->format ) write_char( sctxt, '\n' );
        sctxt->indent += 4;
        json_serialize_object( sctxt, value );
        sctxt->indent -= 4;
        write_indent( sctxt );
        enclose_object( sctxt, '}' );
        break;
    case JSON_ARRAY:
        enclose_array( sctxt, '[' );
        if ( PRETTY_FORMAT & sctxt->format ) write_char( sctxt, '\n' );
        sctxt->indent += 4;
        json_serialize_array( sctxt, value );
        sctxt->indent -= 4;
        write_indent( sctxt );
        enclose_array( sctxt, ']' );
        break;
    case JSON_STRING:
        write_string( sctxt, (const char *)json_get_string_value( value ), false );
        break;
    case JSON_NUMBER:
        write_number( sctxt, value );
        break;
    case JSON_BOOLEAN:
        write_boolean( sctxt, value );
        break;
    case JSON_NULL:
        write_null( sctxt );
        break;
    }
}

extern size_t json_get_serialization_length( const json_value_t *value,
                                             json_serialize_t format )
{
    if ( NULL == value ) return 0;

    serialize_context_t sctxt;
    sctxt.fd = NULL;
    sctxt.ptr = NULL;
    sctxt.remaining_size = 0;
    sctxt.size = 0;
    sctxt.indent = 0;
    sctxt.follows = false;
    sctxt.format = format;

    json_serialize_value( &sctxt, value );
    return sctxt.size;
}

extern size_t json_serialize( const json_value_t *value,
                              json_serialize_t format,
                              size_t max_size, char *buffer )
{
    if ( NULL == value ) return 0;

    serialize_context_t sctxt;
    sctxt.fd = NULL;
    sctxt.ptr = buffer;
    sctxt.remaining_size = max_size;
    sctxt.size = 0;
    sctxt.indent = 0;
    sctxt.follows = false;
    if ( SYNTAX_HIGHLIGHT == format ) format = PRETTY_FORMAT;
    sctxt.format = format;

    json_serialize_value( &sctxt, value );
    if ( sctxt.size < max_size )
        *(sctxt.ptr) = 0;
    return sctxt.size;
}

extern size_t json_print( const json_value_t *value, json_serialize_t format,
                          json_highlight_t *highlight, FILE *fd )
{
    if ( NULL == value || NULL == fd ) return 0;

    serialize_context_t sctxt;
    sctxt.fd = fd;
    sctxt.ptr = NULL;
    sctxt.remaining_size = 0;
    sctxt.size = 0;
    sctxt.indent = 0;
    sctxt.follows = false;
    sctxt.format = format;

    if ( SYNTAX_HIGHLIGHT == format )
        set_highlight( highlight );

    json_serialize_value( &sctxt, value );
    if ( PRETTY_FORMAT & format ) {
        fputc( '\n', fd );
        ++sctxt.size;
    }
    return sctxt.size;
}
