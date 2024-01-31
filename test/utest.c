
#include <stdio.h>

#include "jsonvalue.h"
#include "jsonparse.h"
#include "jsonedit.c"
#include "jsonserial.h"

#include "utest.h"

START_TEST( test_parser_null, NO_SETUP )

    unsigned char buffer[] = "null";

    json_value_t *root = json_parse_buffer( buffer, 0, NULL );
    ASSERT_DIFFERENT( NULL, root );
    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NULL, value_type );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_nuke, NO_SETUP )

    unsigned char buffer[] = "nuke";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_foo, NO_SETUP )

    unsigned char buffer[] = "foo";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_tuxedo, NO_SETUP )

    unsigned char buffer[] = "tuxedo";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_zero, NO_SETUP )

    unsigned char buffer[] = "zero";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_false, NO_SETUP )

    unsigned char buffer[] = "false";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    bool boolean_value = json_get_boolean_value( root );
    ASSERT_EQUAL( false, boolean_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_true, NO_SETUP )

    unsigned char buffer[] = "true";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    bool boolean_value = json_get_boolean_value( root );
    ASSERT_EQUAL( true, boolean_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_string, NO_SETUP )

    unsigned char buffer[] = "\"a short character string\"";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *string_value = json_get_string_value( root );
    ASSERT_EQUAL( 0, strcmp("a short character string", (const char *) string_value) );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_string, NO_SETUP )

    unsigned char buffer[] = "\"";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_INVALID_STRING, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_empty_string, NO_SETUP )

    unsigned char buffer[] = "\"\"";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *string_value = json_get_string_value( root );
    ASSERT_EQUAL( 0, strcmp("", (const char *) string_value) );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_integer_1, NO_SETUP )

    unsigned char buffer[] = "123";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int integer_value = json_get_integer_value( root );
    ASSERT_EQUAL( 123, integer_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_integer_2, NO_SETUP )

    unsigned char buffer[] = "-123";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int integer_value = json_get_integer_value( root );
    ASSERT_EQUAL( -123, integer_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_integer_3, NO_SETUP )

    unsigned char buffer[] = "-123    "; // space at the end is OK
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int integer_value = json_get_integer_value( root );
    ASSERT_EQUAL( -123, integer_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_integer_1, NO_SETUP )

    unsigned char buffer[] = "+123"; // + sign is not acceptable
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_integer_2, NO_SETUP )

    unsigned char buffer[] = "--123"; // -- is not acceptable
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_integer_3, NO_SETUP )

    unsigned char buffer[] = "-+123"; // -+ is not acceptable
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_integer_4, NO_SETUP )

    unsigned char buffer[] = "123 !"; // ! after number is not acceptable
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_integer_5, NO_SETUP )

    unsigned char buffer[] = "00123"; // no leading zero is acceptable
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_integer_6, NO_SETUP )

    unsigned char buffer[] = "23456789012E66"; // very large integer
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_DIFFERENT( JSON_INTEGER_NUMBER, number_type );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_real_1, NO_SETUP )

    unsigned char buffer[] = "0.456";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double real_value = json_get_real_value( root );
    ASSERT_EQUAL( 0.456, real_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_real_2, NO_SETUP )

    unsigned char buffer[] = "1.234e3";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    // 1.234e3 is actually 1234, which is considered as an integer
    // note that the ECMA-104 spec. does not differenciate numbers
    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int int_value = json_get_integer_value( root );
    ASSERT_EQUAL( 1234, int_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_real_3, NO_SETUP )

    unsigned char buffer[] = "1.234E+2";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double real_value = json_get_real_value( root );
    ASSERT_EQUAL( 123.4, real_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_real_4, NO_SETUP )

    unsigned char buffer[] = "12.34E-1";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double real_value = json_get_real_value( root );
    ASSERT_EQUAL( 1.234, real_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_real_5, NO_SETUP )

    unsigned char buffer[] = "-123456.789012345678"; // just within double precision
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double real_value = json_get_real_value( root );
    double ref_value = -123456.789012345678;
    ASSERT_EQUAL( ref_value, real_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_real_6, NO_SETUP )

    unsigned char buffer[] = "0.9007199254740993"; // beyond double precision
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double real_value = json_get_real_value( root );
    double ref_value = 0.9007199254740993;
    ASSERT_EQUAL( ref_value, real_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_real_7, NO_SETUP )

    unsigned char buffer[] = "23456789012E66"; // very large integer
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( root );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double large_value = json_get_real_value( root );
    double expected_value = 23456789012E66;

    ASSERT_EQUAL( expected_value, large_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_real_1, NO_SETUP )

    unsigned char buffer[] = "123.e"; // missing digit after ,
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_real_2, NO_SETUP )

    unsigned char buffer[] = "123.456e"; // missing exponent
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_real_3, NO_SETUP )

    unsigned char buffer[] = "123.456E+"; // missing exponent
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_real_4, NO_SETUP )

    unsigned char buffer[] = "123.456E2+"; // exponent followed by sign
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_empty_array, NO_SETUP )

    unsigned char buffer[] = "[]";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( root );
    ASSERT_EQUAL( 0, array_size );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_array_1, NO_SETUP )

    unsigned char buffer[] = "[";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_array_2, NO_SETUP )

    unsigned char buffer[] = "[[]";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_array_3, NO_SETUP )

    unsigned char buffer[] = "[ 123 \"text\" ]";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_array_4, NO_SETUP )

    unsigned char buffer[] = "[ 123, \n ]";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_array_5, NO_SETUP )

    unsigned char buffer[] = "[ true  ; ]";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_boolean_string_array, NO_SETUP )

    unsigned char buffer[] = "[ true, \"a valid string\" ]";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( root );
    ASSERT_EQUAL( 2, array_size );

    const json_value_t *element = json_get_array_element( root, 0 );
    ASSERT_DIFFERENT( NULL, element );

    value_type = json_get_value_type( element );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    bool boolean_value = json_get_boolean_value( element );
    ASSERT_EQUAL( true, boolean_value );

    element = json_get_array_element( root, 1 );
    ASSERT_DIFFERENT( NULL, element );

    value_type = json_get_value_type( element );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *string_value = json_get_string_value( element );
    ASSERT_EQUAL( 0, strcmp("a valid string", (const char *)string_value) );

    element = json_get_array_element( root, 2 );
    ASSERT_EQUAL( NULL, element );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_small_int_array, NO_SETUP )

    int data[] =              { 10, 20, 30, 40, 50, 60, 70, 80, 90 };
    unsigned char buffer[] = "[ 10, 20, 30, 40, 50, 60, 70, 80, 90 ]";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( root );
    ASSERT_EQUAL( sizeof( data)/sizeof(int), array_size );

    for ( unsigned int i = 0; i < array_size; ++i ) {
        const json_value_t *element = json_get_array_element( root, i );
        ASSERT_DIFFERENT( NULL, element );

        value_type = json_get_value_type( element );
        ASSERT_EQUAL( JSON_NUMBER, value_type );

        json_number_type_t number_type = json_get_value_number_type( element );
        ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

        long long int integer_value = json_get_integer_value( element );
        ASSERT_EQUAL( data[i], integer_value );
    }

END_TEST( json_free_value( root ) )


START_TEST( test_parser_larger_int_array, NO_SETUP )

    int data[] =              { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                                15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
                                27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                                39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
                                51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62 };
    unsigned char buffer[] = "[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,  \
                                15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, \
                                27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, \
                                39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, \
                                51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62 ]";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( root );
    ASSERT_EQUAL( sizeof( data)/sizeof(int), array_size );

    for ( unsigned int i = 0; i < array_size; ++i ) {
        const json_value_t *element = json_get_array_element( root, i );
        ASSERT_DIFFERENT( NULL, element );

        value_type = json_get_value_type( element );
        ASSERT_EQUAL( JSON_NUMBER, value_type );

        json_number_type_t number_type = json_get_value_number_type( element );
        ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

        long long int integer_value = json_get_integer_value( element );
        ASSERT_EQUAL( data[i], integer_value );
    }

END_TEST( json_free_value( root ) )


