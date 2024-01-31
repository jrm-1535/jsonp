
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "jsonedit.h"
#include "jsondata.h"
#include "jsonutf8.h"

extern json_status_t json_insert_element_into_array( json_value_t *varray,
                                                     unsigned int index,
                                                     json_value_t *value )
{
    if ( NULL == varray ) return JSON_STATUS_NOT_AN_ARRAY;
    if ( NULL == value ) return JSON_STATUS_NOT_A_VALUE;

    array_t *array = varray->vdata.array;
    if ( index > array->nb_used ) return JSON_STATUS_OUT_OF_BOUND;

    if ( array->nb_allocated == 1 + array->nb_used ) { // array must be extended
        if ( NULL == array_grow( array ) ) {
            return JSON_STATUS_OUT_OF_MEMORY;
        }
    }

    // FIXME: add lock to make the call safe in case of multithreading
    // FIXME: move any iterator beyond index to index+1
    if ( index < array->nb_used ) {
        element_t **from = array->elements + index;
        element_t ** to = array->elements + index + 1;
        size_t nb = array->nb_used - index;
        memmove( to, from, nb * sizeof( element_t *) );
    }

    array->elements[ index ] = value;
    ++array->nb_used;
    return JSON_STATUS_SUCCESS;
}

extern json_value_t *json_replace_element_in_array( json_value_t *varray,
                                                    unsigned int index,
                                                    json_value_t *value )
{
    if ( NULL == value || NULL == varray ) return NULL;

    array_t *array = varray->vdata.array;
    if ( index >= array->nb_used ) return NULL;

    json_value_t *previous_value = array->elements[ index ];
    array->elements[ index ] = value;
    return previous_value;
}

extern json_value_t *json_remove_element_from_array( json_value_t *varray,
                                                     unsigned int index )
{
    if ( NULL == varray ) return  NULL;

    array_t *array = varray->vdata.array;
    if ( index >= array->nb_used ) return NULL;

    json_value_t *value = array->elements[ index ];
    // FIXME: make this code multi-thread safe
    if ( index < --array->nb_used ) {
        for ( element_iterator_t *curit = array->iterators;
                                                curit; curit = curit->next ) {
            // move back (-1) any iterator at or  beyond index
            if ( curit->index >= index ) --curit->index;
        }
        element_t **from = array->elements + index +1;
        element_t ** to = array->elements + index;
        size_t nb = array->nb_used - index;
        memmove( to, from, nb * sizeof( element_t *) );
    }
    return value;
}

extern json_status_t json_insert_member_into_object( json_value_t *vobject,
                                                     const unsigned char *name,
                                                     json_value_t *value )
{
    if ( NULL == name )    return JSON_STATUS_NOT_A_MEMBER_NAME;
    if ( NULL == value )   return JSON_STATUS_NOT_A_VALUE;

    if ( NULL == vobject || NULL == vobject->vdata.object )
        return JSON_STATUS_NOT_AN_OBJECT;

    if ( ! json_is_utf8_string( name ) )
        return JSON_STATUS_INVALID_STRING;

    object_t *object = vobject->vdata.object;
    member_t *entry = object_find_member( object, name, NULL );
    if ( entry ) {
        return JSON_STATUS_DUPLICATE_MEMBER;    // member exists already, bail out
    }

    object_make_room( object );                 // extend if needed/possible

    // duplicate name - copy will be freed with member
    unsigned char *member_name = (unsigned char *)strdup( (const char *)name );
    if ( NULL == member_name )
        return JSON_STATUS_OUT_OF_MEMORY;

    member_t *member = new_member( member_name, value );
    if ( NULL == member )      // don't free value, it still belongs to caller
        return JSON_STATUS_OUT_OF_MEMORY;

    unsigned int index = member->hash % object->modulo;
    object_store_member( object, index, member );
    return JSON_STATUS_SUCCESS;
}

extern json_value_t *json_replace_member_value_in_object(
                                                    json_value_t *vobject,
                                                    const unsigned char *name,
                                                    json_value_t *value )
{
    if ( NULL == vobject || NULL == name || NULL == value ) return NULL;

    object_t *object = vobject->vdata.object;
    member_t *member = object_find_member( object, name, NULL );
    if ( NULL == member ) return NULL;

    json_value_t *previous_value = member->value;
    member->value = value;
    return previous_value;
}

extern json_value_t *json_remove_member_from_object(
                                                json_value_t *vobject,
                                                const unsigned char *name,
                                                unsigned char **name_to_free )
{
    if ( NULL == vobject || NULL == name || NULL == name_to_free ) return NULL;

    object_t *object = vobject->vdata.object;
    unsigned int index;
    member_t *member = object_find_member( object, name, &index );
    if ( NULL == member ) return NULL;

    *name_to_free = member->name;
    json_value_t *value = member->value;

    object_remove_member( object, index, member );
    return value;
}

