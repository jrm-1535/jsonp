
#ifndef __JSONEDIT_H__
#define __JSONEDIT_H__

#include "jsonvalue.h"

/* ----------------- json tree editing ---------------------- */

/* Create new json values, which can then be added to the tree:
   - as a new array element (add_value_to_array)
   - as a new object member (add_member_to_object)

   The new value is dynamically allocated in the memory heap.
   Once inserted in an array or object it belongs to the array or object and
   will be automatically freed when this array or object is freed. If however
   it is not inserted in any arry of object, it must be freed manually when
   not needed anymore.

   Usage:
   - for null,
      val = json_new_value( JSON_NULL );
   - for boolean,
      val = json_new_value( JSON_BOOLEAN, boolean );
   - for integer number,
      val = json_new_value( JSON_NUMBER, JSON_INTEGER_NUMBER, longLongInt );
      NOTE that a literal negative integer must be specifically suffixed with
           LL to avoid a first automatic extension into an unsigned long long!
   - for real number,
      val = json_new_value( JSON_NUMBER, JSON_REAL_NUMBER, doubleReal );
      NOTE that a literal real number must specifically include a decimal point
           or an exponent to avoid be taken as an integer (23. is taken as a
           double, but 23 is taken as an integer)
   - for string,
      val = json_new_value( JSON_STRING, cString );
   - for array,
      val = json_new_value( JSON_ARRAY );
   - for object,
      val = json_new_value( JSON_OBJECT );

   A new array or a new object is always created empty. An array is extended
   by calling insert_value_into_array and an object is extended by calling
   insert_member_into_object */
extern json_value_t *json_new_value( json_value_type_t type, ... );

/* free a json value that was not added to a tree. For values added to a tree
   (either with add_value_to_array or add_member_to_object) the whole tree is
   freed by calling json_free (that include any array element and object
   members). */
extern void json_free_value( json_value_t *value );

/* duplicate an existing value, either newly created or found in an existing
   tree of objects or arrays. The duplicate can be inserted into an array or
   an object, in which case it belongs to the array or object and will be
   automatically freed when the array of object is freed. If the duplicate
   ends up not beeing insterted in any array or object, it should be freed
   manually. Note that the duplicate can be huge, if the value passed as
   input is already a large object or array. */
extern json_value_t *json_duplicate_value( const json_value_t *value );

/* Insert a newly created value (see json_new_value) to an array. The argument
   index is the location where the value should be inserted (0 for first
   position, array_size for after last position). The argument value is the
   value to add to the array. The returm value is JSON_STATUS_SUCCESS in case
   of success or in case of error one of the following: JSON_STATUS_NOT_AN_ARRAY,
   JSON_STATUS_NOT_A_VALUE, JSON_STATUS_OUT_OF_BOUND, JSON_STATUS_OUT_OF_MEMORY. */
extern json_status_t json_insert_element_into_array( json_value_t *varray,
                                                     unsigned int index,
                                                     json_value_t *value );

/* Remove an array element, given its index. Return the removed element
   value in case of success or NULL in case of error (which can mean one of
   JSON_STATUS_NOT_AN_ARRAY or JSON_STATUS_OUT_OF_BOUND). */
extern json_value_t *json_remove_element_from_array( json_value_t *varray,
                                                     unsigned int index );

/* Replace an array element, given its index. Return the previous element
   value in case of success or NULL in case of error (which could mean one
   of the following: JSON_STATUS_NOT_AN_ARRAY, JSON_STATUS_NOT_A_VALUE, or
   JSON_STATUS_OUT_OF_BOUND). The new value is stored into the array, and it
   is up to the caller to free the previous value with json_free_value() */
extern json_value_t *json_replace_element_in_array( json_value_t *varray,
                                                    unsigned int index,
                                                    json_value_t *value );

/* Insert a member passed as a name newly allocated in the heap and a newly
   created value (see json_new_value) to an object. The argument name
   provides the name of the new member and the argument value provides the
   new member value. The return value is JSON_STATUS_SUCCESS in case of success
   or in case of error one of the following : JSON_STATUS_NOT_AN_OBJECT,
   JSON_STATUS_DUPLICATE_MEMBER, JSON_STATUS_NOT_A_VALUE or
   JSON_STATUS_OUT_OF_MEMORY */
extern json_status_t json_insert_member_into_object( json_value_t *vobject,
                                                     const unsigned char *name,
                                                     json_value_t *value );

/* Remove an object member, given its name. Return the current value in
   case of success or NULL in case of error (which could mean either
   JSON_STATUS_NOT_AN_OBJECT or JSON_STATUS_NOT_A_MEMBER). It is up to the
   caller to then free the member value by calling json_free_value().
   Note that in case of success the member name is returned in the output
   argument name_to_free and must be freed by the caller when appropriate. */

extern json_value_t *json_remove_member_from_object( json_value_t *vobject,
                                                     const unsigned char *name,
                                                     unsigned char **name_to_free );

/* Replace an object member value, given its name. Return the previous member
   value in case of success or NULL in case of failure (which could be one
   of the following: JSON_STATUS_NOT_AN_OBJECT or JSON_STATUS_NOT_A_MEMBER.
   The new value is used as the member value and it is up to the caller to
   fre the previous value bay calling json_free_value() */
extern json_value_t *json_replace_member_value_in_object(
                                                    json_value_t *vobject,
                                                    const unsigned char *name,
                                                    json_value_t *value );

#endif /* __JSONEDIT_H__ */
