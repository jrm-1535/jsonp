
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "jsonparse.h"
#include "jsondata.h"
#include "jsonvalue.h"
#include "jsonutf8.h"

/*  -------------------------------------------------------------------
    simple C JSON parser
    -------------------------------------------------------------------  */

#define MAX_ERROR_STRING_LENGTH  512
typedef struct {
    json_source_t         source;          // for file, pipe or terminal sources

    struct _string_buffer *head, *current; // for string buffering only

    bool                  comments;        // comments accepted
    unsigned int          line;            // current line
    json_status_t         ecode;           // error code & error string below
    char                  estring[MAX_ERROR_STRING_LENGTH];

    unsigned int          open_stack;      // limits stack usage against DOS attack

} json_parse_ctxt_t;

static void error_report( json_parse_ctxt_t *ctxt, json_status_t code, const char *fmt, ... )
{
  va_list ap;

  int next = snprintf( ctxt->estring, MAX_ERROR_STRING_LENGTH,
                       "json_parse: line %d: ", ctxt->line );
  assert( next < MAX_ERROR_STRING_LENGTH );

  va_start(ap, fmt );
  vsnprintf( &ctxt->estring[next], MAX_ERROR_STRING_LENGTH-next,
             fmt, ap );
  va_end( ap );
  ctxt->ecode = code;
}

static void wrong_char_error_report( json_parse_ctxt_t *ctxt, const char *specific, int c )
{
    if ( EOF == c )
        error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
                         "Syntax error (end of text) %s", specific);
    else
        error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
                 "Syntax error (character 0x%02x '%c') %s",  c, (char)c, specific);
}

static bool skip_cpp_comment( json_parse_ctxt_t *ctxt )
{
    while ( 1 ) {             // loop till end of line (0x0a)
        int c;
        bool escaped;
        while ( 0x0a != ( c = ctxt->source.get( &ctxt->source ) ) ) {
            escaped = false;
            if ( '\\' == c ) { // next char is escaped
                escaped = true;
                continue;
            }
            if ( EOF == c ) return true;
        }
        ++ctxt->line;        // increment line count
        if ( ! escaped ) {   // 0x0a was not escaped
            break;           // exit end of line loop
        }
    }
    return false;             // skip end of line
}

static bool skip_c_comment( json_parse_ctxt_t *ctxt )
{
    while ( 1 ) {            // loop till '*'
        int c;
        bool escaped;
        while( '*' != ( c = ctxt->source.get( &ctxt->source ) ) ) {
            escaped = false;
            if ( '\\' == c ) { // next char is escaped
                escaped = true;
                continue;
            }
            if ( EOF == c ) return true;

            if ( 0x0a == c )
                ++ctxt->line; // increment line count
        }
        c = ctxt->source.get( &ctxt->source );
        if ( ! escaped && '/' == c )
            break;           // exit if unescaped '*' and followed by '/'
        ctxt->source.push_back( &ctxt->source, c );
    }
    return false;             // skip '/' following unescaped '*'
}

static int skip_blank( json_parse_ctxt_t *ctxt )
{
    int c;
    //const unsigned char *ptr = ctxt->ptr;
    while ( ( c = ctxt->source.get( &ctxt->source ) ) ) {
        switch( c ) {
        case 0x0a: /* LF */
            ++ctxt->line;
        case 0x09: /* tab */ case 0x0d: /* CR */ case 0x20: /* space */
            continue;
        case '/':
            if ( ctxt->comments ) {
                int c1 = ctxt->source.get( &ctxt->source );
                if ( '/' == c1 ) {          // C++ // comment, till end of line
                    if ( skip_cpp_comment( ctxt ) ) return EOF;
                    continue;
                }
                if ( '*' == c1 ) {          // C comment, till "*/"
                    if ( skip_c_comment( ctxt ) ) return EOF;
                    continue;
                }
                ctxt->source.push_back( &ctxt->source, c1 ); // push '/' or '*' back
                // the previous '/' will generate an error in caller
            }
            break;
        default:
            break;
        }
        break;
    }
    return c;                            // first non blank char
}