START_TEST( test_parser_empty_object, NO_SETUP )

    unsigned char buffer[] = "{}";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int count = json_get_object_member_count( root );
    ASSERT_EQUAL( 0, count );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_object_member, NO_SETUP )

    unsigned char buffer[] = "{ \"\": 123 }"; // empty member name is legal
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int count = json_get_object_member_count( root );
    ASSERT_EQUAL( 1, count );

    const json_value_t *number = json_search_for_object_member_by_name( root,
                                                  (const unsigned char *)"" );
    ASSERT_DIFFERENT( NULL, number );

    value_type = json_get_value_type( number );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( number );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int integer_value = json_get_integer_value( number );
    ASSERT_EQUAL( 123, integer_value );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_object_1, NO_SETUP )

    unsigned char buffer[] = "{";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_object_2, NO_SETUP )

    unsigned char buffer[] = "{{}";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_object_3, NO_SETUP )

    unsigned char buffer[] = "{ \"first-member\": 123 \"text\" }";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_object_4, NO_SETUP )

    unsigned char buffer[] = "{ \"first-member\": 123, }";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_bad_object_5, NO_SETUP )

    unsigned char buffer[] = "{ \"first-member\": 123 - }";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_EQUAL( NULL, root );
    ASSERT_DIFFERENT( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( NOT_A_JSON_VALUE, value_type );

    ASSERT_EQUAL( JSON_STATUS_PARSE_SYNTAX_ERROR, error.status );
    PRINT_NORMAL( "Expected error: \"%s\"\n", error.error_string );
    free( error.error_string );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_null_int_object, NO_SETUP )

    unsigned char buffer[] = "{ \"null-member\"    : null,\n"
                             "  \"int-member\":      1000}";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int number = json_get_object_member_count( root );
    ASSERT_EQUAL( 2, number );

    const json_value_t *member = json_search_for_object_member_by_name(
                                      root, (const unsigned char *)"null-member" );
    ASSERT_DIFFERENT( NULL, member );

    value_type = json_get_value_type( member );
    ASSERT_EQUAL( JSON_NULL, value_type );

    member = json_search_for_object_member_by_name(
                                      root, (const unsigned char *)"int-member" );
    ASSERT_DIFFERENT( NULL, member );

    value_type = json_get_value_type( member );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( member );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int int_value = json_get_integer_value( member );
    ASSERT_EQUAL( 1000, int_value );

    member = json_search_for_object_member_by_name(
                                      root, (const unsigned char *)"real-member" );
    ASSERT_EQUAL( NULL, member );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_array_object_object, NO_SETUP )

    unsigned char buffer[] = "{ \"array-member\"    : [ ],\n"
                             "  \"object-member\":      {}}";
    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int number = json_get_object_member_count( root );
    ASSERT_EQUAL( 2, number );

    const json_value_t *member = json_search_for_object_member_by_name(
                                      root, (const unsigned char *)"array-member" );
    ASSERT_DIFFERENT( NULL, member );

    value_type = json_get_value_type( member );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( member );
    ASSERT_EQUAL( 0, array_size );

    member = json_search_for_object_member_by_name(
                                      root, (const unsigned char *)"object-member" );
    ASSERT_DIFFERENT( NULL, member );

    value_type = json_get_value_type( member );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int count = json_get_object_member_count( member );
    ASSERT_EQUAL( 0, count );

    member = json_search_for_object_member_by_name(
                                      root, (const unsigned char *)"other-member" );
    ASSERT_EQUAL( NULL, member );

END_TEST( json_free_value( root ) )

START_TEST( test_parser_larger_object, NO_SETUP )

    unsigned char buffer[] = "{ \"member #1\": 1, \"member #2\": 2, \
                                \"member #3\": 3, \"member #4\": 4, \
                                \"member #5\": 5, \"member #6\": 6, \
                                \"member #7\": 7, \"member #8\": 8, \
                                \"member #9\": 9, \"member #10\": 10, \
                                \"member #11\": 11, \"member #12\": 12, \
                                \"member #13\": 13, \"member #14\": 14, \
                                \"member #15\": 15, \"member #16\": 16, \
                                \"member #17\": 17, \"member #18\": 18, \
                                \"member #19\": 19, \"member #20\": 20, \
                                \"member #21\": 21, \"member #22\": 22, \
                                \"member #23\": 23, \"member #24\": 24, \
                                \"member #25\": 25, \"member #26\": 26, \
                                \"member #27\": 27, \"member #28\": 28, \
                                \"member #29\": 29, \"member #30\": 30, \
                                \"member #31\": 31, \"member #32\": 32, \
                                \"member #33\": 33, \"member #34\": 34, \
                                \"member #35\": 35, \"member #36\": 36, \
                                \"member #37\": 37, \"member #38\": 38, \
                                \"member #39\": 39, \"member #40\": 40, \
                                \"member #41\": 41, \"member #42\": 42, \
                                \"member #43\": 43, \"member #44\": 44, \
                                \"member #45\": 45, \"member #46\": 46, \
                                \"member #47\": 47, \"member #48\": 48, \
                                \"member #49\": 49, \"member #50\": 50, \
                                \"member #51\": 51, \"member #52\": 52, \
                                \"member #53\": 53, \"member #54\": 54 }";

    json_error_report_t error;

    json_value_t *root = json_parse_buffer( buffer, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    json_value_type_t value_type = json_get_value_type( root );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int count = json_get_object_member_count( root );
    ASSERT_EQUAL( 54, count );

    char member_name[ sizeof("member #54") ];
    memcpy( member_name, "member #", sizeof("member #") );
    int index = sizeof( "member #" ) - 1;

    for ( unsigned int i = 0; i < count; ++i ) {
        snprintf( &member_name[index], 3, "%u", 1 + i );
        //PRINT_NORMAL( "member name: %s\n", member_name );
        const json_value_t *member = json_search_for_object_member_by_name(
                                      root, (const unsigned char *)member_name );
        ASSERT_DIFFERENT( NULL, member );

        value_type = json_get_value_type( member );
        ASSERT_EQUAL( JSON_NUMBER, value_type );

        json_number_type_t number_type = json_get_value_number_type( member );
        ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

        long long int integer_value = json_get_integer_value( member );
        ASSERT_EQUAL( 1 + i, integer_value );
    }

END_TEST( json_free_value( root ) )
// ===============================================================

START_TEST( test_new_null, NO_SETUP )

    json_value_t *new_null = json_new_value( JSON_NULL );
    ASSERT_DIFFERENT( NULL, new_null );
//    ASSERT( new_null != new_null );

    json_value_type_t value_type = json_get_value_type( new_null );
    ASSERT_EQUAL( JSON_NULL, value_type );

END_TEST( json_free_value( new_null ) )

START_TEST( test_new_boolean, NO_SETUP )

    json_value_t *new_boolean = json_new_value( JSON_BOOLEAN, true );
    ASSERT_DIFFERENT( NULL, new_boolean );

    json_value_type_t value_type = json_get_value_type( new_boolean );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    bool boolean_value = json_get_boolean_value( new_boolean );
    ASSERT_EQUAL( true, boolean_value );

END_TEST( json_free_value( new_boolean ) )

START_TEST( test_new_string, NO_SETUP )

    json_value_t *new_string = json_new_value( JSON_STRING, "Hello json" );
    ASSERT_DIFFERENT( NULL, new_string );

    json_value_type_t value_type = json_get_value_type( new_string );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *string_value = json_get_string_value( new_string );
    ASSERT_EQUAL( 0, strcmp("Hello json", (const char *)string_value) );

END_TEST( json_free_value( new_string ) )

START_TEST( test_new_integer, NO_SETUP )

    json_value_t *new_int = json_new_value( JSON_NUMBER, JSON_INTEGER_NUMBER, 17 );
    ASSERT_DIFFERENT( NULL, new_int );

    json_value_type_t value_type = json_get_value_type( new_int );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( new_int );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int int_value = json_get_integer_value( new_int );
    ASSERT_EQUAL( 17, int_value );

END_TEST( json_free_value( new_int ) )

START_TEST( test_new_real, NO_SETUP )

    json_value_t *new_real = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, 23.5 );
    ASSERT_DIFFERENT( NULL, new_real );

    json_value_type_t value_type = json_get_value_type( new_real );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( new_real );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double real_value = json_get_real_value( new_real );
    ASSERT_EQUAL( 23.5, real_value );