static inline json_value_t *free_value_return_NULL( json_value_t *v )
{
    json_free_value( v );
    return NULL;
}

extern json_value_t *json_duplicate_value( const json_value_t *value )
{
    json_value_t *res = malloc( sizeof( json_value_t ) );
    if ( NULL == res ) return NULL;

    json_value_type_t vtype = json_get_value_type( value );
    res->vtype = vtype;

    json_number_type_t ntype;
    switch( vtype ) {   /* Recursively duplicate the value and its children
                           values in case of array or objects. */
    default:
        return free_value_return_NULL( res );

    case JSON_OBJECT:
        res->vdata.object = new_object( );
        if ( NULL == res->vdata.object ) return free_value_return_NULL( res );
        {
            json_object_iterator_t objit = json_new_object_iterator( value );
            while ( true ) {
                const unsigned char *mname;
                const json_value_t *mval = json_iterate_object_member(
                                                            &objit, &mname );
                if ( NULL == mval ) break;

                json_value_t *duplicate = json_duplicate_value( mval );
                if ( NULL == duplicate )
                    return free_value_return_NULL( res );

                json_status_t status = json_insert_member_into_object(
                                                    res, mname, duplicate );
                if ( JSON_STATUS_SUCCESS != status )
                    return free_value_return_NULL( res );
            }
            json_free_object_iterator( objit );
        }
        break;

    case JSON_ARRAY:
        res->vdata.array = new_array( );
        if ( NULL == res->vdata.array ) return free_value_return_NULL( res );
        {
            json_array_iterator_t arrit = json_new_array_iterator( value );
            for ( unsigned int index = 0; ; ++index ) {
                const json_value_t *element = json_iterate_array_element(
                                                                &arrit );
                if ( NULL == element ) break;

                json_value_t *duplicate = json_duplicate_value( element );
                if ( NULL == duplicate )
                    return free_value_return_NULL( res );

                json_status_t status = json_insert_element_into_array(
                                                    res, index, duplicate );
                if ( JSON_STATUS_SUCCESS != status )
                    return free_value_return_NULL( res );
            }
            json_free_array_iterator( arrit );
        }
        break;

    case JSON_STRING:
        res->vdata.string = (unsigned char *)strdup(
                                        (const char *)(value->vdata.string) );
        if ( NULL == res->vdata.string )
            return free_value_return_NULL( res );
        break;

    case JSON_NUMBER:
        ntype = json_get_value_number_type( value );
        if ( JSON_INTEGER_NUMBER == ntype ) {
            res->vdata.number = new_number( JSON_INTEGER_NUMBER,
                                        json_get_integer_value( value ), 0 );
        } else {
            res->vdata.number = new_number( JSON_REAL_NUMBER, 0,
                                        json_get_real_value( value ) );
        }
        if ( NULL == res->vdata.number )
            return free_value_return_NULL( res );
        break;

    case JSON_BOOLEAN:
        res->vdata.boolean = json_get_boolean_value( value );
        break;

    case JSON_NULL:
        break;
    }
    return res;
}

extern json_value_t *json_new_value( json_value_type_t type, ... )
{
    va_list ap;
    va_start( ap, type );

    json_value_t *res = malloc( sizeof( json_value_t ) );
    if ( NULL == res ) return NULL;

    res->vtype = type;
    json_number_type_t nb_type;
    char *string;
    switch( type ) {
    case JSON_STRING: // FIXME: check if string is valid UTF8 here
        string = va_arg(ap, char *);
        if ( json_is_utf8_string( (unsigned char *)string ) ) {
            res->vdata.string = (unsigned char *)strdup( string );
            break;
        } // else falls in default case and return error (next line needed)
        // fall through
    default:
        free( res );
        res = NULL;
        break;
    case JSON_OBJECT:
        res->vdata.object = new_object( );
        break;
    case JSON_ARRAY:
        res->vdata.array = new_array( );
        break;
    case JSON_NUMBER: // litterate numbers must be suffixed with LL to indicate long long
        nb_type = va_arg( ap, json_number_type_t );
        if ( JSON_INTEGER_NUMBER == nb_type ) {
            res->vdata.number = new_number( JSON_INTEGER_NUMBER,
                                            va_arg( ap, long long int), 0 );
        } else {
            res->vdata.number = new_number( JSON_REAL_NUMBER, 0,
                                            va_arg( ap, double) );
        }
        break;
    case JSON_BOOLEAN:
        res->vdata.boolean = va_arg( ap, int ); // warning: bool arg is extended to int
        break;
    case JSON_NULL:
        break;
    }
    va_end( ap );
    return res;
}