/*
    JSON syntax:

object
    {}
    { members }

members
    pair
    pair , members

pair
    string : value

value
    object
    array
    string
    number
    true
    false
    null

array
    []
    [ elements ]

elements
    value
    value , elements

string
    ""
    " chars "

chars
    char
    char chars

char
    any-Unicode-character-
        except-"-or-\-or-
        control-character
    \"
    \\
    \/
    \b
    \f
    \n
    \r
    \t
    \u four-hex-digits

number
    int
    int frac
    int exp
    int frac exp

int
    digit
    digit1-9 digits
    - digit
    - digit1-9 digits

frac
    . digits

exp
    e digits

digits
    digit
    digit digits

e
    e
    e+
    e-
    E
    E+
    E-

*/

static ucs4_t encode_4hex_in_ucs4( unsigned char *ptr )
{
    ucs4_t res = 0;

    for ( int i = 0; i < 4; ++i ) {
        int c = *ptr++;
        if ( c >= '0' && c <= '9' )
            c -= '0';
        else if ( c >= 'A' && c <= 'F' )
            c -= 'A' - 10; // c == 'A' gives 10
        else if ( c >= 'a' && c <= 'f' )
            c -= 'a' - 10; // c == 'a' gives 10
        else return (ucs4_t)0xffffffff;
        res <<= 4;
        res += c;
    }
    return res;
}

/*
    Because we allow the source to be anything, a memory buffer, a stream,
    a file, a pipe or a terminal, we cannot seek back into the source as we
    could with a memory buffer or a file only. Therefore we need a temporary
    buffer to store the string as we parse it to get the length... We use an
    initial 4 k buffer in stack, that is extended with allocated 4 k chunk
    extensions as needed.
*/
#define STRING_BUFFER_SIZE      4000
typedef struct _string_buffer {
    struct _string_buffer *next;
    unsigned char buffer[ STRING_BUFFER_SIZE ];
    unsigned char *ptr;    // current char ptr inside the buffer
} string_buffer_t;

// return true if the character was buffered, truefalseif it failed to allocate
static bool buffer_source_string( string_buffer_t **blockp, unsigned char c )
{
    string_buffer_t *block = *blockp;

    if ( STRING_BUFFER_SIZE == block->ptr - block->buffer ) {
        block = malloc( sizeof( string_buffer_t ) );
        if ( NULL == block ) return false;
        block->next = NULL;
        block->ptr = block->buffer;
        (*blockp)->next = block;
        *blockp = block;
    }
    *block->ptr++ = c;
    return true;
}

static void post_process_buffered_source_string( string_buffer_t *block )
{
    while ( block ) {
        block->ptr = block->buffer;
        block = block->next;
    }
}

static unsigned char read_buffered_source_string( string_buffer_t **blockp )
{
    string_buffer_t *block = *blockp;

    if ( STRING_BUFFER_SIZE == block->ptr - block->buffer ) {
        block = block->next;
        if ( NULL == block )
            return 0;
        *blockp = block;
    }
    return *block->ptr++;
}

static void free_buffered_source_string( string_buffer_t *to_free )
{
    while ( to_free ) {
        string_buffer_t *next = to_free->next;
        free( to_free );
        to_free = next;
    }
}

typedef struct {
    json_parse_ctxt_t *ctxt;
    int               first_char;
} string_ctxt_t;

static int get_string_data( json_data_input_t *data_input )
{
    assert( data_input );
    string_ctxt_t *string_ctxt = data_input->ctxt;
    assert( string_ctxt );
    json_parse_ctxt_t *ctxt = string_ctxt->ctxt;

    int c;
    if ( -1 != string_ctxt->first_char ) {
        c = string_ctxt->first_char;
        string_ctxt->first_char = -1;
    } else {
        c = ctxt->source.get( &ctxt->source );
    }

    unsigned char to_store = ( EOF == c ) ? 0 : (unsigned char)c;
    if ( ! buffer_source_string( &(ctxt->current), to_store ) ) c = EOF;
    return c;
}