END_TEST( json_free_value( new_real ) )

START_TEST( test_new_array, NO_SETUP )

    json_value_t *new_array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, new_array );

    json_value_type_t value_type = json_get_value_type( new_array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( new_array );
    ASSERT_EQUAL( 0, array_size );

END_TEST( json_free_value( new_array ) )

START_TEST( test_new_object, NO_SETUP )

    json_value_t *new_object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, new_object );

    json_value_type_t value_type = json_get_value_type( new_object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int member_count = json_get_object_member_count( new_object );
    ASSERT_EQUAL( 0, member_count );

END_TEST( json_free_value( new_object ) )

START_TEST( test_duplicate_null, NO_SETUP )

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );

    json_value_t *duplicate_null = json_duplicate_value( new_null );
    ASSERT_DIFFERENT( NULL, duplicate_null );

    json_value_type_t value_type = json_get_value_type( duplicate_null );
    ASSERT_EQUAL( JSON_NULL, value_type );

END_TEST( json_free_value( new_null ); json_free_value( duplicate_null ) )

START_TEST( test_duplicate_boolean, NO_SETUP )

    json_value_t *new_boolean = json_new_value( JSON_BOOLEAN, false );
    ASSERT_DIFFERENT( NULL, new_boolean );

    json_value_t *duplicate_boolean = json_duplicate_value( new_boolean );
    ASSERT_DIFFERENT( NULL, duplicate_boolean );

    json_value_type_t value_type = json_get_value_type( duplicate_boolean );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    bool boolean_value = json_get_boolean_value( duplicate_boolean );
    ASSERT_EQUAL( false, boolean_value );

END_TEST( json_free_value( new_boolean ); json_free_value( duplicate_boolean ) )

START_TEST( test_duplicate_string, NO_SETUP )

    json_value_t *new_string = json_new_value( JSON_STRING, "Hello again json" );
    ASSERT_DIFFERENT( NULL, new_string );

    json_value_t *duplicate_string = json_duplicate_value( new_string );
    ASSERT_DIFFERENT( NULL, duplicate_string );

    json_value_type_t value_type = json_get_value_type( duplicate_string );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *string_value = json_get_string_value( duplicate_string );
    ASSERT_EQUAL( 0, strcmp("Hello again json", (const char *)string_value) );

END_TEST( json_free_value( new_string ); json_free_value( duplicate_string ) )

START_TEST( test_duplicate_integer, NO_SETUP )

    json_value_t *new_int = json_new_value( JSON_NUMBER, JSON_INTEGER_NUMBER, -17LL );
    ASSERT_DIFFERENT( NULL, new_int );

    json_value_t *duplicate_int = json_duplicate_value( new_int );
    ASSERT_DIFFERENT( NULL, duplicate_int );

    json_value_type_t value_type = json_get_value_type( duplicate_int );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( duplicate_int );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int int_value = json_get_integer_value( duplicate_int );
    ASSERT_EQUAL( -17, int_value );

END_TEST( json_free_value( new_int ); json_free_value( duplicate_int ) )

START_TEST( test_duplicate_real, NO_SETUP )

    json_value_t *new_real = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, 1e3 );
    ASSERT_DIFFERENT( NULL, new_real );

    json_value_t *duplicate_real = json_duplicate_value( new_real );
    ASSERT_DIFFERENT( NULL, duplicate_real );

    json_value_type_t value_type = json_get_value_type( duplicate_real );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( duplicate_real );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double real_value = json_get_real_value( duplicate_real );
    ASSERT_EQUAL( 1e3, real_value );

END_TEST( json_free_value( new_real ); json_free_value( duplicate_real ) )

START_TEST( test_duplicate_empty_array, NO_SETUP )

    json_value_t *new_array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, new_array );

    json_value_t *duplicate_array = json_duplicate_value( new_array );
    ASSERT_DIFFERENT( NULL, duplicate_array );

    json_value_type_t value_type = json_get_value_type( duplicate_array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( duplicate_array );
    ASSERT_EQUAL( 0, array_size );

END_TEST( json_free_value( new_array ); json_free_value( duplicate_array ) )

START_TEST( test_duplicate_empty_object, NO_SETUP )

    json_value_t *new_object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, new_object );

    json_value_t *duplicate_object = json_duplicate_value( new_object );
    ASSERT_DIFFERENT( NULL, duplicate_object );

    json_value_type_t value_type = json_get_value_type( duplicate_object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int member_count = json_get_object_member_count( duplicate_object );
    ASSERT_EQUAL( 0, member_count );

END_TEST( json_free_value( new_object ); json_free_value( duplicate_object ) )

START_TEST( test_array_insert, NO_SETUP )

    json_value_t *array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, array );

    json_value_type_t value_type = json_get_value_type( array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );

    json_status_t status = json_insert_element_into_array( array, -1, new_null );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 10, new_null );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 1, new_null );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 0, new_null );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    // try inserting the same value twice => probable crash...

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    json_value_t *new_boolean = json_new_value( JSON_BOOLEAN, true );
    ASSERT_DIFFERENT( NULL, new_boolean );

    status = json_insert_element_into_array( array, 2, new_boolean );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 1, new_boolean );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_string = json_new_value( JSON_STRING, "Hello json" );
    ASSERT_DIFFERENT( NULL, new_string );

    status = json_insert_element_into_array( array, 3, new_string );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 0, new_string );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_int = json_new_value( JSON_NUMBER, JSON_INTEGER_NUMBER, 17 );
    ASSERT_DIFFERENT( NULL, new_int );

    status = json_insert_element_into_array( array, 4, new_int );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 1, new_int );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_real = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, 23.5 );
    ASSERT_DIFFERENT( NULL, new_real );

    status = json_insert_element_into_array( array, 5, new_real );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 1, new_real );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, new_array );

    status = json_insert_element_into_array( array, 6, new_array );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 4, new_array );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, new_object );

    status = json_insert_element_into_array( array, 7, new_object );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 6, new_object );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 7, array_size );

    json_array_iterator_t iterator = json_new_array_iterator( array );
    ASSERT_DIFFERENT( NULL, iterator );

    unsigned int count = 0;
    const json_value_t *value; // direct pointer to the array content (not a copy)

    const unsigned char *string_value;
    json_number_type_t number_type;
    long long int int_value;
    double real_value;
    bool boolean_value;
    unsigned int new_array_size;
    unsigned int new_member_count;

    while ( ( value = json_iterate_array_element( &iterator ) ) ) {
//        PRINT_NORMAL("count = %d\n", count );
        json_value_type_t value_type = json_get_value_type( value );
        switch ( count ) {
        case 0: // should be string
            ASSERT_EQUAL( JSON_STRING, value_type );
            string_value = json_get_string_value( value );
            ASSERT_EQUAL( 0, strcmp("Hello json", (const char *)string_value) );
            break;
        case 1: // should be real
            ASSERT_EQUAL( JSON_NUMBER, value_type );
            number_type = json_get_value_number_type( value );
            ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );
            real_value = json_get_real_value( value );
            ASSERT_EQUAL( 23.5, real_value );
            break;
        case 2:  // should be int
            ASSERT_EQUAL( JSON_NUMBER, value_type );
            number_type = json_get_value_number_type( value );
            ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );
            int_value = json_get_integer_value( value );
            ASSERT_EQUAL( 17, int_value );
            break;
        case 3: // should be null
            ASSERT_EQUAL( JSON_NULL, value_type );
            break;
        case 4: // ahould ne ab empty array
            ASSERT_EQUAL( JSON_ARRAY, value_type );
            new_array_size = json_get_array_size( new_array );
            ASSERT_EQUAL( 0, new_array_size );
            break;
        case 5: // should be boolean
            ASSERT_EQUAL( JSON_BOOLEAN, value_type );
            boolean_value = json_get_boolean_value( value );
            ASSERT_EQUAL( true, boolean_value );
            break;
        case 6: // should be an empty object
            ASSERT_EQUAL( JSON_OBJECT, value_type );
            new_member_count = json_get_object_member_count( new_object );
            ASSERT_EQUAL( 0, new_member_count );
            break;
        }
        ++count;
    }
    // iterator could be free here, but it will be automatcially freed when
    // the array is freed in the tear down function below.

