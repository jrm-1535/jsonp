
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "jsonvalue.h"
#include "jsondata.h"
#include "jsonedit.h"

#ifdef _JSON_FAST_ACCESS_LARGER_CODE
/*  -----------------------------------------------------------------
    hash table manipulation
    -----------------------------------------------------------------  */

/*
  Zero terminated UTF8 string hashing,
  based on Bob Jenkin's one-at-a-time hash function.
*/
static uint32_t UTF8_string_hash( const unsigned char *string )
{
    uint32_t hash = 0;
    unsigned char c;

    while ( ( c = *string++ ) ) {
        hash += c;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    return hash + (hash << 15);;
}

/* free the member possibly in a collision chain list.
   Does not free the member name nor value.
   Does not attempt to shrink the hash table,
     as entries are randomly assigned
   Does not update max_collision as it does not know
     if max_colllision was reached at multiple places.
   Check if any iterator is at this member and move it
   back to the previous member in the iterator list.
   Update collision list, update links in iteration list. */
void object_remove_member( object_t *object, unsigned int index, member_t *member )
{
    // FIXME: make sure this code is multi-thread safe
    member_t *prev = NULL, *cur = object->members[index];
    while ( cur != member ) {
        prev = cur;
        cur = cur->next;
        assert( cur );
    }

    for ( member_iterator_t *membit = object->iterators;
                                            membit; membit = membit->next ) {
        if ( member == membit->member ) {
            membit->member = member->iprev;
        }
    }
    if ( prev ) prev->next = member->next;
    else        object->members[index] = member->next;

    if ( member->iprev )  member->iprev->inext = member->inext;
    if ( member->inext )  member->inext->iprev = member->iprev;
    if ( object->ihead == member ) object->ihead = member->inext;
    if ( object->itail == member ) object->itail = member->iprev;

    free( member );
    --object->nb_used;
}

// member is an already allocated and filled member, index is where to store
// its pointer in the object member table - possibly as a collsion.
// object ihead and itail are assumed to be correct at the time of the call.
void object_store_member( object_t *object, unsigned int index, member_t *member )
{
    unsigned int count = 0;
    // FIXME: make sure this code is multi-thread safe
    if ( object->members[index] ) { // already valid entry
        ++count;                    // collision

        member_t *existing = object->members[index];
        while ( existing->next ) {
            existing = existing->next;
            ++count;                // more collitions
        }
        existing->next = member;
    } else {
        object->members[index] = member;
    }

    if ( object->max_collision < count )
        object->max_collision = count;

    member->inext = NULL;
    member->iprev = object->itail;

    if ( object->itail ) {
        object->itail->inext = member;
    } else {
        object->ihead = member;
    }
    object->itail = member;
    ++object->nb_used;
}

static void object_shuffle_members( object_t *object, member_t **old_table )
{
    assert( NULL == object->ihead->iprev );
    assert( NULL == object->itail->inext );

    member_t *old_start = object->ihead;    // save head of old member list
    object->ihead = object->itail = NULL;   // start with new empty member table

    member_t *old_next = NULL;              // for each old member
    for ( member_t *member = old_start; member; member = old_next ) {
        old_next = member->inext;           // temporarily save next old member
        // make an identical new member (not in a collison chain by default)
        member->next = NULL;
        unsigned int index = member->hash % object->modulo; // new table index
        assert( index < object->nb_allocated );
        object_store_member( object, index, member );
    }
    free( old_table );
}

static uint32_t get_prime( uint32_t size )
{
    unsigned int i;
    static const uint32_t greatest_prime[] = {
        1,           /* 2^00            1 */
        2,           /* 2^01            2 */
        3,           /* 2^02            4 */
        7,           /* 2^03            8 */
        13,          /* 2^04           16 */
        31,          /* 2^05           32 */
        61,          /* 2^06           64 */
        127,         /* 2^07          128 */
        251,         /* 2^08          256 */
        509,         /* 2^09          512 */
        1021,        /* 2^10         1024 */
        2039,        /* 2^11         2048 */
        4093,        /* 2^12         4096 */
        8191,        /* 2^13         8192 */
        16381,       /* 2^14        16384 */
        32749,       /* 2^15        32768 */
        65521,       /* 2^16        65536 */
        131071,      /* 2^17       131072 */
        262139,      /* 2^18       262144 */
        524287,      /* 2^19       524288 */
        1048573,     /* 2^20      1048576 */
        2097143,     /* 2^21      2097152 */
        4194301,     /* 2^22      4194304 */
        8388593,     /* 2^23      8388608 */
        16777213,    /* 2^24     16777216 */
        33554393,    /* 2^25     33554432 */
        67108859,    /* 2^26     67108864 */
        134217689,   /* 2^27    134217728 */
        268435399,   /* 2^28    268435456 */
        536870909,   /* 2^29    536870912 */
        1073741789,  /* 2^30   1073741824 */
        2147483647,  /* 2^31   2147483648 */
        4294967291,  /* 2^32   4294967296 */
    };

    /* assuming size is a power of 2 */
    for ( i = 0; size ; ++i )
        size >>= 1;
    if ( i > sizeof( greatest_prime ) / sizeof( unsigned int ) )
        return 0;            // can't extend more than 64 GB!
    return greatest_prime[i-1];
}

bool object_make_room( object_t *object )
{
    /* if less than 25% left or more than 4 colliding entries in list, double the size */
    if ( ( 4 * (1 + object->nb_used) >= 3 * (object->nb_allocated) ) ||
                                             object->max_collision > 4 ) {
        uint32_t old_size = object->nb_allocated;

        if ( 2147483648 == old_size ) // 2^31 is max object size
            return false;

        /* starting from MIN_MEMBER_NUMBER (power of 2), double the size */
        unsigned int new_allocated = ( old_size ) ?  2 * old_size : MIN_MEMBER_NUMBER;

        member_t **old_table = object->members;
        member_t **new_table = malloc( sizeof(member_t *) * new_allocated );
        if ( NULL == new_table )
            return false;       // keep existing object if it cannot be extended

        memset( (void *)new_table, 0, sizeof(member_t *) * new_allocated );
        object->members = new_table;
        object->nb_allocated = new_allocated;
        object->nb_used = 0;      // updated by object_shuffle_members if needed
        object->modulo = get_prime( new_allocated );
        assert ( object->modulo ); // guaranteed if max size is 2^31 only
        object->max_collision = 0;

        if ( old_table ) {
            object_shuffle_members ( object, old_table );
        }
        return true;         // object has been extended
    }
    return false;          // no need to extend
}

member_t *object_locate_existing_member( object_t *object, uint32_t hash,
                                         const unsigned char *name,
                                         unsigned int *pindex )
{
    if ( NULL == object->members )
        return NULL;

    unsigned int index = hash % object->modulo;
    member_t *member = object->members[index];

    while( member ) {
        if ( 0 == strcmp( (const char *)member->name, (const char *)name ) )
            break;
        member = member->next;
    }
    if ( pindex )
        *pindex = index;

    return member;
}

member_t *object_find_member( object_t *object, const unsigned char *name,
                              unsigned int *pindex )
{
    uint32_t hash = UTF8_string_hash( name );
    return object_locate_existing_member( object, hash, name, pindex );
}

#if 0
void debug_hash_table( hash_table_t *ht )
{
    printf( "Hash Table %p information\n", ht );
    printf( "  allocated room  %d\n", ht->allocated );
    printf( "  modulo          %d\n", ht->modulo );
    printf( "  max collisions  %d\n", ht->max_collision );
    printf( "  nb entries      %d\n\n", ht->nb );

    printf( "  Entries {\n" );
    hash_entry_t *entries = ht->table;
    if ( entries ) {
        for( int i = 0; i < ht->allocated; i++, entries++ ) {
            if ( entries->key ) {
                printf( "   Index %d, key %zu, hash %zu\n",
                        i, (size_t)(entries->key), entries->hash );
                for ( hash_entry_t *collide = entries->next; collide;
                                              collide = collide->next ) {
                    printf( "   Index %d, key %zu, hash %zu\n",
                            i, (size_t)(collide->key), collide->hash );
                }
            }
        }
    }
    printf( "  }\n");
}
#endif
#endif

/*  -----------------------------------------------------------------
    freeing json object tree
    -----------------------------------------------------------------  */

void json_free_array( array_t *array )
{
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    if ( NULL == array ) return;
    element_iterator_t *eitn;
    for ( element_iterator_t *eit = array->iterators; eit; eit = eitn ) {
        eitn = eit->next;
        free( eit );
    }

    unsigned int i = array->nb_used;
    while ( i-- ) {
        json_free_value( array->elements[i] );
    }

    free( array->elements );
    free( array );
#else
    element_t *next;
    for ( element_t *cur = array; cur; cur = next ) {
        json_free_value( cur->value );
        next = cur->next;
        free( cur );
    }
#endif
}

void json_free_object( object_t *object )
{
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    if ( NULL == object ) return;
    member_iterator_t *mitn;
    for ( member_iterator_t *mit = object->iterators; mit; mit = mitn ) {
        mitn = mit->next;
        free( mit );
    }

    member_t **mbp = object->members;
    unsigned int nb = object->nb_used;
    while ( nb ) {
        member_t *mbn;
        for ( member_t *mb = *mbp; mb; mb = mbn ) {
            free( mb->name );
            json_free_value( mb->value );
            mbn = mb->next;
            free( mb );
            --nb;
        }
        ++mbp;
    }
    free( object->members );
    free( object );
#else
    member_t *mbn;
    for ( member_t *mb = object; mb; mb = mbn ) {
        free( mb->name );
        json_free_value( mb->value );
        mbn = mb->next;
        free( mb );
    }
#endif
}

extern void json_free_value( json_value_t *value )
// recursively free sub trees, if arrays or objects, before freeing the value
{
    if ( NULL == value ) return; // json_free_value( NULL ) is valid

    switch( value->vtype ) {
    default:
        JSON_DEBUG_ASSERT(0);
        return;
    case JSON_OBJECT:
        json_free_object( value->vdata.object );
        break;
    case JSON_ARRAY:
        json_free_array( value->vdata.array );
        break;
    case JSON_STRING:
        free( value->vdata.string );
        break;
    case JSON_NUMBER:
        free( value->vdata.number );
        break;
    case JSON_BOOLEAN:  case JSON_NULL:
        break;
    }
    free( value );
}

void json_free( json_value_t *root )
{
    json_free_value( root );
}

/*  -------------------------------------------------------------------
    internal JSON tree access (no editing)
    -------------------------------------------------------------------  */

extern json_value_type_t json_get_value_type( const json_value_t *value )
{
    if ( NULL == value ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NOT_A_JSON_VALUE;
    }
    switch( value->vtype ) {
    default:
        JSON_DEBUG_ASSERT(0);
        return NOT_A_JSON_VALUE;

    case JSON_OBJECT: case JSON_ARRAY: case JSON_STRING:
    case JSON_NUMBER: case JSON_BOOLEAN:  case JSON_NULL:
        break;
    }
    return value->vtype;
}

extern bool json_get_boolean_value( const json_value_t *value )
{
    if ( NULL == value || JSON_BOOLEAN != value->vtype ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return false;
    }
    return value->vdata.boolean;
}

extern const unsigned char *json_get_string_value( const json_value_t *value )
{
    if ( NULL == value || JSON_STRING != value->vtype ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NULL;
    }
    return (const unsigned char *)value->vdata.string;
}

extern json_number_type_t json_get_value_number_type(
                                                    const json_value_t *value )
{
    if ( NULL == value || JSON_NUMBER != value->vtype ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NOT_A_JSON_NUMBER;
    }
    return value->vdata.number->ntype;
}

extern long long int json_get_integer_value( const json_value_t *value )
{
    if ( NULL == value || JSON_NUMBER != value->vtype ||
         JSON_INTEGER_NUMBER != value->vdata.number->ntype) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return 0;
    }
    return value->vdata.number->ndata.integer;
}