/* all error cases from string processing must use string_error in order
   to clen up the memory buffers in use */
static inline int string_error( json_parse_ctxt_t *ctxt,
                                json_status_t status,
                                char *error_string )
{
    error_report( ctxt, status, error_string );
    free_buffered_source_string( ctxt->head->next );
    return -1;
}

// read 4 hex char into the buffer fh that must have room for at least 4 chars
static int read_four_hex( json_parse_ctxt_t *ctxt, unsigned char *fh )
{
    for ( int i = 0; i < 4; ++i ) {
        int c = ctxt->source.get( &ctxt->source ); // not buffered
        if ( EOF == c )
            return string_error( ctxt, JSON_STATUS_INVALID_ENCODING,
                                 "end of file while processing \\u four-hex-digits" );
        fh[ i ] = c;
    }
    return 0;
}

static int process_escaped_ascii_char( json_parse_ctxt_t *ctxt, unsigned char c )
{
    switch( c ) {
    default:
       string_error( ctxt, JSON_STATUS_INVALID_STRING, "invalid escape sequence");
       return -1;

    case '"': case '\\': case '/':            break;
    case 'b':                       c = '\b'; break;
    case 'f':                       c = '\f'; break;
    case 'n':                       c = '\n'; break;
    case 'r':                       c = '\r'; break;
    case 't':                       c = '\t'; break;
    }

    if ( ! buffer_source_string( &(ctxt->current), c ) )
        return string_error( ctxt, JSON_STATUS_INVALID_STRING,
                            "Out of memory while parsing string" );

    return 1;
}

/*
    if no error returns the length as a strictly positive number, -1 otherwise
    The escaped 4 hex digits are converted into a valid UTF8 character sequence.
    If 2 valid escaped surrogates are consecutive (head followed by tail), the
    two are combined into a single UTF8 sequence.
*/
static int process_escaped_4_hex_digits( json_parse_ctxt_t *ctxt )
{
    unsigned char four_hex[ 4 ]; // fills up 4 hex characters following '\u'
    if ( read_four_hex( ctxt, four_hex ) )  return -1;

    ucs4_t encoded = encode_4hex_in_ucs4( four_hex );
    if ( 0xffffffff == encoded )
        return string_error( ctxt, JSON_STATUS_INVALID_ENCODING,
                             "Invalid unicode encoding \\u four-hex-digits" );

    /* if encoded is in the range [ 0xd800 - 0xdbff ] it should be
       followed by a tail surrogate in the range [ 0xdc00 - 0xdfff],
       otherwise an isolated head or tail surrogate is an error.
       So, check if a \u immediately follows (reads 2 bytes ahead) */
    int c1; // holds the single escape char if single_escape_follows is true
    if ( encoded >= 0xd800 && encoded <= 0xdbff ) {
        int c = ctxt->source.get( &ctxt->source );
        if ( '\\' == c ) {
            c1 = ctxt->source.get( &ctxt->source );
            if ( 'u' == c1 ) { // possibly a tail surrogate
                if ( read_four_hex( ctxt, four_hex ) )  return -1;
                ucs4_t tail = encode_4hex_in_ucs4( four_hex );
                if ( tail >= 0xdc00 && tail <= 0xdfff ) {
                    // combine head and tail surrogate into single ucs4_t
                    encoded = ( ( encoded - 0xd800 ) << 10 ) + 0x10000;
                    encoded += tail - 0xdc00;
                }   // all other cases should fail
            }
        }
        /* else the sequence is not valid: the first surrogate
           will return an error (isolated head) below */
    }

    unsigned char tmp[5], *utf8 = tmp;
    if ( json_output_utf8( encoded, &utf8 ) )
        return string_error( ctxt, JSON_STATUS_INVALID_ENCODING,
                            "Unsupported UTF8 encoding \\u four-hex-digits" );

    assert ( utf8 > tmp );
    int len = utf8-tmp;
    // store the decoded escape sequence
    for ( unsigned char *ptr = tmp; ptr < utf8; ++ptr ) {
        if ( ! buffer_source_string( &(ctxt->current), *ptr ) )
            return string_error( ctxt, JSON_STATUS_INVALID_STRING,
                                 "Out of memory while parsing string" );
    }
    return len;
}