END_TEST( json_free_value( array ) )

START_TEST( test_duplicate_array_with_elements, NO_SETUP )

    json_value_t *array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, array );

    json_value_type_t value_type = json_get_value_type( array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );

    json_status_t status = json_insert_element_into_array( array, -1, new_null );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 10, new_null );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 1, new_null );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 0, new_null );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    // try inserting the same value twice => probable crash...

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    json_value_t *new_boolean = json_new_value( JSON_BOOLEAN, true );
    ASSERT_DIFFERENT( NULL, new_boolean );

    status = json_insert_element_into_array( array, 2, new_boolean );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 1, new_boolean );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_string = json_new_value( JSON_STRING, "Hello json" );
    ASSERT_DIFFERENT( NULL, new_string );

    status = json_insert_element_into_array( array, 3, new_string );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 0, new_string );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_int = json_new_value( JSON_NUMBER, JSON_INTEGER_NUMBER, 17 );
    ASSERT_DIFFERENT( NULL, new_int );

    status = json_insert_element_into_array( array, 4, new_int );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 1, new_int );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_real = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, 23.5 );
    ASSERT_DIFFERENT( NULL, new_real );

    status = json_insert_element_into_array( array, 5, new_real );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 1, new_real );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, new_array );

    status = json_insert_element_into_array( array, 6, new_array );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 4, new_array );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    json_value_t *new_object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, new_object );

    status = json_insert_element_into_array( array, 7, new_object );
    ASSERT_EQUAL( JSON_STATUS_OUT_OF_BOUND, status );

    status = json_insert_element_into_array( array, 6, new_object );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 7, array_size );

    json_value_t *duplicate_array = json_duplicate_value( array );
    ASSERT_DIFFERENT( NULL, duplicate_array );

    json_array_iterator_t iterator = json_new_array_iterator( duplicate_array );
    ASSERT_DIFFERENT( NULL, iterator );

    unsigned int count = 0;
    const json_value_t *value; // direct pointer to the array content (not a copy)

    const unsigned char *string_value;
    json_number_type_t number_type;
    long long int int_value;
    double real_value;
    bool boolean_value;
    unsigned int new_array_size;
    unsigned int new_member_count;

    while ( ( value = json_iterate_array_element( &iterator ) ) ) {
//        PRINT_NORMAL("count = %d\n", count );
        json_value_type_t value_type = json_get_value_type( value );
        switch ( count ) {
        case 0: // should be string
            ASSERT_EQUAL( JSON_STRING, value_type );
            string_value = json_get_string_value( value );
            ASSERT_EQUAL( 0, strcmp("Hello json", (const char *)string_value) );
            break;
        case 1: // should be real
            ASSERT_EQUAL( JSON_NUMBER, value_type );
            number_type = json_get_value_number_type( value );
            ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );
            real_value = json_get_real_value( value );
            ASSERT_EQUAL( 23.5, real_value );
            break;
        case 2:  // should be int
            ASSERT_EQUAL( JSON_NUMBER, value_type );
            number_type = json_get_value_number_type( value );
            ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );
            int_value = json_get_integer_value( value );
            ASSERT_EQUAL( 17, int_value );
            break;
        case 3: // should be null
            ASSERT_EQUAL( JSON_NULL, value_type );
            break;
        case 4: // ahould ne ab empty array
            ASSERT_EQUAL( JSON_ARRAY, value_type );
            new_array_size = json_get_array_size( new_array );
            ASSERT_EQUAL( 0, new_array_size );
            break;
        case 5: // should be boolean
            ASSERT_EQUAL( JSON_BOOLEAN, value_type );
            boolean_value = json_get_boolean_value( value );
            ASSERT_EQUAL( true, boolean_value );
            break;
        case 6: // should be an empty object
            ASSERT_EQUAL( JSON_OBJECT, value_type );
            new_member_count = json_get_object_member_count( new_object );
            ASSERT_EQUAL( 0, new_member_count );
            break;
        }
        ++count;
    }
    // iterator could be free here, but it will be automatcially freed when
    // the array is freed in the tear down function below.

END_TEST( json_free_value( array ); json_free_value( duplicate_array ) )

START_TEST( test_array_replace_1, NO_SETUP )

    json_value_t *array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, array );

    json_value_type_t value_type = json_get_value_type( array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    const json_value_t *in_array = json_get_array_element( array, 0 );
    ASSERT_EQUAL( NULL, in_array );

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );

    json_status_t status = json_insert_element_into_array( array, 0, new_null );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_DIFFERENT( NULL, in_array );

    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_NULL, value_type );

    in_array = json_get_array_element( array, 1 );
    ASSERT_EQUAL( NULL, in_array );

    long long int integer = 1234567890;
    json_value_t *new_int = json_new_value( JSON_NUMBER, JSON_INTEGER_NUMBER, integer );
    ASSERT_DIFFERENT( NULL, new_null );

    json_value_t *old_value = json_replace_element_in_array( array, 0, new_int );
    ASSERT_DIFFERENT( NULL, old_value );
    ASSERT_EQUAL( old_value, new_null );
    json_free_value( old_value );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( in_array );
    ASSERT_EQUAL( JSON_INTEGER_NUMBER, number_type );

    long long int in_array_int = json_get_integer_value( in_array );
    ASSERT_EQUAL( 1234567890, in_array_int );

END_TEST( json_free_value( array ) )

START_TEST( test_array_replace_2, NO_SETUP )

    json_value_t *array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, array );

    json_value_type_t value_type = json_get_value_type( array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    const json_value_t *in_array = json_get_array_element( array, 0 );
    ASSERT_EQUAL( NULL, in_array );

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );
    json_status_t status = json_insert_element_into_array( array, 0, new_null );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_DIFFERENT( NULL, in_array );

    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_NULL, value_type );

    json_value_t *new_string = json_new_value( JSON_STRING, (const unsigned char *)"Strong Signal" );
    ASSERT_DIFFERENT( NULL, new_string );
    status = json_insert_element_into_array( array, 1, new_string );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    in_array = json_get_array_element( array, 1 );
    ASSERT_DIFFERENT( NULL, in_array );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *in_array_string = json_get_string_value( in_array );
    ASSERT_EQUAL( 0, strcmp( "Strong Signal", (const char *)in_array_string ) );

    json_value_t *new_boolean = json_new_value( JSON_BOOLEAN, true );
    ASSERT_DIFFERENT( NULL, new_null );

    json_value_t *old_value = json_replace_element_in_array( array, 1, new_boolean );
    ASSERT_DIFFERENT( NULL, old_value );
    ASSERT_EQUAL( old_value, new_string );
    json_free_value( old_value );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 2, array_size );

    in_array = json_get_array_element( array, 0 );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_NULL, value_type );

    in_array = json_get_array_element( array, 1 );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    bool in_array_bool = json_get_boolean_value( in_array );
    ASSERT_EQUAL( true, in_array_bool );

END_TEST( json_free_value( array ) )

START_TEST( test_array_replace_3, NO_SETUP )

    json_value_t *array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, array );

    json_value_type_t value_type = json_get_value_type( array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    const json_value_t *in_array = json_get_array_element( array, 0 );
    ASSERT_EQUAL( NULL, in_array );

    json_value_t *new_boolean = json_new_value( JSON_BOOLEAN, true );
    ASSERT_DIFFERENT( NULL, new_boolean );
    json_status_t status = json_insert_element_into_array( array, 0, new_boolean );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_DIFFERENT( NULL, in_array );

    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    json_value_t *new_string = json_new_value( JSON_STRING, (const unsigned char *)"Strong Signal" );
    ASSERT_DIFFERENT( NULL, new_string );
    status = json_insert_element_into_array( array, 0, new_string );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 2, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_DIFFERENT( NULL, in_array );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *in_array_string = json_get_string_value( in_array );
    ASSERT_EQUAL( 0, strcmp( "Strong Signal", (const char *)in_array_string ) );

    json_value_t *new_real = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, 0.5 );
    ASSERT_DIFFERENT( NULL, new_real );

    json_value_t *old_value = json_replace_element_in_array( array, 0, new_real );
    ASSERT_DIFFERENT( NULL, old_value );
    ASSERT_EQUAL( old_value, new_string );
    json_free_value( old_value );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 2, array_size );

    in_array = json_get_array_element( array, 0 );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_NUMBER, value_type );

    json_number_type_t number_type = json_get_value_number_type( in_array );
    ASSERT_EQUAL( JSON_REAL_NUMBER, number_type );

    double real_number = json_get_real_value( in_array );
    ASSERT_EQUAL( 0.5, real_number );

    in_array = json_get_array_element( array, 1 );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    bool in_array_bool = json_get_boolean_value( in_array );
    ASSERT_EQUAL( true, in_array_bool );

