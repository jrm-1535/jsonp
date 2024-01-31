
#include <stdio.h>
#include "jsonutf8.h"

/* variable length 8 bit byte encoding */

#define IS_UTF8_1(_b)     (!((_b)&0x80) )
#define IS_UTF8_2(_b)     ( ((_b)&0xe0) == 0xc0 )
#define IS_UTF8_3(_b)     ( ((_b)&0xf0) == 0xe0 )
#define IS_UTF8_4(_b)     ( ((_b)&0xf8) == 0xf0 )

#define IS_UTF8_TAIL(_b)  ( ((_b)&0xc0) == 0x80 )

extern unsigned int json_check_utf8( json_data_input_t *input )
{
    int constraint = 0;
    int remaining = 4;
    unsigned int nb_read = 0;

    while ( 1 ) {
        int data = input->read_byte( input );
        if ( EOF == data )                                  return 0;
        ++nb_read;
        unsigned char byte = data;
        if ( 4 == remaining ) {    // first byte
            if ( IS_UTF8_1(byte) ) {
                remaining = 0;     // no more to
                break;             // exit while loop here
            }
            if ( IS_UTF8_2(byte) ) {
                if( 0xc2 > byte )  /* illegal 1st byte */   return 0;
                remaining = 1;     // 1 more to go
            } else if ( IS_UTF8_3(byte) ) {
                switch (  byte ) {
                case 0xe0: constraint = 1; break;
                case 0xed: constraint = 2; break;
                }
                remaining = 2;     // 2 more to go
            } else if ( IS_UTF8_4(byte) ) {
                switch ( byte ) {
                case 0xf0: constraint = 3; break;
                case 0xf1: case 0xf2: case 0xf3: break;
                case 0xf4: constraint = 4; break;
                default:                                    return 0;
                }
                remaining = 3;     // 3 more to go
            } else                 /* illegal first byte */ return 0;

        } else if ( IS_UTF8_TAIL(byte) ) {
            switch ( constraint ) {
            default:
                break;
            case 1:
                if ( byte < 0xa0 )                          return 0;
                constraint = 0;
                break;
            case 2:
                if ( (byte < 0x80 ) || (byte > 0x9f) )      return 0;
                constraint = 0;
                break;
            case 3:
                if ( byte < 0x90 )                          return 0;
                constraint = 0;
                break;
            case 4:
                if ( (byte < 0x80 ) || (byte > 0x8f) )      return 0;
                constraint = 0;
                break;
            }
            if ( 0 == --remaining )
                break;            // exit loop here
        } else                                              return 0;
    }
    if ( remaining )             /* incomplete utf8 */      return 0;
    return nb_read;
}

static int get_data_from_string( json_data_input_t *data_input )
{
    const unsigned char *string = data_input->ctxt;
    int data = *string;
    if ( data ) {
        data_input->ctxt = (unsigned char *)(1 + string);
        return data;
    }
    return EOF;
}

// in memory string
extern bool json_is_utf8_string( const unsigned char *string )
{
    json_data_input_t input;
    input.ctxt = (unsigned char *)string;
    input.read_byte = get_data_from_string;

    while ( *((const unsigned char *)input.ctxt) ) {
        if ( 0 == json_check_utf8( &input ) ) {
            return false;
        }
    }
    return true;
}

/* ouput must have room for at least 4 bytes */
extern int json_output_utf8( ucs4_t val, unsigned char **output )
{
  unsigned char *p = *output;

  if ( val < 0x0080U ) { /* 1 byte encoding */
    *p = (unsigned char)val;
  } else if ( val < 0x0800U) { /* 2 byte encoding */
    *p++ = 0xc0 + (val >> 6);
    *p = 0x80 + (val & 0x3f);
  } else if ( val < 0x010000U ) { /* 3 byte encoding */
    if ( ( val >= 0xd800 ) && ( val < 0xe000 ) )
      return -1; /* isolated surrogates cannot be encoded in UTF8 */
    *p++ = 0xe0 + (val >> 12 );
    *p++ = 0x80 + ((val >> 6) & 0x3f );
    *p = 0x80 + (val & 0x3f);
  } else if ( val < 0x0110000 ) { /* 4 byte encoding */
    *p++ = 0xf0 + (val >> 18);
    *p++ = 0x80 + ((val >> 12) & 0x3f);
    *p++ = 0x80 + ((val >> 6) & 0x3f);
    *p = 0x80 + (val & 0x3f);
  } else
    return -1; /* anything else cannot be encoded in UTF8 */

  *output = p+1;
  return 0;
}