static unsigned char *make_string( json_parse_ctxt_t *ctxt )
{
    assert( ctxt );

    string_buffer_t first_block; // fortunately not a recursive function !
    first_block.next = NULL;
    first_block.ptr = first_block.buffer;

    ctxt->head = ctxt->current = &first_block;

    /* first loop: calculate the string length */
    bool terminated = false;
    int backslash = 0, len = 0;

    string_ctxt_t string_ctxt;
    string_ctxt.first_char = -1;
    string_ctxt.ctxt = ctxt;

    json_data_input_t input;
    input.ctxt = &string_ctxt;
    input.read_byte = get_string_data;

    /* " was already removed when entering here */
    int c;
    while ( EOF != ( c = ctxt->source.get( &ctxt->source ) ) ) {
        if ( backslash ) {
            int escaped_len = 0;
            escaped_len = ( 'u' == c ) ? process_escaped_4_hex_digits( ctxt )
                                       : process_escaped_ascii_char( ctxt, c );
             if ( -1 == escaped_len )
                return NULL; // string error was called already

            len += escaped_len;
            backslash = 0;
            continue;       // skip normal UTF8 checking
         }

        if ( '"' == c ) {
            terminated = true;
            break;             // exit loop
        }
        if ( c < 0x20 ) {      // should have been escaped
            string_error( ctxt, JSON_STATUS_INVALID_STRING,
                          "non-escaped control characters" );
            return NULL;
        }
        if ( '\\' == c ) {
            backslash = 1;
        } else {
            string_ctxt.first_char = c;
            unsigned int nbbytes = json_check_utf8( &input );
            if ( 0 == nbbytes ) {
                string_error( ctxt, JSON_STATUS_INVALID_ENCODING,
                              "Invalid UTF8 encoding" );
                return NULL;
            }
            len += nbbytes;
        }
    }
    if ( ! terminated ) {
        string_error( ctxt, JSON_STATUS_INVALID_STRING, "unterminated string");
        return NULL;
    }

    if ( ! buffer_source_string( &(ctxt->current), '\0' ) ) {
        string_error( ctxt, JSON_STATUS_INVALID_STRING,
                      "Out of memory while parsing string" );
        return NULL;
    }

    unsigned char *string = malloc( 1 + len );
    if ( NULL == string ) {
        string_error( ctxt, JSON_STATUS_INVALID_STRING, "Out of memory while allocating string");
        return NULL;
    }

    // get ready for reading buffered string
    ctxt->current = ctxt->head;
    post_process_buffered_source_string( ctxt->head );
    unsigned char *cptr = string;
    /* second loop, just to copy the string */
    while ( ( c = read_buffered_source_string( &(ctxt->current) ) ) ) {
        *cptr++ = c;  // no backslash to worry about anymore...
    }
    assert( cptr - string <= len ); // < if a stray 0 was escaped in the string
    while( cptr - string <= len )
        *cptr++ = 0;                // in which case the string is truncated.

    free_buffered_source_string( ctxt->head->next );
    ctxt->head = ctxt->current = NULL;
    return string;
}

static json_value_t *make_value( json_parse_ctxt_t *ctxt );
static member_t *make_member( json_parse_ctxt_t *ctxt )
{
    assert( ctxt );

    /* " was already removed when entering here */
    unsigned char *name = make_string( ctxt );
    if ( NULL == name ) return NULL;

    json_value_t *value = NULL;
    int c = skip_blank( ctxt );

    if ( ':' != c ) {
        error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
            "Syntax error (missing ':') while expecting \"name\" : value" );
        free( name );
        return NULL;
    }
    value = make_value( ctxt );
    if ( NULL == value ) {
        free( name );
        return NULL;
    }
    member_t *member = new_member( name, value );
    if ( NULL == member ) { // name has already been freed
        json_free( value );
    }
    return member;
}