END_TEST( json_free_value( array ) )

START_TEST( test_array_remove_1, NO_SETUP )

    json_value_t *array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, array );

    json_value_type_t value_type = json_get_value_type( array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    const json_value_t *in_array = json_get_array_element( array, 0 );
    ASSERT_EQUAL( NULL, in_array );

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );

    json_status_t status = json_insert_element_into_array( array, 0, new_null );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_DIFFERENT( NULL, in_array );

    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_NULL, value_type );

    in_array = json_get_array_element( array, 1 );
    ASSERT_EQUAL( NULL, in_array );

    json_value_t *old_value = json_remove_element_from_array( array, 0 );
    ASSERT_DIFFERENT( NULL, old_value );
    ASSERT_EQUAL( old_value, new_null );
    json_free_value( old_value );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_EQUAL( NULL, in_array );

END_TEST( json_free_value( array ) )

START_TEST( test_array_remove_2, NO_SETUP )

    json_value_t *array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, array );

    json_value_type_t value_type = json_get_value_type( array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    const json_value_t *in_array = json_get_array_element( array, 0 );
    ASSERT_EQUAL( NULL, in_array );

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );
    json_status_t status = json_insert_element_into_array( array, 0, new_null );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_DIFFERENT( NULL, in_array );

    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_NULL, value_type );

    in_array = json_get_array_element( array, 1 );
    ASSERT_EQUAL( NULL, in_array );

    json_value_t *new_string = json_new_value( JSON_STRING, (const unsigned char *)"Strong Signal" );
    ASSERT_DIFFERENT( NULL, new_null );
    status = json_insert_element_into_array( array, 1, new_string );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 2, array_size );

    in_array = json_get_array_element( array, 1 );
    ASSERT_DIFFERENT( NULL, in_array );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *in_array_string = json_get_string_value( in_array );
    ASSERT_EQUAL( 0, strcmp( "Strong Signal", (const char *)in_array_string ) );

    json_value_t *old_value = json_remove_element_from_array( array, 1 );
    ASSERT_DIFFERENT( NULL, old_value );
    ASSERT_EQUAL( old_value, new_string );
    json_free_value( old_value );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_NULL, value_type );

    in_array = json_get_array_element( array, 1 );
    ASSERT_EQUAL( NULL, in_array );

END_TEST( json_free_value( array ) )

START_TEST( test_array_remove_3, NO_SETUP )

    json_value_t *array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, array );

    json_value_type_t value_type = json_get_value_type( array );
    ASSERT_EQUAL( JSON_ARRAY, value_type );

    unsigned int array_size = json_get_array_size( array );
    ASSERT_EQUAL( 0, array_size );

    const json_value_t *in_array = json_get_array_element( array, 0 );
    ASSERT_EQUAL( NULL, in_array );

    json_value_t *new_boolean = json_new_value( JSON_BOOLEAN, true );
    ASSERT_DIFFERENT( NULL, new_boolean );
    json_status_t status = json_insert_element_into_array( array, 0, new_boolean );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_DIFFERENT( NULL, in_array );

    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    json_value_t *new_string = json_new_value( JSON_STRING, (const unsigned char *)"Strong Signal" );
    ASSERT_DIFFERENT( NULL, new_string );
    status = json_insert_element_into_array( array, 0, new_string );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 2, array_size );

    in_array = json_get_array_element( array, 0 );
    ASSERT_DIFFERENT( NULL, in_array );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_STRING, value_type );

    const unsigned char *in_array_string = json_get_string_value( in_array );
    ASSERT_EQUAL( 0, strcmp( "Strong Signal", (const char *)in_array_string ) );

    json_value_t *old_value = json_remove_element_from_array( array, 0 );
    ASSERT_DIFFERENT( NULL, old_value );
    ASSERT_EQUAL( old_value, new_string );
    json_free_value( old_value );

    array_size = json_get_array_size( array );
    ASSERT_EQUAL( 1, array_size );

    in_array = json_get_array_element( array, 0 );
    value_type = json_get_value_type( in_array );
    ASSERT_EQUAL( JSON_BOOLEAN, value_type );

    bool in_array_bool = json_get_boolean_value( in_array );
    ASSERT_EQUAL( true, in_array_bool );

END_TEST( json_free_value( array ) )

/* --------------------------------------------------------------- */

// helper function
bool check_object( json_value_t *object, unsigned int member_count,
                   json_value_type_t *type_array,
                   json_number_type_t *number_type_array )
{
    json_object_iterator_t iterator = json_new_object_iterator( object );
    if ( NULL == iterator ) return false;

    unsigned int count = 0;

    const unsigned char *name;
    const json_value_t *value; // direct pointer to the object content (not a copy)
    while ( ( value = json_iterate_object_member( &iterator, &name ) ) ) {
//        PRINT_NORMAL("count = %d member name %s\n", count, (const char *)name );
        json_value_type_t value_type = json_get_value_type( value );
        if ( NOT_A_JSON_VALUE == value_type ) return false;
        --type_array[ value_type ];

        json_number_type_t number_type;

        switch( value_type ) {
        default: /* case JSON_NULL: case JSON_BOOLEAN: case JSON_STRING:
                    case JSON_ARRAY: case JSON_OBJECT: */
            break;
        case JSON_NUMBER:
            number_type = json_get_value_number_type( value );
            --number_type_array[ number_type ];
            break;
        }
        ++count;
    }
    if ( count != member_count ) return false;
    for ( json_value_type_t vt = NOT_A_JSON_VALUE; vt <= JSON_OBJECT; ++vt ) {
        if ( type_array[ vt ] ) return false;
    }
    for ( json_number_type_t nt = NOT_A_JSON_NUMBER; nt <= JSON_REAL_NUMBER; ++nt ) {
        if ( type_array[ nt ] ) return false;
    }
    return true;
}

START_TEST( test_object_insert, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 0, member_count );

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );

    json_status_t status =
             json_insert_member_into_object( object, (unsigned char *)"foo", new_null );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 1, member_count );

    // try inserting the same member twice
    status = json_insert_member_into_object( object, (unsigned char *)"foo", new_null );
    ASSERT_EQUAL( JSON_STATUS_DUPLICATE_MEMBER , status );

    // try inserting a different member with the same value: probable crash

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 1, member_count );

    json_value_t *new_boolean = json_new_value( JSON_BOOLEAN, true );
    ASSERT_DIFFERENT( NULL, new_boolean );

    status = json_insert_member_into_object( object, (unsigned char *)"foo", new_boolean );
    ASSERT_EQUAL( JSON_STATUS_DUPLICATE_MEMBER , status );

    status = json_insert_member_into_object( object, (unsigned char *)"bool", new_boolean );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 2, member_count );

    json_value_t *new_string = json_new_value( JSON_STRING, "Hello json" );
    ASSERT_DIFFERENT( NULL, new_string );

    status = json_insert_member_into_object( object, (unsigned char *)"bool", new_string );
    ASSERT_EQUAL( JSON_STATUS_DUPLICATE_MEMBER , status );

    status = json_insert_member_into_object( object, (unsigned char *)"string", new_string );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 3, member_count );

    json_value_t *new_int = json_new_value( JSON_NUMBER, JSON_INTEGER_NUMBER, 17 );
    ASSERT_DIFFERENT( NULL, new_int );

    status = json_insert_member_into_object( object, (unsigned char *)"string", new_int );
    ASSERT_EQUAL( JSON_STATUS_DUPLICATE_MEMBER , status );

    status = json_insert_member_into_object( object, (unsigned char *)"integer", new_int );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 4, member_count );

    json_value_t *new_real = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, 23.5 );
    ASSERT_DIFFERENT( NULL, new_real );

    status = json_insert_member_into_object( object, (unsigned char *)"integer", new_real );
    ASSERT_EQUAL( JSON_STATUS_DUPLICATE_MEMBER , status );

    status = json_insert_member_into_object( object, (unsigned char *)"real", new_real );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 5, member_count );

    json_value_t *new_array = json_new_value( JSON_ARRAY );
    ASSERT_DIFFERENT( NULL, new_array );

    status = json_insert_member_into_object( object, (unsigned char *)"integer", new_array );
    ASSERT_EQUAL( JSON_STATUS_DUPLICATE_MEMBER , status );

    status = json_insert_member_into_object( object, (unsigned char *)"array", new_array );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 6, member_count );

    json_value_t *new_object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, new_object );

    status = json_insert_member_into_object( object, (unsigned char *)"real", new_object );
    ASSERT_EQUAL( JSON_STATUS_DUPLICATE_MEMBER , status );

    status = json_insert_member_into_object( object, (unsigned char *)"object", new_object );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, status );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    json_value_type_t type_array[] =
            { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
              1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] = { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };
    ASSERT_EQUAL( true, check_object( object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ) )

