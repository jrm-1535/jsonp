
#ifndef __JSONVALUE_H__
#define __JSONVALUE_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct _value json_value_t;  // Opaque type for a json value

// Returns the type of a json value
typedef enum {
    NOT_A_JSON_VALUE,
    JSON_NULL, JSON_BOOLEAN, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT
} json_value_type_t;

extern json_value_type_t json_get_value_type( const json_value_t *value );

// returns the bool value (false | true) of a jason boolean value
extern bool json_get_boolean_value( const json_value_t *value );

// returns the string value (zero-terminated UTF8 string) of a json string
// value. The string is directly pointed in the value; it is not a copy,
// don't free it. The string must be duplicated if need after the whole
// json root is freed (see json_free)
extern const unsigned char *json_get_string_value( const json_value_t *value );

/* returns the type of a json number (which can be integer or real) */
typedef enum {
    NOT_A_JSON_NUMBER, JSON_INTEGER_NUMBER, JSON_REAL_NUMBER
} json_number_type_t;

extern json_number_type_t json_get_value_number_type(
                                                const json_value_t *value );

/* return the integer value (long long int) of a json integer value */
extern long long int json_get_integer_value( const json_value_t *value );

/* return the real value (double) of a json real number */
extern double json_get_real_value( const json_value_t *value );

/* return the number of members of a json object value or -1 if
   the value is not an object */
extern int json_get_object_member_count( const json_value_t *value );

/* return the number of elements of a json array value or -1
   if the value is not an array */
extern int json_get_array_size( const json_value_t *value );

/* create a member iterator from a json object value. Note that the order
   in which members are returned through the iterator is undefined */
typedef void *json_object_iterator_t;
extern json_object_iterator_t json_new_object_iterator(
                                                const json_value_t *value );

/* return the value and the name of the member associated to the current
   value of the iterator, and move the iterator to the "next" member.
   Returns NULL if no other member is available in the object. */
extern const json_value_t *json_iterate_object_member(
                                        json_object_iterator_t *object_iterator,
                                        const unsigned char **name );

extern void json_free_object_iterator( json_object_iterator_t object_iterator );

/* create an element iterator from a json array value. Note that the order
   in which elements are returned through the iterator is the same as the
   order in which the elements were given to the parser */
typedef void *json_array_iterator_t;
extern json_array_iterator_t json_new_array_iterator(
                                        const json_value_t *value );

/* return the value of the element associated to the current value of the
   iterator, and move the iterator to the next element in the array.
   return NULL if it reaches the end of the array. */
extern const json_value_t *json_iterate_array_element(
                                        json_array_iterator_t *array_iterator );

extern void json_free_array_iterator( json_array_iterator_t array_iterator );

/* search for the given member name into the json object value passed.
   Return the member value or NULL if not found or not an object */
extern const json_value_t *json_search_for_object_member_by_name(
                                            const json_value_t *object,
                                            const unsigned char *name );

/* retrieve the value at the given index in the json array passed. Return
   the value or NULL if the index is out of range or not an array */
extern const json_value_t *json_get_array_element( const json_value_t *array,
                                                   unsigned int index );

typedef enum {
    JSON_STATUS_INVALID_STRING = -12,
    JSON_STATUS_INVALID_PARAMETERS = -11,
    JSON_STATUS_INVALID_ENCODING = -10,
    JSON_STATUS_PARSE_SYNTAX_ERROR = -9,
    JSON_STATUS_OUT_OF_MEMORY = -8,
    JSON_STATUS_OUT_OF_BOUND = -7,
    JSON_STATUS_DUPLICATE_MEMBER = -6,
    JSON_STATUS_NOT_A_MEMBER_NAME = -5,
    JSON_STATUS_NOT_A_VALUE = -4,
    JSON_STATUS_NOT_A_MEMBER = -3,
    JSON_STATUS_NOT_AN_ARRAY = -2,
    JSON_STATUS_NOT_AN_OBJECT = -1,
    JSON_STATUS_SUCCESS = 0
} json_status_t;

#endif /* __JSONVALUE_H__ */
