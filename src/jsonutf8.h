
#ifndef __JSONUTF8_H__
#define __JSONUTF8_H__

#include <stdint.h>
#include <stdbool.h>

/* UTF8 encoding/decoding */
typedef uint32_t ucs4_t;

struct _json_data_input;

typedef int (*json_read_byte_fct)( struct _json_data_input *data_input );
typedef struct _json_data_input {
    void               *ctxt;
    json_read_byte_fct read_byte;
} json_data_input_t;

/* Check if the input text is valid UTF8 encoding, one code point at a time.
   Return the number of bytes read if it is valid or return 0 otherwise */
extern unsigned int json_check_utf8( json_data_input_t *input );

/* Check if the input string is valid UTF8 encoding, up to the terminating 0 */
extern bool json_is_utf8_string( const unsigned char *string );

/* encode the ucs4 unicode val into UTF8. If the passed ucs4 unicode val can
   be encoded in UTF8, the buffer pointed to output is filled with the
   corresponding sequence of bytes, output is updated to point to the first
   following byte and 0 is returned. If the value val cannot be  encoded in
   UTF8, -1 is returned. Note that output must have room for at least 4 bytes */
extern int json_output_utf8( ucs4_t val, unsigned char **output );

#endif /* __JSONUTF8_H__ */
