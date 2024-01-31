
#ifndef __JSONPARSE_H__
#define __JSONPARSE_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "jsonvalue.h"

typedef struct {
    char          *error_string;
    json_status_t status;
} json_error_report_t;

#define MAX_OPEN_DEPTH  256  // limit number of open arrays & objects to
                             // prevent potential stack overflow in parser.

/* parse the given json text given as const char buffer (zero terminated UTF8
   characters), accepting C/C++ comments only if the argument comments is true.

   It returns the parsed value in case of success, or NULL in case of parsing
   (or allocation) error. In that case, if the argument error is not NULL, a
   json_error_report is filled and returned. The error string is allocated on
   the heap and must be freed by the caller after use

   The returned value must also be freed by calling json_free  after use,
   otherwise memory will be leaked. After parsing, the souce buffer can also be
   freed if appropriate (useful text content has been duplicated on the heap).
*/

extern json_value_t *json_parse_buffer( const unsigned char *buffer,
                                        bool comments,
                                        json_error_report_t *error );

/* Same as above, but directly from a file, pipe or terminal input */
extern json_value_t *json_parse_stream( FILE *fd, bool comments,
                                        json_error_report_t *error );

/* the underlying common interface for any type of data parser */
typedef struct _json_source json_source_t;

typedef int (*get_next_char_fct)( json_source_t *source );
typedef void (*push_back_char_fct)( json_source_t *source, int c );

struct _json_source {
    void               *src;
    get_next_char_fct  get;        // return EOF (stdio.h) if not more data
    push_back_char_fct push_back;
};

extern json_value_t *json_parse_source( json_source_t *source,
                                        bool comments,
                                        json_error_report_t *error );

/* free the json tree passed as root */
extern void json_free( json_value_t *root );

#endif /* __JSONPARSE_H__ */