extern double json_get_real_value( const json_value_t *value )
{
    if ( NULL == value || JSON_NUMBER != value->vtype ||
         JSON_REAL_NUMBER != value->vdata.number->ntype) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return 0.0;
    }
    return value->vdata.number->ndata.real;
}

extern json_object_iterator_t json_new_object_iterator(
                                             const json_value_t *value )
{
    if ( NULL == value || JSON_OBJECT != value->vtype ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NULL;
    }
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    member_iterator_t *iterator = malloc( sizeof( member_iterator_t ) );
    if ( NULL == iterator ) return NULL;

    iterator->object = value->vdata.object;
#if 0
    if ( 0 == iterator->object->nb_used ) {
        iterator->member = NULL;
    } else {
        iterator->member = iterator->object->ihead;
    }
#else
    iterator->member = NULL;
#endif
    iterator->next = iterator->object->iterators;
    iterator->object->iterators = iterator;

    return iterator;
#else
    return value->vdata.object;
#endif
}

extern const json_value_t *json_iterate_object_member(
                                        json_object_iterator_t *object_iterator,
                                        const unsigned char **name )
{
    if ( NULL == object_iterator ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NULL;
    }

    member_iterator_t *iterator = *object_iterator;
    if ( NULL == iterator ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NULL;
    }
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    if ( NULL == iterator->object ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NULL;
    }

    if ( 0 == iterator->object->nb_used ) return NULL;

    member_t *member = iterator->member;
    if ( NULL == member ) {
        member = iterator->object->ihead;
    } else {
        member = member->inext;
        if ( NULL == member ) return NULL;
    }
    iterator->member = member;
#else
    member_t *member = iterator;
    *object_iterator = (void *)(iterator->next);
#endif
    *name = (const unsigned char *)member->name;
    return member->value;
}