static object_t *make_object( json_parse_ctxt_t *ctxt )
{
    assert( ctxt );

    object_t *object = new_object( );
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    if( NULL == object ) {
        error_report( ctxt, JSON_STATUS_OUT_OF_MEMORY,
                               "Out of memory while creating an object" );
        return NULL;
    }
#endif
    member_t *last_member = NULL;
    int c = skip_blank( ctxt );    // { was already removed when entering here

    if ( '}' == c ) return object; // empty object is ok

#if ACCEPT_LAST_COMMA
    while ( '}' != c ) {
#else
    while ( true ) {
#endif
        if ( '"' == c ) {
            member_t *member = make_member( ctxt );
            if ( NULL == member ) {
                json_free_object(object);
                return NULL;
            }

            object = object_attach_member( object, member, &last_member );
            if( NULL == object ) {
                error_report( ctxt, JSON_STATUS_OUT_OF_MEMORY,
                               "Out of memory while extending an object" );
                json_free_object(object);
                return NULL;
            }

            c = skip_blank( ctxt );
            if ( ',' == c ) {
                c = skip_blank( ctxt );
                continue;
            } else if ( '}' == c )
                break;
        }
        wrong_char_error_report( ctxt, "while expecting object member", c );
        json_free_object(object);
        return NULL;
    }
    return object;
}

static element_t *make_element( json_parse_ctxt_t *ctxt )
{
    json_value_t *value = make_value( ctxt );
    if ( NULL == value )
        return NULL;
    element_t *element = new_element( value );
    if ( NULL == element ) { // value was already freed
        error_report( ctxt, JSON_STATUS_OUT_OF_MEMORY,
                                "Out of memory while creating an element" );
    }
    return element;
}

static array_t *make_array( json_parse_ctxt_t *ctxt )
{
    assert( ctxt );

    array_t *array = new_array( );
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    if( NULL == array ) {
        error_report( ctxt, JSON_STATUS_OUT_OF_MEMORY,
                                  "Out of memory while creating an array" );
        return NULL;
    }
#endif
    element_t *last_element = NULL;
    int c = skip_blank( ctxt );   // [ was already removed when entering here
    if ( ']' == c ) return array; // empty array is ok

#if ACCEPT_LAST_COMMA
    while ( ']' != c ) {
#else
    while ( true ) {
#endif
        ctxt->source.push_back( &ctxt->source, c ); // backtrack 1 character for make_value
        element_t *element = make_element( ctxt );
        if ( NULL == element ) {
            json_free_array( array );
            return NULL;
        }

        array = array_append_element( array, element, &last_element );
        if( NULL == array ) {  // array was freed in case of error
            error_report( ctxt, JSON_STATUS_OUT_OF_MEMORY,
                                  "Out of memory while extending an array" );
            return NULL;
        }

        c = skip_blank( ctxt );
        if ( ',' == c ) {
            c = skip_blank( ctxt );
            continue;
        } else if ( ']' == c )
            break;

        wrong_char_error_report( ctxt, "while expecting ',' or ']'", c );
        json_free_array( array );
        return NULL;
    }
    return array;
}

static bool normalize_ecma_404_num( json_parse_ctxt_t *ctxt, char *buffer )
{
    bool integer_part = false;

    /* The maximum double mantissa representable is 9,007,199,254,740,992
       So we need to keep track of at least 17 digits for the mantissa.
       The maximum douuble exponent is +308, the minimum is -308.
       The normalized form is x.xxxxxxxxxxxxxxxx eyyy
       Therefore we need at worst 3 (-x.) + 16 + 2( e+/-) + 3 chars, that
       is 24 characters.

       This function normalizes the input string in a passed buffer that
       must be of at least 25 bytes (with terminating 0). Make it 32 for
       safety... */

#define MAX_DECIMAL_NUMBER_STRING_LENGTH 32
#define MAX_DECIMAL_MANTISSA             18

    int c = ctxt->source.get( &ctxt->source );
    if ( '-' == c ) {
        *buffer++ = c;
        c = ctxt->source.get( &ctxt->source ); // accept negative sign
    }
    int nb_mantissa_digits = 0;
    int exponent;
    if ( '0' == c ) {                  // first 0 must be the whole integer part
        integer_part = true;
        *buffer++ = c;
        *buffer++ = '.';
        exponent = 0;
        c = ctxt->source.get( &ctxt->source );
    } else if ( isdigit( c ) ) {       // Non zero begins integer part
        integer_part = true;
        exponent = -1;                 // will be incremented in loop
        do {
            if ( nb_mantissa_digits <  MAX_DECIMAL_MANTISSA ) {
                *buffer++ = c;         // keep significant digits
                if ( 1 == ++nb_mantissa_digits )
                    *buffer++ = '.';
            }
            ++exponent;                // always count 10 power
            c = ctxt->source.get( &ctxt->source );
        } while ( isdigit(c) );
    }
    if ( ! integer_part ) {            // not valid according to ECMA 404
        error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
                      "Syntax error while expecting a number: no integer part" );
        return false;
    }
    if ( '.' == c ) {                  // enter fractional part
        c = ctxt->source.get( &ctxt->source );
        if ( ! isdigit( c ) ) {        // invalid fractional part
            error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
                      "Syntax error while expecting a number: no fractional part after '.'" );
            return false;
        }
        do {
            if ( nb_mantissa_digits <  MAX_DECIMAL_MANTISSA ) {
               *buffer++ = c;          // keep significant digits, ignore extra
               ++nb_mantissa_digits;
            }
            c = ctxt->source.get( &ctxt->source );
        } while ( isdigit(c) );
    }

    int literal_exponent = 0;
    if ( 'e' == c || 'E' == c ) {      // enter exponent
        c = ctxt->source.get( &ctxt->source );
        int exponent_sign = 1;
        if ( '+' == c || '-' == c ) {
            if ( '-' == c ) exponent_sign = -1;
            c = ctxt->source.get( &ctxt->source ); // skip exponent sign
        }
        if ( ! isdigit( c ) ) {        // invalid exponent
            error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
                      "Syntax error while expecting a number: invalid exponent" );
            return false;
        }
        do {
            literal_exponent *= 10;
            literal_exponent += exponent_sign * (c - '0');
            c = ctxt->source.get( &ctxt->source );
        } while ( isdigit(c) );
    }
    ctxt->source.push_back( &ctxt->source, c ); // backtrack last read char
    exponent += literal_exponent;
    if ( exponent ) { // 2( e+/-) + 3 chars
        buffer += snprintf( buffer, 6, "e%d", exponent );
    }
    *buffer = 0;
    return true;
}