// helper functions
const char *get_type_name( json_value_type_t vt )
{
    switch ( vt ) {
    default:            break;
    case JSON_NULL:     return "null";
    case JSON_BOOLEAN:  return "boolean";
    case JSON_STRING:   return "string";
    case JSON_NUMBER:   return "number";
    case JSON_ARRAY:    return "array";
    case JSON_OBJECT:   return "object";
    }
    return "";
}

bool setup_object_member( json_value_t *object, json_value_type_t vt,
                         json_number_type_t nt, unsigned int member_count )
{
    // member names are automatically generated as 'member_<type>_xx'
    char name[ 128 ];
    snprintf( name, 128, "member_%s_%02d", get_type_name( vt ), member_count );

    json_value_t *new_value;
    char string[ 128 ];

    switch ( vt ) {
    default: return false;
    case JSON_NULL: case JSON_ARRAY: case JSON_OBJECT:
        new_value = json_new_value( vt );
        break;
    case JSON_BOOLEAN:
        new_value = json_new_value( vt, true );
        break;
    case JSON_STRING:
        snprintf( string, 128, "string_%d", member_count );
        new_value = json_new_value( vt, string );
        break;
    case JSON_NUMBER:
        if ( JSON_INTEGER_NUMBER == nt ) {
            new_value = json_new_value( vt, nt, member_count * 100 );
        } else {
            new_value = json_new_value( vt, nt, member_count + 23.5 );
        }
    }

    if ( NULL == new_value ) return false;

    json_status_t status = json_insert_member_into_object( object,
                                         (const unsigned char*)name, new_value );
    if ( JSON_STATUS_SUCCESS != status ) return false;

    int count = json_get_object_member_count( object );
    if ( -1 == count || 1 + member_count != (unsigned int)count  ) return false;
    return false;
}

json_number_type_t get_next_number_type( json_number_type_t *number_type_array )
{
    for ( json_number_type_t nt = JSON_INTEGER_NUMBER; // skip NOT_A_JSON_NUMBER
                                                nt <= JSON_REAL_NUMBER; ++nt ) {
        if ( number_type_array[ nt ] ) {
            -- number_type_array[ nt ];
            return nt;
        }
    }
    return NOT_A_JSON_NUMBER;
}

bool setup_object( json_value_t *object, json_value_type_t *type_array,
                                    json_number_type_t *number_type_array )
{
    unsigned int member_count = json_get_object_member_count( object );
    if ( 0 != member_count ) return false;

    for ( json_value_type_t vt = JSON_NULL; // skip JSON_NOT_A_VALUE
                                       vt <= JSON_OBJECT; ++ vt ) {
        while ( type_array[ vt ] ) {
            json_number_type_t nt =    ( JSON_NUMBER == vt ) ?
                get_next_number_type(number_type_array) : NOT_A_JSON_NUMBER;
            setup_object_member( object, vt, nt, member_count );
            --type_array[ vt ];
            ++member_count;

            int count = json_get_object_member_count( object );
            if ( -1 == count || (unsigned int)count != member_count ) return false;
        }
    }
    return true;
}