extern void json_free_object_iterator( json_object_iterator_t object_iterator )
{
    if ( NULL == object_iterator ) return; // free(null) is valid
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    member_iterator_t *iterator = object_iterator;
    object_t *object = iterator->object;
    if ( NULL == object ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return;
    }
    member_iterator_t *pmit = NULL;
    for ( member_iterator_t *mit = object->iterators; mit;
                                            pmit = mit, mit = mit->next ) {
        if ( mit == iterator ) {
            if ( pmit ) pmit->next = mit->next;
            else object->iterators = mit->next;
            free( mit );
            return;
        }
    }
#endif
}

extern json_array_iterator_t json_new_array_iterator(
                                                    const json_value_t *value )
{
    if ( NULL == value || JSON_ARRAY != value->vtype ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NULL;
    }
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    element_iterator_t *iterator = malloc( sizeof( element_iterator_t ) );
    if ( NULL == iterator ) return NULL;

    iterator->index = 0;
    iterator->array = value->vdata.array;

    iterator->next  = iterator->array->iterators;
    iterator->array->iterators = iterator;

    return iterator;
#else
    return value->vdata.array;
#endif
}

extern const json_value_t *json_iterate_array_element(
                                        json_array_iterator_t *array_iterator )
{
    if ( NULL == array_iterator ) {
        JSON_DEBUG_ASSERT(0); // exported API may be called with wrong args
        return NULL;
    }
    element_iterator_t *iterator = *array_iterator;
    if ( NULL == iterator ) return NULL;

#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    unsigned int index = iterator->index;
    if ( index >= iterator->array->nb_used ) return NULL;

    const json_value_t *value = iterator->array->elements[index];
    iterator->index = 1 + index;
    return value;
#else
    *array_iterator = iterator->next;
    return iterator->value;
#endif
}