static number_t *make_json_number_from_double( double number )
{
// the C compiler removes the non-relevant ifs (it keeps only the proper check)
    if ( 8 == sizeof(long long int) ) {
        if ( -9223372036854775807LL > number ||
              9223372036854775807LL < number )
            return new_number( JSON_REAL_NUMBER, 0, number );
    } else if ( 4 == sizeof( long long int ) ) {
        if ( -2147483647 > number || 2147483647 < number )
            return new_number( JSON_REAL_NUMBER, 0, number );
    } else { // assume 16 bit only
        if ( -32768 > number || 32768 < number )
            return new_number( JSON_REAL_NUMBER, 0, number );
    }
    double ipart;
    modf( number, &ipart );
    return ( ipart == number ) ? new_number( JSON_INTEGER_NUMBER, number, 0 )
                               : new_number( JSON_REAL_NUMBER, 0, number );
}

static number_t *make_number( json_parse_ctxt_t *ctxt )
{
    assert( ctxt );

    char buffer[MAX_DECIMAL_NUMBER_STRING_LENGTH];
    if ( ! normalize_ecma_404_num( ctxt, buffer ) ) return NULL;

    char *moved;
    errno = 0;   // make sure errno is not set
    double d = strtod( buffer, &moved );
    if ( moved == buffer || ERANGE == errno ) {
        error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
                                 "number cannot be represented (range error)" );
        return NULL;
    }
    /* strtod is more lenient and may accept digits beyond the ECMA definition.
       The following line forces the next char to be the one defined by ECMA
       404, which if it is within the number input accepted by strtod will
       cause an error immediately after ( e.g. for 0001 strtod is happy to
       consume the whole number whereas ECMA 404 stops after the first 0). */

    number_t *number = make_json_number_from_double( d );
    if ( NULL == number ) {
        error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
                                 "Out of memory while  while expecting a number" );
    }
    return number;
}

