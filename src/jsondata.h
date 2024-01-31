
#ifndef __JSONDATA_H__
#define __JSONDATA_H__

/* Internal json library data structures */

#ifdef JSON_DEBUG
#define JSON_DEBUG_ASSERT( _assertion ) assert( assertion );
#else
#define JSON_DEBUG_ASSERT( _assertion )
#endif

#include <stdint.h>

/*  -----------------------------------------------------------------
    json tree node: value_data_t, which can be:
    - null (no value)
    - boolean (bool)
    - number (either long long int or double)
    - string (C-style zero terminated string)
    - array element (vectors or linked list of value_data_t)
    - object member (hash tables or linked list of members { name, value_data_t })

    Note that linked lists may be used here instead of hash tables or vectors
    for 3 reasons:
    - it is expected that lists are often not huge (small objects and arrays)
    - the main way of accessing members or array elements is by iterators
    - hash tables require additional management that can slow down the
      parser and increase its size.

    Whether hash tables and vectors are used insted of linked lists is
    controlled by the compile flag -D_JSON_FAST_ACCESS_LARGER_CODE
    -----------------------------------------------------------------  */

#ifdef _JSON_FAST_ACCESS_LARGER_CODE

#define MIN_MEMBER_NUMBER   16 // this value MUST be a power of 2

/* iterate over members in a hash table with collision list.


   The iteration order is first valid main entry, then members in the
   collision list if it is not empty, and then next valid main entry,
   and so on, until the end of the main table.
*/
typedef struct _member_iterator {
    struct _member_iterator *next;   // next iterator
    struct _member          *member; // actual, possibly in a collision list
    struct _object          *object; // parent object
} member_iterator_t;

/*
    Member table:
    - non-populated entries in the hash table have a NULL pointer
    - populated entries have a pointer to the member structure, which may
      point to following members in a collision chain.

    Separate pointers (iprev & inext) are used for iterating over all members.
    The object keeps track of the first member in the iteration list (ihead).
    This list is used solely for iterating over the object members in
    sequential order (hashing is used for random access). The order is the
    reverse insertion order (last inserted first).
*/
typedef struct _member {
    struct _member      *next;       // linked list only in case of collisions
    unsigned char       *name;       // member name
    json_value_t        *value;      // member value
    struct _member      *iprev;      // previous in iteration list
    struct _member      *inext;      // next in iteration list
    uint32_t            hash;        // hash(name)
} member_t;

typedef struct _object {
    member_iterator_t *iterators;    // list of iterators on this object
    member_t          **members;     // member table
    member_t          *ihead;        // pointer to head of insertion list
    member_t          *itail;        // pointer to tail of insertion list
    uint32_t          nb_used;       // nb members in the table
    uint32_t          nb_allocated;  // capacity of the table
    uint32_t          modulo;        // modulo used to locate a hash
    unsigned int      max_collision; // length of the worst collision chain
} object_t;

typedef struct _element_iterator {
    struct _element_iterator  *next;  // next in iterator list
    struct _array             *array; // parent array
    unsigned int              index;  // index in parent array
} element_iterator_t;

#define MIN_ELEMENT_NUMBER 10
typedef json_value_t element_t;
typedef struct _array {
    element_iterator_t *iterators;    // list of iterators
    element_t          **elements;    // array of elements *
    unsigned int       nb_allocated;  // array size
    unsigned int       nb_used;       // array portion in use
} array_t;

#else /* slower implemetation but smaller code */

typedef struct _member {
    struct _member  *next;            // next in member list
    unsigned char   *name;
    value_t         *value;
} member_t;

typedef struct _member member_iterator_t;
typedef struct _member object_t;

typedef struct _element {
    struct _element *next;            // next in element list
    value_t         *value;
} element_t;

typedef struct _element array_t;
typedef struct _element element_iterator_t;
#endif

typedef union {
    object_t            *object;   // JSON_OBJECT
    array_t             *array;    // JSON_ARRAY
    unsigned char       *string;   // JSON_STRING
    struct _number      *number;   // JSON_NUMBER
    bool                boolean;   // JSON_BOOLEAN
} value_data_t;

struct _value {
    json_value_type_t   vtype;
    value_data_t        vdata;     // depending on vtype
};

typedef union {
    long long           integer;
    double              real;
} number_data_t;

typedef struct _number {
    json_number_type_t  ntype;
    number_data_t       ndata;
} number_t;

/* internal use only */
object_t *new_object( void );
void json_free_object( object_t *object );

member_t *new_member( unsigned char *name, json_value_t *value );
object_t *object_attach_member( object_t *object, member_t *member,
                                member_t **last_member );

#ifdef _JSON_FAST_ACCESS_LARGER_CODE
member_t *object_locate_existing_member( object_t *object, uint32_t hash,
                                         const unsigned char *name,
                                         unsigned int *pindex );
member_t *object_find_member( object_t *object, const unsigned char *name,
                              unsigned int *pindex );
bool object_make_room( object_t *object );
void object_store_member( object_t *object, unsigned int index, member_t *member );
void object_remove_member( object_t *object, unsigned int index, member_t *member );
json_value_t **array_grow( array_t *array ); // return NULL in case of failure
#endif

array_t *new_array( void );
void json_free_array( array_t *array );

element_t *new_element( json_value_t *value );
array_t *array_append_element( array_t *array, element_t *element, element_t **last_element );

number_t *new_number( json_number_type_t nbtype,
                      long long int integer, double real );

#endif /* __JSONDATA_H__ */