extern void json_free_array_iterator( json_array_iterator_t array_iterator )
{
    if ( NULL == array_iterator ) return;
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    element_iterator_t *iterator = array_iterator;
    if ( NULL == iterator ) {
        assert(0);    // should never happen
        return;
    }
    array_t *array = iterator->array;
    if ( NULL == array ) {
        assert(0);    // should never happen either
        return;
    }
    element_iterator_t *peit = NULL;
    for ( element_iterator_t *eit = array->iterators; eit;
                                            peit = eit, eit = eit->next ) {
        if ( eit == iterator ) {
            if ( peit ) peit->next = eit->next;
            else array->iterators = eit->next;
            free( eit );
            return;
        }
    }
#endif
}

extern const json_value_t *json_search_for_object_member_by_name(
                                                const json_value_t *vobject,
                                                const unsigned char *name )
{
    if ( NULL == vobject || JSON_OBJECT != vobject->vtype || NULL == name )
        return NULL;             // but "" is a valid member name
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    uint32_t hash = UTF8_string_hash( name );
    member_t *member = object_locate_existing_member( vobject->vdata.object,
                                                      hash, name, NULL );
    if ( NULL == member ) return NULL;
    return member->value;
#else
    /* inefficient linear search:
      if objects can be large, it is better to create a hash table */
    for ( member_t *m = vobject->vdata.object; m; m = m->next ) {
        if ( 0 == strcmp( (const char *)m->name, (const char *)name ) )
            return (const value_t *)m->value;
    }
    return NULL;
#endif
}

