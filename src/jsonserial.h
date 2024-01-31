
#ifndef __JSONSERIAL_H__
#define __JSONSERIAL_H__

#include <stdio.h>
#include <stddef.h>
#include "jsonvalue.h"

/* get serialization size, depending on the serialization format, from the
   json value passed. The possible formats are PACKED_FORMAT (no space, no
   new line) or PRETTY_FORMAT (added new lines and spaces, indentation) or
   SYNTAX_HIGHLIGHT, similar to PRETTY_FORMAT as far as get_serialization
   is concerned. SYNTAX_HIGHLIGHT is only meaningful when used with json_print
   where it controls json syntax highlighting.

   The terminating zero is not counted in the serialization size. */
typedef enum {
    PACKED_FORMAT, PRETTY_FORMAT = 1, SYNTAX_HIGHLIGHT = 3
} json_serialize_t;

extern size_t json_get_serialization_length( const json_value_t *value,
                                             json_serialize_t format );

/* Serialize a json tree, depending on the serialization format (where
   SYNTAX_HIGHLIGHT is treated as PRETTY_FORMAT), from the json value passed.
   The output buffer must be allocated and its size given as max_size. It is
   expected that json_get_serialization_length has been called and a buffer for
   the returned length, + 1 for the terminating zero, has been allocated.

   The return value is the actual length necessary for storing the serialized
   tree, excluding the terminating zero. Note that at most max_size bytes are
   used to fill up the buffer and the result may be truncated, in which case
   the returned value is larger than or equal to the value passed as max_size.
   Note that the returned value is the always the same as the value returned by
   json_get_serialization_length */
extern size_t json_serialize( const json_value_t *value,
                              json_serialize_t format,
                              size_t max_size, char *buffer );

typedef enum {
    NORMAL, BOLD, FAINT, ITALIC, UNDERLINE
} json_highlight_mode_t;

typedef enum {
    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
} json_highlight_color_t;

typedef struct {
    json_value_type_t      vtype;   // use NOT_A_VALUE for assignment mark (':')
    json_highlight_mode_t  mode;
    json_highlight_color_t color;
} json_highlight_definition_t;      // missing definitions use default values

/* default highlight values:
    NOT_A_VALUE  BOLD   YELLOW
    JSON_NULL    NORMAL MAGENTA
    JSON_BOOLEAN NORMAL GREEN
    JSON_NUMBER  NORMAL BLUE
    JSON_STRING  NORMAL CYAN
    JSON_ARRAY   BOLD   RED
    JSON_OBJECT  BOLD   WHITE */

typedef struct {
    unsigned int                nb_entries;
    json_highlight_definition_t definitions[]; // tail-padded
} json_highlight_t;

#define sizeof_higlight( _nb ) ( sizeof(json_highlight_t) + \
                                ( _nb * sizeof(json_highlight_definition_t) ) )

/* Serialize a json tree, depending on the serialization format (where
   SYNTAX_HIGHLIGH is used to trigger ANSI escape sequences to control the
   character colors), from the json value passed. The file descriptor fd must
   be opened before calling. SYNTAX_HIGHLIGHTING should not be used when
   the stored file is intended for exchanging JSON data, as it adds non-json
   characters to the output.

   The return value is the number of written bytes. */
extern size_t json_print( const json_value_t *value, json_serialize_t format,
                          json_highlight_t *highlight, FILE *fd );

#endif /* __JSONSERIAL_H__ */