START_TEST( test_duplicate_object_with_elements, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    json_value_type_t type_array[] =
            { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
              1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] = { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };

    bool result = setup_object( object, type_array, number_type_array );
    ASSERT_EQUAL( true, result );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    json_value_t *duplicate_object = json_duplicate_value( object );

    type_array[JSON_NULL] = 1;
    type_array[JSON_BOOLEAN] = 1;
    type_array[JSON_NUMBER] = 2;
    type_array[JSON_STRING] = 1;
    type_array[JSON_ARRAY] = 1;
    type_array[JSON_OBJECT] = 1;

    number_type_array[JSON_INTEGER_NUMBER] = 1;
    number_type_array[JSON_REAL_NUMBER] = 1;

    ASSERT_EQUAL( true, check_object( duplicate_object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ); json_free_value( duplicate_object ) )

START_TEST( test_object_replace_one, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    json_value_type_t type_array[] =
        { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
          1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] =
        { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };

    bool result = setup_object( object, type_array, number_type_array );
    ASSERT_EQUAL( true, result );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    // replace string with integer

    json_value_t *new_integer = json_new_value(
                                    JSON_NUMBER, JSON_INTEGER_NUMBER, 101LL );
    ASSERT_DIFFERENT( NULL, new_integer );

    const json_value_t *string = json_search_for_object_member_by_name(
                            object, (const unsigned char *)"member_string_04" );
    ASSERT_DIFFERENT( NULL, string );

    json_value_t *old_string = json_replace_member_value_in_object(
               object, (const unsigned char *)"member_string_04", new_integer );
    ASSERT_EQUAL( string, old_string );
    json_free_value( old_string );

    type_array[JSON_NULL] = 1;
    type_array[JSON_BOOLEAN] = 1;
    type_array[JSON_NUMBER] = 3;
    type_array[JSON_STRING] = 0;
    type_array[JSON_ARRAY] = 1;
    type_array[JSON_OBJECT] = 1;

    number_type_array[JSON_INTEGER_NUMBER] = 2;
    number_type_array[JSON_REAL_NUMBER] = 1;

    ASSERT_EQUAL( true, check_object( object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ) )

START_TEST( test_object_replace_two, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    json_value_type_t type_array[] =
            { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
              1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] = { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };

    bool result = setup_object( object, type_array, number_type_array );
    ASSERT_EQUAL( true, result );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    // replace string with boolean & boolean with string (swap members)

    json_value_t *duplicate_string = json_new_value( JSON_STRING, "string" );
    ASSERT_DIFFERENT( NULL, duplicate_string );
    json_value_t *duplicate_boolean = json_new_value(JSON_BOOLEAN, false );
    ASSERT_DIFFERENT( NULL, duplicate_boolean );

    const json_value_t *boolean = json_search_for_object_member_by_name( object,
                                             (const unsigned char *)"member_boolean_01" );
    ASSERT_DIFFERENT( NULL, boolean );

    json_value_t *old_boolean = json_replace_member_value_in_object( object,
                                                  (const unsigned char *)"member_boolean_01",
                                                           duplicate_string );
    ASSERT_EQUAL( boolean, old_boolean );
    json_free_value( old_boolean );

    const json_value_t *string = json_search_for_object_member_by_name( object,
                                                (const unsigned char *)"member_string_04" );
    ASSERT_DIFFERENT( NULL, string );

    json_value_t *old_string = json_replace_member_value_in_object( object,
                                                  (const unsigned char *)"member_string_04",
                                                          duplicate_boolean );
    ASSERT_EQUAL( string, old_string );
    json_free_value( old_string );

    type_array[JSON_NULL] = 1;
    type_array[JSON_BOOLEAN] = 1;
    type_array[JSON_NUMBER] = 2;
    type_array[JSON_STRING] = 1;
    type_array[JSON_ARRAY] = 1;
    type_array[JSON_OBJECT] = 1;

    number_type_array[JSON_INTEGER_NUMBER] = 1;
    number_type_array[JSON_REAL_NUMBER] = 1;

    ASSERT_EQUAL( true, check_object( object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ) )

START_TEST( test_object_replace_three, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    json_value_type_t type_array[] =
            { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
              1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] = { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };

    bool result = setup_object( object, type_array, number_type_array );
    ASSERT_EQUAL( true, result );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    json_value_t *new_boolean = json_new_value(JSON_BOOLEAN, false );
    ASSERT_DIFFERENT( NULL, new_boolean );

    const json_value_t *boolean = json_search_for_object_member_by_name( object,
                                             (const unsigned char *)"member_boolean_01" );
    ASSERT_DIFFERENT( NULL, boolean );

    json_value_t *old_boolean = json_replace_member_value_in_object( object,
                                                  (const unsigned char *)"member_boolean_01",
                                                           new_boolean );
    ASSERT_EQUAL( boolean, old_boolean );
    json_free_value( old_boolean );

    json_value_t *new_string = json_new_value( JSON_STRING, "another string" );
    ASSERT_DIFFERENT( NULL, new_string );

    const json_value_t *string = json_search_for_object_member_by_name( object,
                                                (const unsigned char *)"member_string_04" );
    ASSERT_DIFFERENT( NULL, string );

    json_value_t *old_string = json_replace_member_value_in_object( object,
                                                  (const unsigned char *)"member_string_04",
                                                          new_string );
    ASSERT_EQUAL( string, old_string );
    json_free_value( old_string );


    json_value_t *new_number = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, 3.14159 );
    ASSERT_DIFFERENT( NULL, new_number );

    const json_value_t *number = json_search_for_object_member_by_name( object,
                                                (const unsigned char *)"member_number_02" );
    ASSERT_DIFFERENT( NULL, number );

    json_value_t *old_number = json_replace_member_value_in_object( object,
                                                  (const unsigned char *)"member_number_02",
                                                          new_number );
    ASSERT_EQUAL( number, old_number );
    json_free_value( old_number );

    type_array[JSON_NULL] = 1;
    type_array[JSON_BOOLEAN] = 1;
    type_array[JSON_NUMBER] = 2;
    type_array[JSON_STRING] = 1;
    type_array[JSON_ARRAY] = 1;
    type_array[JSON_OBJECT] = 1;

    number_type_array[JSON_INTEGER_NUMBER] = 1;
    number_type_array[JSON_REAL_NUMBER] = 2;

    ASSERT_EQUAL( true, check_object( object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ) )

START_TEST( test_object_remove_one, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    json_value_type_t type_array[] =
            { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
              1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] = { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };

    bool result = setup_object( object, type_array, number_type_array );
    ASSERT_EQUAL( true, result );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    unsigned char *old_name;
    json_value_t *old_string = json_remove_member_from_object( object,
                                      (const unsigned char *)"member_string_04",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_string );
    ASSERT_EQUAL( 0, strcmp( "member_string_04", (char *)old_name ) );

    free( old_name );
    json_free_value( old_string );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 6, member_count );

    type_array[JSON_NULL] = 1;
    type_array[JSON_BOOLEAN] = 1;
    type_array[JSON_NUMBER] = 2;
    type_array[JSON_STRING] = 0;
    type_array[JSON_ARRAY] = 1;
    type_array[JSON_OBJECT] = 1;

    number_type_array[JSON_INTEGER_NUMBER] = 1;
    number_type_array[JSON_REAL_NUMBER] = 1;

    ASSERT_EQUAL( true, check_object( object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ) )

START_TEST( test_object_remove_two, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    json_value_type_t type_array[] =
            { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
              1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] = { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };

    bool result = setup_object( object, type_array, number_type_array );
    ASSERT_EQUAL( true, result );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    unsigned char *old_name;
    json_value_t *old_string = json_remove_member_from_object( object,
                                      (const unsigned char *)"member_string_04",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_string );
    ASSERT_EQUAL( 0, strcmp( "member_string_04", (char *)old_name ) );

    free( old_name );
    json_free_value( old_string );

    json_value_t *old_boolean = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_boolean_01",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_boolean );
    ASSERT_EQUAL( 0, strcmp( "member_boolean_01", (char *)old_name ) );

    free( old_name );
    json_free_value( old_boolean );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 5, member_count );

    type_array[JSON_NULL] = 1;
    type_array[JSON_BOOLEAN] = 0;
    type_array[JSON_NUMBER] = 2;
    type_array[JSON_STRING] = 0;
    type_array[JSON_ARRAY] = 1;
    type_array[JSON_OBJECT] = 1;

    number_type_array[JSON_INTEGER_NUMBER] = 1;
    number_type_array[JSON_REAL_NUMBER] = 1;

    ASSERT_EQUAL( true, check_object( object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ) )

START_TEST( test_object_remove_three, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    json_value_type_t type_array[] =
            { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
              1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] =
            { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };

    bool result = setup_object( object, type_array, number_type_array );
    ASSERT_EQUAL( true, result );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    unsigned char *old_name;
    json_value_t *old_string = json_remove_member_from_object( object,
                                      (const unsigned char *)"member_string_04",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_string );
    ASSERT_EQUAL( 0, strcmp( "member_string_04", (char *)old_name ) );

    free( old_name );
    json_free_value( old_string );

    json_value_t *old_boolean = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_boolean_01",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_boolean );
    ASSERT_EQUAL( 0, strcmp( "member_boolean_01", (char *)old_name ) );

    free( old_name );
    json_free_value( old_boolean );

    json_value_t *old_number = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_number_02",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_number );
    ASSERT_EQUAL( 0, strcmp( "member_number_02", (char *)old_name ) );

    free( old_name );
    json_free_value( old_number );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 4, member_count );

    type_array[JSON_NULL] = 1;
    type_array[JSON_BOOLEAN] = 0;
    type_array[JSON_NUMBER] = 1;
    type_array[JSON_STRING] = 0;
    type_array[JSON_ARRAY] = 1;
    type_array[JSON_OBJECT] = 1;

    number_type_array[JSON_INTEGER_NUMBER] = 0;
    number_type_array[JSON_REAL_NUMBER] = 1;

    ASSERT_EQUAL( true, check_object( object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ) )

START_TEST( test_object_remove_all, NO_SETUP )

    json_value_t *object = json_new_value( JSON_OBJECT );
    ASSERT_DIFFERENT( NULL, object );

    json_value_type_t value_type = json_get_value_type( object );
    ASSERT_EQUAL( JSON_OBJECT, value_type );

    json_value_type_t type_array[] =
        { 0, 1 /* JSON_NULL */, 1 /* JSON_BOOLEAN */, 2 /* JSON_NUMBER */,
          1 /* JSON_STRING */, 1 /* JSON_ARRAY */, 1 /* JSON_OBJECT */ };

    json_number_type_t number_type_array[] = { 0, 1 /* JSON_INTEGER_NUMBER */, 1 /* JSON_REAL_NUMBER */ };

    bool result = setup_object( object, type_array, number_type_array );
    ASSERT_EQUAL( true, result );

    unsigned int member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 7, member_count );

    unsigned char *old_name;
    json_value_t *old_string = json_remove_member_from_object( object,
                                      (const unsigned char *)"member_string_04",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_string );
    ASSERT_EQUAL( 0, strcmp( "member_string_04", (char *)old_name ) );

    free( old_name );
    json_free_value( old_string );

    json_value_t *old_boolean = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_boolean_01",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_boolean );
    ASSERT_EQUAL( 0, strcmp( "member_boolean_01", (char *)old_name ) );

    free( old_name );
    json_free_value( old_boolean );

    json_value_t *old_number = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_number_02",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_number );
    ASSERT_EQUAL( 0, strcmp( "member_number_02", (char *)old_name ) );

    free( old_name );
    json_free_value( old_number );

    json_value_t *old_null = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_null_00",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_null );
    ASSERT_EQUAL( 0, strcmp( "member_null_00", (char *)old_name ) );

    free( old_name );
    json_free_value( old_null );

    json_value_t *old_array = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_array_05",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_array );
    ASSERT_EQUAL( 0, strcmp( "member_array_05", (char *)old_name ) );

    free( old_name );
    json_free_value( old_array );

    json_value_t *old_real = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_number_03",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_real );
    ASSERT_EQUAL( 0, strcmp( "member_number_03", (char *)old_name ) );

    free( old_name );
    json_free_value( old_real );

    json_value_t *old_object = json_remove_member_from_object( object,
                                     (const unsigned char *)"member_object_06",
                                       &old_name );
    ASSERT_DIFFERENT( NULL, old_object );
    ASSERT_EQUAL( 0, strcmp( "member_object_06", (char *)old_name ) );

    free( old_name );
    json_free_value( old_object );

    member_count = json_get_object_member_count( object );
    ASSERT_EQUAL( 0, member_count );

    type_array[JSON_NULL] = 0;
    type_array[JSON_BOOLEAN] = 0;
    type_array[JSON_NUMBER] = 0;
    type_array[JSON_STRING] = 0;
    type_array[JSON_ARRAY] = 0;
    type_array[JSON_OBJECT] = 0;

    number_type_array[JSON_INTEGER_NUMBER] = 0;
    number_type_array[JSON_REAL_NUMBER] = 0;

    ASSERT_EQUAL( true, check_object( object, member_count, type_array, number_type_array ) );

END_TEST( json_free_value( object ) )

// ===========================================================================

START_TEST( test_serialize_from_new_null, NO_SETUP )

    char data [] = "null";

    json_value_t *new_null = json_new_value( JSON_NULL, true );
    ASSERT_DIFFERENT( NULL, new_null );

    size_t len = json_get_serialization_length( new_null, PACKED_FORMAT );
    ASSERT_EQUAL( len + 1, sizeof(data) );

    char *buffer = malloc( 1 + len );
    size_t len1 = json_serialize( new_null, PACKED_FORMAT, 1 + len, buffer );
    ASSERT_EQUAL( len1, len );

    ASSERT_EQUAL( 0, strcmp( data, buffer ) );

END_TEST( json_free_value( new_null ); free( buffer ) )

START_TEST( test_serialize_from_parsed_escaped_string, NO_SETUP )

#define JSON_STRING "\"\\tescaped chars \\n & \\\" \""
    const char *string = JSON_STRING;
    size_t size = sizeof( JSON_STRING );

    json_error_report_t error;
    json_value_t *root = json_parse_buffer( (const unsigned char *)string, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    size_t len = json_get_serialization_length( root, PACKED_FORMAT );
    ASSERT_EQUAL( len + 1, size );

    char *buffer = malloc( 1 + len );
    size_t len1 = json_serialize( root, PACKED_FORMAT, 1 + len, buffer );
    ASSERT_EQUAL( len1, len );

    ASSERT_EQUAL( 0, strcmp( string, buffer ) );

END_TEST( json_free_value( root ); free( buffer ) )

START_TEST( test_serialize_from_parsed_array, NO_SETUP )

#define JSON_ARRAY "[123.456,\"tobacco\",true,null]"
    const char data[] = JSON_ARRAY;
    size_t size = sizeof( data );

    json_error_report_t error;

    json_value_t *root = json_parse_buffer( (const unsigned char *)data, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    size_t len = json_get_serialization_length( root, PACKED_FORMAT );
    ASSERT_EQUAL( len + 1, size );

    char *buffer = malloc( 1 + len );
    size_t len1 = json_serialize( root, PACKED_FORMAT, 1 + len, buffer );
    ASSERT_EQUAL( len1, len );

    ASSERT_EQUAL( 0, strcmp( data, buffer ) );

END_TEST( json_free_value( root ); free( buffer ) )

START_TEST( test_serialize_from_parsed_object, NO_SETUP )

#define JSON_OBJECT "{\"foo\":123.456,\"bar\":\"tobacco\",\"booze\":true,\"zilch\":null}"
    const char data[] = JSON_OBJECT;
    size_t size = sizeof( data );

    json_error_report_t error;

    json_value_t *root = json_parse_buffer( (const unsigned char *)data, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    size_t len = json_get_serialization_length( root, PACKED_FORMAT );
    ASSERT_EQUAL( len + 1, size );

    char *buffer = malloc( 1 + len );
    size_t len1 = json_serialize( root, PACKED_FORMAT, 1 + len, buffer );
    ASSERT_EQUAL( len1, len );

    ASSERT_EQUAL( 0, strcmp( buffer, data ) );

END_TEST( json_free_value( root ); free( buffer ) )


START_TEST( test_serialize_to_file_from_parsed_object, NO_SETUP )

#define JSON_OBJECT "{\"foo\":123.456,\"bar\":\"tobacco\",\"booze\":true,\"zilch\":null}"
    const char data[] = JSON_OBJECT;
    size_t size = sizeof( data );

    json_error_report_t error;

    json_value_t *root = json_parse_buffer( (const unsigned char *)data, 0, &error );
    ASSERT_DIFFERENT( NULL, root );
    ASSERT_EQUAL( JSON_STATUS_SUCCESS, error.status );

    size_t len = json_get_serialization_length( root, PACKED_FORMAT );
    ASSERT_EQUAL( len + 1, size );

    size_t len1 = json_print( root, PACKED_FORMAT, NULL, stdout );
    PRINT_NORMAL( "\n" );
    ASSERT_EQUAL( len1, len );

END_TEST( json_free_value( root ) )

int main( int argc, char **argv )
{
    (void)argc;
    (void)argv;

BEGIN_TEST_SUITE( json_parser )

    test_parser_null();

    test_parser_nuke();
    test_parser_foo();
    test_parser_tuxedo();
    test_parser_zero();

    test_parser_false();
    test_parser_true();
    test_parser_string();
    test_parser_bad_string();
    test_parser_empty_string();

    test_parser_integer_1();
    test_parser_integer_2();
    test_parser_integer_3();
    test_parser_bad_integer_1();
    test_parser_bad_integer_2();
    test_parser_bad_integer_3();
    test_parser_bad_integer_4();
    test_parser_bad_integer_5();
    test_parser_bad_integer_6();

    test_parser_real_1();
    test_parser_real_2();
    test_parser_real_3();
    test_parser_real_4();
    test_parser_real_5();
    test_parser_real_6();
    test_parser_real_7();
    test_parser_bad_real_1();
    test_parser_bad_real_2();
    test_parser_bad_real_3();
    test_parser_bad_real_4();

    test_parser_empty_array();
    test_parser_bad_array_1();
    test_parser_bad_array_2();
    test_parser_bad_array_3();
    test_parser_bad_array_4();
    test_parser_bad_array_5();
    test_parser_boolean_string_array();
    test_parser_small_int_array();
    test_parser_larger_int_array();

    test_parser_empty_object();
    test_parser_object_member();
    test_parser_bad_object_1();
    test_parser_bad_object_2();
    test_parser_bad_object_3();
    test_parser_bad_object_4();
    test_parser_bad_object_5();
    test_parser_null_int_object();
    test_parser_array_object_object();
    test_parser_larger_object();

END_TEST_SUITE()

BEGIN_TEST_SUITE( json_editor )

    test_new_null();
    test_new_boolean();
    test_new_string();
    test_new_integer();
    test_new_real();
    test_new_array();
    test_new_object();

    test_duplicate_null();
    test_duplicate_boolean();
    test_duplicate_string();
    test_duplicate_integer();
    test_duplicate_real();
    test_duplicate_empty_array();
    test_duplicate_empty_object();

    test_array_insert();
    test_duplicate_array_with_elements();

    test_array_replace_1();
    test_array_replace_2();
    test_array_replace_3();

    test_array_remove_1();
    test_array_remove_2();
    test_array_remove_3();

    test_object_insert();
    test_duplicate_object_with_elements();

    test_object_replace_one();
    test_object_replace_two();
    test_object_replace_three();

    test_object_remove_one();
    test_object_remove_two();
    test_object_remove_three();

    test_object_remove_all();

END_TEST_SUITE()

BEGIN_TEST_SUITE( json_serialize )
    test_serialize_from_new_null();
    test_serialize_from_parsed_escaped_string();
    test_serialize_from_parsed_array();
    test_serialize_from_parsed_object();

    test_serialize_to_file_from_parsed_object();
END_TEST_SUITE()
}