extern const json_value_t *json_get_array_element( const json_value_t *array,
                                                   unsigned int index )
{
    if ( NULL == array || JSON_ARRAY != array->vtype ) return NULL;
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    if ( index >= array->vdata.array->nb_used ) return NULL;
    return array->vdata.array->elements[index];
#else
    /* extremely inefficient linear access to element[index].
      if arrays can be large, it is better to create a resizable array */
    element_t *element;
    for ( element = array->vdata.array; element && index;
                                            --index, element = element->next );
    return element->value;
#endif
}

extern int json_get_object_member_count( const json_value_t *value )
{
    if ( NULL == value || JSON_OBJECT != value->vtype ) {
        assert(0);
        return -1;
    }
#ifdef  _JSON_FAST_ACCESS_LARGER_CODE
    return (int)value->vdata.object->nb_used;
#else
    unsigned int count = 0;
    for ( member_t *member = value->vdata.object; member; member = member->next )
        ++count;
    return (int)count;
#endif
}

extern int json_get_array_size( const json_value_t *value )
{
    if ( NULL == value || JSON_ARRAY != value->vtype ) {
        assert(0);
        return -1;
    }
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    return (int)value->vdata.array->nb_used;
#else
    unsigned int count = 0;
    for ( element_t *element = value->vdata.array; element; element = element->next )
        ++count;
    return (int)count;
#endif
}

/*  -------------------------------------------------------------------
    Json tree editing: adding, deleting, modifying values and members
    -------------------------------------------------------------------  */

member_t *new_member( unsigned char *name, json_value_t *value )
{
    member_t *member = malloc( sizeof( member_t ) );
    if( member ) {
        member->next = NULL;
        member->name = name; // (*) name must have been allocated or duplicated !!
        member->value = value;
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
        member->hash = UTF8_string_hash( name );
//        member->inext = member->iprev = NULL;
#endif
    } else {
        free( name );       // callers free value as appropriate
    }
    return member;
}

void free_member( member_t *member )
{
    free( member->name );
    json_free_value( member->value );
    free( member );
}