static bool check_litteral( json_parse_ctxt_t *ctxt, const unsigned char *litteral )
{
    int c;
    const unsigned char *ref = litteral + 1;
    do {
        c = ctxt->source.get( &ctxt->source );
        if ( *ref != c ) {
            error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
            "Syntax error (character 0x%02x '%c') while expecting %s",
             c, (char)c, litteral );
            return false;
        }
        ++ref;
    } while ( *ref );
    return true;
}

static int make_boolean( json_parse_ctxt_t *ctxt, unsigned char c )
{
    assert( ctxt );
    assert( 't' == c || 'f' == c );
    if ( 't' == c ) {
        if ( check_litteral( ctxt, (const unsigned char *)"true" ) )
            return true;
        return -1;
    }
    if ( check_litteral( ctxt, (const unsigned char *)"false" ) )
        return false;
    return -1;
}

static json_value_t *make_value( json_parse_ctxt_t *ctxt )
{
    assert( ctxt );

    json_value_type_t vtype;
    value_data_t vdata;

    json_value_t *value = malloc( sizeof( json_value_t ) );
    if ( NULL == value ) return NULL;

    int boolean;
    int c = skip_blank( ctxt );
    switch ( c ) {
    case '{':
        vtype = JSON_OBJECT;
        if ( ++ctxt->open_stack > MAX_OPEN_DEPTH ) {
            error_report( ctxt, JSON_STATUS_OUT_OF_MEMORY,
                            "ran out of allocated stack depth in processing object\n");
            goto error_exit;
        }
        vdata.object = make_object( ctxt );
        if ( NULL == vdata.object ) goto error_exit;
        --ctxt->open_stack;
        break;
    case '[':
        vtype = JSON_ARRAY;
        if ( ++ctxt->open_stack > MAX_OPEN_DEPTH ) {
            error_report( ctxt, JSON_STATUS_OUT_OF_MEMORY,
                            "ran out of allocated stack depth in processing array\n");
            goto error_exit;
        }
        vdata.array = make_array( ctxt );
        if ( NULL == vdata.array ) goto error_exit;
        --ctxt->open_stack;
        break;
    case '"':
        vtype = JSON_STRING;
        vdata.string = make_string( ctxt );
        if ( NULL == vdata.string ) goto error_exit;
        break;
    default:
        if ( EOF == c ) {
            error_report( ctxt, JSON_STATUS_PARSE_SYNTAX_ERROR,
                          "Syntax error (end of text) while expecting value");
            goto error_exit;
        }
        if ( ! isdigit( c ) ) {
            wrong_char_error_report( ctxt, "while expecting value", c );
            goto error_exit;
        } // else falls though number case (next line needed to silence gcc)
        // fall through
    case '-':
        vtype = JSON_NUMBER;
        ctxt->source.push_back( &ctxt->source, c ); // backtrack 1 char
        vdata.number = make_number( ctxt );
        if ( NULL == vdata.number ) goto error_exit;
        break;
    case 't': case 'f':
        vtype = JSON_BOOLEAN;
        boolean = make_boolean( ctxt, c );
        if ( -1 == boolean ) goto error_exit;
        vdata.boolean = (bool)boolean;
        break;
    case 'n':
        vtype = JSON_NULL;
        if ( ! check_litteral( ctxt, (const unsigned char *)"null" ) )
            goto error_exit;
        vdata.string = NULL; // any pointer would work here
        break;
    }
    value->vtype = vtype;
    value->vdata = vdata;
    return value;

error_exit:
    free( value );
    return NULL;
}