object_t *new_object( void )
{
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    object_t *object = malloc( sizeof( object_t ) );
    if ( NULL == object ) return NULL;

    object->iterators = NULL;
    object->nb_used = 0;
    object->nb_allocated = 0;
    object->modulo = 0;
    object->max_collision = 0;
    object->members = NULL;
    object->ihead = NULL;
    object->itail = NULL;
    if ( ! object_make_room( object ) ) { // try to create an initial table
        free( object );                   // failed (no room), bail out
        return NULL;
    }
    return object;
#else
    return NULL;
#endif
}

// only used by the parser
object_t *object_attach_member( object_t *object, member_t *member,
                                member_t **last_member )
{
    assert( member );
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    (void)last_member;  // suppress GCC warning
    assert( object );
    member_t *entry = object_locate_existing_member( object, member->hash, member->name, NULL );
    if ( entry ) {
        printf( "json_parse: member %s exists already in object, ignoring duplicate\n",
                member->name );
        free_member( member );
        return object;                          // member exists already, ignore
    }
    object_make_room( object );                 // extend if needed

    unsigned int index = member->hash % object->modulo; // get possibly new index
    object_store_member( object, index, member );
#else
    for ( member_t *in_obj = object; in_obj; in_obj = in_obj->next ) {
        if ( 0 == strcmp( (const char *)in_obj->name, (const char *)member->name ) ) {
            printf( "json_parse: member %s exists already in object, ignoring duplicate\n",
                    member->name );
            free_member( member );
            return object;                      // member exists already, ignore
        }
    }
    if ( *last_member )
        (*last_member)->next = member;
    else
        object = member;

     *last_member = member;
#endif
    return object;
}

array_t *new_array( void )
{
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    array_t *array = malloc( sizeof( array_t  ) );
    if ( NULL == array ) return NULL;

    memset( array, 0, sizeof( array_t ) );
    array->elements = malloc( sizeof( element_t *) * MIN_ELEMENT_NUMBER );
    if ( NULL == array->elements ) {
        free( array );                    // bail out
        return NULL;
    }
    array->nb_allocated = MIN_ELEMENT_NUMBER;
    return array;
#else
    return NULL;
#endif
}

#ifdef _JSON_FAST_ACCESS_LARGER_CODE
/* returns NULL is it can't grow the array, non-null if it can */
element_t **array_grow( array_t *array )
{
    unsigned int nb_allocated = array->nb_allocated * 2;
    element_t **new_elements = realloc( array->elements,
                                        sizeof( element_t *) * nb_allocated );
    if ( NULL == new_elements ) {
        return NULL;      // do not touch the original array.
    }
    array->elements = new_elements;
    array->nb_allocated = nb_allocated;
    return new_elements;
}
#endif

/* used only during parsing. Failure to grow the array is fatal */
array_t *array_append_element( array_t *array, element_t *element,
                               element_t **last_element )
{
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    (void)last_element;  // suppress GCC warning
    if ( NULL == array ) return NULL;
    if ( array->nb_used == array->nb_allocated ) {
        if ( NULL == array_grow( array ) ) {
            json_free_array( array );
            return NULL;
        }
    }

    unsigned int offset = array->nb_used++;
    array->elements[ offset ] = element;
#else
    if ( *last_element )
        (*last_element)->next = element;
    else
        array = element;
    *last_element = element;
#endif
    return array;
}

element_t *new_element( json_value_t *value )
{
#ifdef _JSON_FAST_ACCESS_LARGER_CODE
    return (element_t *)value;
#else
    element_t *element = malloc( sizeof(element_t) );
    if ( element ) {
        element->value = value;
        element->next = NULL;
    } else {
        free_json_value( value );
    }
    return element;
#endif
}

number_t *new_number( json_number_type_t nbtype,
                      long long int integer, double real )
{
    number_t *number = malloc( sizeof( number_t ) );
    if ( number ) {
        number->ntype = nbtype;
        if ( JSON_INTEGER_NUMBER == nbtype ) { number->ndata.integer = integer; }
        else                                 { number->ndata.real = real; }
    }
    return number;
}