extern json_value_t *json_parse_source( json_source_t *source,
                                 bool comments, json_error_report_t *error )
{
    json_parse_ctxt_t ctxt;
    ctxt.source.src = source->src;
    ctxt.source.get = source->get;
    ctxt.source.push_back = source->push_back;
    ctxt.comments = comments;
    ctxt.line = 1;
    ctxt.estring[0] = 0;
    ctxt.ecode = JSON_STATUS_SUCCESS;
    ctxt.open_stack = 0;

    json_value_t *value = make_value( &ctxt );
    if ( error ) {
        error->status = ctxt.ecode;
        if ( ctxt.estring[0] )
            error->error_string = strdup( ctxt.estring );
        else
            error->error_string = NULL;
    }
    return value;
}

static json_value_t *json_parse_data( json_parse_ctxt_t *ctxt )
{
    json_value_t *value = make_value( ctxt );
    if ( value) {
        int c = skip_blank( ctxt );
        if( c != EOF ) {
            json_free( value );
            value = NULL;
            wrong_char_error_report( ctxt, "while expecting end of text", c );
        }
    }
    return value;
}

static int get_next_stream_char( json_source_t *source )
{
    return fgetc( (FILE *)(source->src) );
}

static void push_back_stream_char( json_source_t *source, int c )
{
    if ( EOF == c ) return;
    ungetc( c, (FILE *)(source->src) );
}

extern json_value_t *json_parse_stream( FILE *fd, bool comments, json_error_report_t *error )
{
    json_parse_ctxt_t ctxt;
    ctxt.source.src = fd;
    ctxt.source.get = get_next_stream_char;
    ctxt.source.push_back = push_back_stream_char;
    ctxt.comments = comments;
    ctxt.line = 1;
    ctxt.estring[0] = 0;
    ctxt.ecode = JSON_STATUS_SUCCESS;
    ctxt.open_stack = 0;

    json_value_t *value = json_parse_data( &ctxt );
    if ( NULL == value && JSON_STATUS_SUCCESS == ctxt.ecode ) {
        error_report( &ctxt, JSON_STATUS_INVALID_PARAMETERS, "Empty source stream\n" );
    }
    if ( error ) {
        error->status = ctxt.ecode;
        if ( ctxt.estring[0] )
            error->error_string = strdup( ctxt.estring );
        else
            error->error_string = NULL;
    }
    return value;
}

static int get_next_buffer_char( json_source_t *source )
{
    unsigned char *ptr = source->src;
    if ( 0 == *ptr ) return EOF;  // stays on end of file

    int c = *ptr;
    source->src = ++ptr;
    return c;
}

static void push_back_buffer_char( json_source_t *source, int c )
{
    if ( EOF == c ) return;
    unsigned char *ptr = source->src;
    source->src = --ptr;
}

extern json_value_t *json_parse_buffer( const unsigned char *buffer,
                                        bool comments,
                                        json_error_report_t *error )
{
    json_parse_ctxt_t ctxt;
    ctxt.source.src = (void *)buffer;
    ctxt.source.get = get_next_buffer_char;
    ctxt.source.push_back = push_back_buffer_char;
    ctxt.comments = comments;
    ctxt.line = 1;
    ctxt.estring[0] = 0;
    ctxt.ecode = JSON_STATUS_SUCCESS;
    ctxt.open_stack = 0;

    json_value_t *value;
    if ( buffer ) {
        value = json_parse_data( &ctxt );
    } else {
        value = NULL;
        error_report( &ctxt, JSON_STATUS_INVALID_PARAMETERS, "Empty source buffer\n" );
    }

    if ( error ) {
        error->status = ctxt.ecode;
        if ( ctxt.estring[0] )
            error->error_string = strdup( ctxt.estring );
        else
            error->error_string = NULL;
    }
    return value;
}
