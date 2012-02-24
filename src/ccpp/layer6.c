#include "layer6.h"

#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>


/**
 * See l6spec.txt for the specifications of the message protocol and format.
 */

#define L6_FIELD_LENGTH_SCALAR           1
#define L6_FIELD_LENGTH_ARRAY            3
#define L6_FIELD_LENGTH_STRING           3

#define L6_FLAG_FIELD_IS_ARRAY          16
#define L6_FLAG_FIELD_IS_EXTENSION      32
#define L6_FLAG_FIELD_HAS_NAME          64
#define L6_FLAG_FIELD_HAS_ID           128
#define L6_FLAG_FIELD_TYPE_MASK       0x1F
#define L6_FLAG_MSG_IS_TEMPLATE         32


#define L6_PRIMITIVE_DATATYPE_MIN        1
#define L6_PRIMITIVE_DATATYPE_MAX       10
#define L6_ARRAY_DATATYPE_MIN           16
#define L6_ARRAY_DATATYPE_MAX           29
#define L6_ALL_DATATYPES_MAX            32
#define L6_BASE_MSG_HDR_LENGTH           8
#define L6_BASE_FLD_HDR_LENGTH           1
#define L6_MAX_UNUSED_SERIALIZATIONS    16

#define L6_ERR_CODES_MIN                32
#define L6_ERR_CODES_MAX                52
#define L6_ERR_CODES_OFFSET             28
#define L6_ERR_CODES_NUM                14

#define L6_DATATYPE_L6MSG_SERIALIZED    28

//Internal limits
#define L6_MAX_NUM_FIELDS               255
#define L6_FIELD_MAX_COUNT              65535
#define L6_MSG_FIELD_DATA_MAX_SIZE      USHRT_MAX   //(USHRT_MAX - 255)

#define _SET_DEBUG_INFO 1

#define DISALLOW_TRANSPARENT_OVERWRITE   0
#define TRACK_MSG_SIZE_CONTINUALLY       0

#if defined _SET_DEBUG_INFO && _SET_DEBUG_INFO
#include <stdarg.h>
#endif
//redefine this to only char if we don't want const char *keys
#define _const_char const char

static const char* const l6_error_msgs[] = {
        "No error.",
        "Error: Unknown error.",
        "Reserved.",
        "Reserved.",
        "Error: L6_ERR_MEM_ALLOC: Memory allocation _failure.",
        "Error: L6_ERR_INSUFF_BUFFER_SIZE - Insufficient buffer size for serialization.",
        "Error: L6_ERR_INCORRECT_MSG_SIZE - Insufficient buffer size for deserialization.",
        "Error: L6_ERR_FIELD_EXISTS - Field with this id/name already exists",
        "Error: L6_ERR_FIELD_NOT_FOUND - Field not found.",
        "Error: L6_ERR_FIELD_NAME_TOO_LONG - Fieldname longer than 128.",
        "Error: L6_ERR_FIELD_UNNAMED - Field not named.",
        "Error: L6_ERR_FIELD_NO_ID - Field has not ID assigned.",
        "Error: L6_ERR_FIELD_ID_INVALID - Invalid (negative or too large) Field ID provided.",
        "Error: L6_ERR_FIELD_TYPE_INCOMPATIBLE - Requested datatype incompatible with field type.",
        "Error: L6_ERR_FIELD_MAX_COUNT_EXCEEDED - Array element count too high (must be <= 65535.)",
        "Error: L6_ERR_FIELD_MAX_SIZE_EXCEEDED - Field size too high (must be <= 65535.)",
        "Error: L6_ERR_MAX_NUM_FIELDS_EXCEEDED - Total number of fields too high (must be <= 255)",
        "Error: L6_ERR_MAX_MSG_SIZE_EXCEEDED - Total message size too high (must be <= 65535.)",
        "Error: L6_ERR_MSG_LOCKED - Message structure locked for serializing/deserializing.",
        "Error: L6_ERR_RECURSIVE_SUB_MSG - Embedded sub-message recursively embeds this message.",
        "Error: L6_ERR_SUB_MSG_DESERIALIZE - Error while deserializing embedded sub-message.",
        "Error: L6_ERR_OFFSET_OUT_OF_BOUNDS - Memory offset out of bounds.",
        "Error: L6_ERR_UNHANDLED_FIELD_TYPE - Unsupported, unhandled or undefined field extension.",
        "Error: L6_ERR_NOT_IMPLEMENTED - Functionality not implemented.",
        "Reserved.",
        "Warning: L6_WARN_INDEX_OUT_OF_BOUNDS - Array index out of bounds.",
        "Warning: L6_WARN_FIELD_REPLACED - Field replaced."};

const char *l6_datatype(int dtype)
{
    switch(dtype)
    {
        case L6_DATATYPE_UINT8              : return "uint08";
        case L6_DATATYPE_BYTE               : return " int08";
        case L6_DATATYPE_UINT16             : return "uint16";
        case L6_DATATYPE_INT16              : return " int16";
        case L6_DATATYPE_UINT32             : return "uint32";
        case L6_DATATYPE_INT32              : return " int32";
        case L6_DATATYPE_UINT64             : return "uint64";
        case L6_DATATYPE_INT64              : return " int64";
        case L6_DATATYPE_FLOAT              : return " float";
        case L6_DATATYPE_DOUBLE             : return "double";
        case L6_DATATYPE_STRING             : return "string";
        case L6_DATATYPE_BYTES              : return " i08[]";
        case L6_DATATYPE_UINT16_ARRAY       : return "ui16[]";
        case L6_DATATYPE_INT16_ARRAY        : return " i16[]";
        case L6_DATATYPE_UINT32_ARRAY       : return "ui32[]";
        case L6_DATATYPE_INT32_ARRAY        : return " i32[]";
        case L6_DATATYPE_UINT64_ARRAY       : return "ui64[]";
        case L6_DATATYPE_INT64_ARRAY        : return " i64[]";
        case L6_DATATYPE_FLOAT_ARRAY        : return " flt[]";
        case L6_DATATYPE_DOUBLE_ARRAY       : return " dbl[]";
        case L6_DATATYPE_L6MSG              : return " l6msg";
        //case L6_DATATYPE_L6MSG_SERIALIZED   : return "l6msgz";
        //case L6_DATATYPE_STRING_ARRAY       : return " str[]";
        //case L6_DATATYPE_BITFIELD           : return "bitfld";
        case L6_DATATYPE_EXTENSION          : return "extnsn";
        //case L6_DATATYPE_CUSTOM             : return "custom";
    }
    return "unknown";
}



//typedef void* l6msgField;
void keep_in_place(unsigned char *pX, int sz)
{
    return;
}

void byte_swap_16(uint16_t *pX)
{
    *pX = (*pX >> 8 ) | (*pX << 8);
}

void byte_swap_32(uint32_t *pX)
{
    *pX = (*pX >> 24) | ((*pX & 0x00ff0000) >> 8) 
            | ((*pX & 0x0000ff00) << 8) | (*pX << 24);
}

void byte_swap_64(uint64_t *pX)
{
    *pX = ( *pX >> 56) |
          ((*pX << 40) & 0x00FF000000000000ULL) | ((*pX << 24) & 0x0000FF0000000000ULL) |
          ((*pX << 8 ) & 0x000000FF00000000ULL) | ((*pX >> 8 ) & 0x00000000FF000000ULL) |
          ((*pX >> 24) & 0x0000000000FF0000ULL) | ((*pX >> 40) & 0x000000000000FF00ULL) |
           (*pX << 56);
}

void swap_in_place(unsigned char *pX, int sz)
{
    switch(sz) {
    case 1:
        break;
    case 2:
        byte_swap_16((uint16_t*)pX);
        break;
    case 4:
        byte_swap_32((uint32_t*)pX);
        break;
    case 8:
        byte_swap_64((uint64_t*)pX);
        break;

    default: 
    {
        unsigned int c;
        int len = sz>>1;
        int i = 0;
        for(i=0; i < len; i++)
        {
            c = pX[i];
            pX[i] = pX[sz - i - 1];
            pX[sz - i - 1] = c;
        }
    }
    }
    return;
}

/*
************************** PRIVATE METHODS *******************************
*/
//compile time computation to determine if host-to-network byte order needs be changed
void (*change_byte_order)(unsigned char *pX, int sz);// = (htons(5) == 5 ? keep_in_place : swap_in_place);

/* Internal data structures */
//use our own collection data structs

/********* DEFAULT DYNAMIC ARRAY/VECTOR IMPLEMENTATION ********/
//Could use a lot of optimization for the common case, i.e. few (< 10) fields
//Maybe instead of array of pointers to entry structs, an array of entry structs

typedef struct _mvec_t
{
    int     size;
    int     capacity;
    int     starting_cap;
    float   factor;
    void    **items;
} *mvec_t;

static int __l6_mvec_init(void **pq, int elem_size, int start)
{
    //do nothing
    mvec_t q = (*pq) = (mvec_t) malloc(sizeof(struct _mvec_t));
    q->size         = 0;
    q->factor       = 1 + 1.0f;
    if(start <= 0) start = 8;
    q->capacity     = (q->factor < 1.0 ? (start / q->factor) + 1 : start);
    q->starting_cap = q->capacity;
    q->items        = (void**)malloc(sizeof(void*) * q->capacity);
    return 0;
}

static int __l6_mvec_free(void *v)
{
    mvec_t q = (mvec_t)v;
    free(q->items);
    free(q);
    return 0;
}

static int __l6_mvec_resize(void *v, float factor)
{
    mvec_t q = (mvec_t)v;
    int newcap  = (int)(factor * q->capacity) + 1;
    if((newcap < q->starting_cap) || (newcap < q->size)) return 0;  //don't wanna grow too small
    q->capacity = newcap;
    q->items    = (void**)realloc(q->items, sizeof(void*) * q->capacity);
    return 0;
}

static int __l6_mvec_push(void *v, void *field)
{
    mvec_t q = (mvec_t)v;
    if(q->size >= q->capacity)
    {
        //grow
        if(__l6_mvec_resize(q, q->factor) < 0)
            return -1;
    }
    q->items[q->size] = field;
    ++(q->size);
    return 0;
}

static int __l6_mvec_size(void *v)
{
    mvec_t q = (mvec_t)v;
    return q->size;
}

static void *__l6_mvec_get(void *v, int index)
{
    mvec_t q = (mvec_t)v;
    void *field = NULL;
    //if(index < q->size)   //skip size check because it's usually been done by calling method
    field = q->items[index];
    return field;
}


static void *__l6_mvec_remove(void *v, int index)
{
    mvec_t q = (mvec_t)v;
    void *field = NULL;
    if((index >= 0) && (index < q->size))
    {
        field = q->items[index];
        //shift items back
        int i = 0;
        --(q->size);
        for(i = index; i < q->size; i++)
            q->items[i] = q->items[i + 1]; 

        //don't bother shrinking, as typically this happens only when freeing messages
        //if((q->size << 2) <= q->capacity) __l6_mvec_resize(q, (1/q->factor));        //loadfactor <= 0.5
    }
    return field;
}

static int __l6_mvec_index_of(void *v, void *field)
{
    mvec_t q = (mvec_t)v;
    int i = 0;
    for(i = 0; i < q->size; i++)
    {
       if(q->items[i] == field)
           return i;
    }
    return -1;
}


static int __l6_mvec_remove_item(void *v, void *field)
{
    mvec_t q = (mvec_t)v;
    int i = __l6_mvec_index_of(v, field);
    if(i >- 0)
    {
        //remove this
        __l6_mvec_remove(q, i);
        return 1;
    }
    return 0;
}

static void *__l6_mvec_pop(void *v)
{
    mvec_t q = (mvec_t)v;    
    return __l6_mvec_remove(q, q->size - 1);
}

/********* DEFAULT HASHTABLE IMPLEMENTATION ********/
//Could use a lot of optimization for the common case, i.e. few (< 10) fields
//Maybe instead of array of pointers to entry structs, an array of entry structs

static unsigned int murmurhash2 ( void * key, int32_t len, uint32_t seed )
{
    const uint32_t m = 0x5bd1e995;
    const int32_t r = 24;
    uint32_t h = seed ^ len;
    const unsigned char * data = (const unsigned char *)key;
    while(len >= 4)
    {
        unsigned int k = *(unsigned int *)data;
        k *= m;   k ^= k >> r; 
        k *= m;   h *= m;     h ^= k;
        data += 4;    len -= 4;
    }
    switch(len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    };
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}

static uint32_t l6_htbl_hash(void *data, int size, int limit, uint64_t salt)
{
    //TODO: maybe a more efficient hash for really small keys and tables?
    if(size == 0)   //int key
    {
        size = sizeof(int32_t);
        //maybe apply salt to increase entropy a leeeettle bit, esp for 16 bit keys?
        //size = sizeof(salt);
        //salt ^= *((int32_t*)data);
        //data = &salt;
    }
    return murmurhash2(data, size, 20090403);
}

//static uint32_t (*l6_htbl_hash)(char *data, int size int limit) = __l6_htbl_hash;

typedef struct _l6_htbl_entry_t
{
    int         keysize;
    union 
    {
        void    *p;
        int32_t i;
    } key;
    void        *value;
    uint32_t    hash;
    struct _l6_htbl_entry_t *next;
} l6_htbl_entry_t;

typedef struct _l6_htbl_t
{
    int             idtype;
    int             size;
    int             cap;
    float           factor;
    int             poolsize;
    //int             hashmask;
    int64_t         salt;
    l6_htbl_entry_t *pool;
    l6_htbl_entry_t **items;
} l6_htbl_t;

static int __l6_htbl_init(void *vpmap, int type, int cap) 
{
    l6_htbl_t **ppmap = (l6_htbl_t**) vpmap;
    (*ppmap) = (l6_htbl_t*)malloc(sizeof(struct _l6_htbl_t));

    (*ppmap)->size      = 0;
    (*ppmap)->factor    = 0.25f;
    if(cap <= 0) cap    = L6_MSG_MAP_INIT_CAP;
    (*ppmap)->cap       = cap;
    (*ppmap)->poolsize  = 0;
    (*ppmap)->pool      = NULL;
    (*ppmap)->idtype    = type;
    (*ppmap)->salt      = 0;
    
    (*ppmap)->items = (l6_htbl_entry_t**) malloc((*ppmap)->cap * sizeof(l6_htbl_entry_t*)); 
    bzero((*ppmap)->items, ((*ppmap)->cap * sizeof(l6_htbl_entry_t*)));
    return 0;
}

static l6_htbl_entry_t *__l6_htbl_entry_alloc(l6_htbl_t *ph, void *key, int keysize)
{
    l6_htbl_entry_t *entry = (l6_htbl_entry_t *)malloc(sizeof(l6_htbl_entry_t));
    entry->keysize = keysize;
    if(keysize)
    {
        entry->key.p = (char*)malloc(keysize);
        memcpy(entry->key.p, key, keysize);
    }
    else
        entry->key.i = (*(int*)key);

    entry->next = NULL;
    return entry;
    
}

static void __l6_htbl_entry_free(l6_htbl_t *ph, l6_htbl_entry_t *entry) 
{
    if(entry->keysize)
        free(entry->key.p);
    free(entry);
}

static int __l6_htbl_free(void *pmap) 
{
    l6_htbl_t **ppmap = (l6_htbl_t**) pmap;
    int i = 0;
    for(i = 0; i < (*ppmap)->cap; ++i)
    {
       l6_htbl_entry_t *entry = (*ppmap)->items[i];
       while(entry)
       {
           //free entries
           l6_htbl_entry_t *prev = entry; 
           entry = entry->next;
           __l6_htbl_entry_free((*ppmap), prev);
       }
    }
    free((*ppmap)->items);
    free((*ppmap));
    *ppmap = NULL;
    return 0;
}

static int __l6_htbl_generic_put(l6_htbl_t *ph, void *key, int keysize, void* field)
{
    //don't worry about replacing duplicate key entries, app code handles that
    uint32_t hash       = l6_htbl_hash(key, keysize, ph->cap, ph->salt);
    unsigned int index  = (hash % ph->cap);
    l6_htbl_entry_t *entry   = __l6_htbl_entry_alloc(ph, key, keysize);
    l6_htbl_entry_t *current = ph->items[index];
    entry->hash         = hash;
    entry->value        = field;
    entry->next         = current;
    ph->items[index]    = entry;
    ++(ph->size);
    return 0;
}

static int __l6_htbl_generic_get(l6_htbl_t *ph, void *key, int keysize, l6_htbl_entry_t **ret)
{ 
    uint32_t hash  = l6_htbl_hash(key, keysize, ph->cap, ph->salt);
    uint32_t index = (hash % ph->cap);
    l6_htbl_entry_t *entry = ph->items[index];
    *ret = NULL;
    while(entry)
    {
        if((keysize == 0) ? (*(int*)key == entry->key.i) :
            ((entry->hash == hash) && (keysize == entry->keysize) 
                && ((key == entry->key.p) || !memcmp(key, entry->key.p, keysize))))
        {
            *ret = entry;
            return 1;
        }
        entry = entry->next;
    }
    return 0;
}

static int __l6_htbl_generic_remove(l6_htbl_t *ph, void *key, int keysize, l6_htbl_entry_t **ret)
{ 
    uint32_t hash  = l6_htbl_hash(key, keysize, ph->cap, ph->salt);
    uint32_t index = (hash % ph->cap);
    l6_htbl_entry_t *entry = ph->items[index];
    l6_htbl_entry_t *prev = NULL;
    *ret = NULL;
    while(entry)
    {
        if((keysize == 0) ? (*(int*)key == entry->key.i) :
            ((entry->hash == hash) && (keysize == entry->keysize) 
                && ((key == entry->key.p) || !memcmp(key, entry->key.p, keysize))))
        {
            *ret = entry;
            if(prev == NULL)
                ph->items[index] = entry->next;
            else
                prev->next = entry->next;
            --(ph->size);
            return 1;
        }
        prev = entry;
        entry = entry->next;
    }
    return 0;
}

static int __l6_htbl_skey_put(void *ph, _const_char *key, void* field)
{
    int keysize = strlen(key);  //strnlen(key, L6_MSG_FIELD_NAME_MAX_LEN);  //not portable
    return __l6_htbl_generic_put((l6_htbl_t*)ph, (void*)key, keysize, field);
}

static int __l6_htbl_ikey_put(void *ph, int32_t key, void* field)
{
    return __l6_htbl_generic_put((l6_htbl_t*)ph, &key, 0, field);
}

static int __l6_htbl_skey_get(void *ph, _const_char *key, void **field)
{
    int keysize = strlen(key);
    l6_htbl_entry_t *entry = NULL;
    int ret = __l6_htbl_generic_get((l6_htbl_t*)ph, (void*)key, keysize, &entry);
    if(ret && entry)
    {
        *field = entry->value;
    }
    return ret;
}

static int __l6_htbl_ikey_get(void *ph, int32_t key, void **field)
{ 
    l6_htbl_entry_t *entry = NULL;
    int ret = __l6_htbl_generic_get((l6_htbl_t*)ph, &key, 0, &entry);
    if(ret && entry)
    {
        *field = entry->value;
    }
    return ret;
}

static int __l6_htbl_skey_remove(void *ph, _const_char *key, void **field)
{
    int keysize = strlen(key);  //(key, L6_MSG_FIELD_NAME_MAX_LEN);
    l6_htbl_entry_t *entry = NULL;
    int ret = __l6_htbl_generic_remove((l6_htbl_t*)ph, (void*)key, keysize, &entry);
    if(ret && entry)
    {
        *field = entry->value;
        __l6_htbl_entry_free((l6_htbl_t*)ph, entry);
    }
    return ret;
}

static int __l6_htbl_ikey_remove(void *ph, int32_t key, void** field)
{ 
    l6_htbl_entry_t *entry = NULL;
    int ret = __l6_htbl_generic_remove((l6_htbl_t*)ph, &key, 0, &entry);
    if(ret && entry)
    {
        *field = entry->value;
        __l6_htbl_entry_free((l6_htbl_t*)ph, entry);
    }
    return ret;
}

//Plugin-abble hash put get methods
//unsigned int l6_htbl_hash(char *data, int size, int limit, uint32_t salt)
int (*l6_htbl_init)        (void *vpmap, int opt, int start)           = __l6_htbl_init;
int (*l6_htbl_free)        (void *pmap)                                = __l6_htbl_free;
int (*l6_htbl_ikey_put)    (void *ph, int32_t fid,      void  *field)  = __l6_htbl_ikey_put;
int (*l6_htbl_ikey_get)    (void *ph, int32_t fid,      void **field)  = __l6_htbl_ikey_get;
int (*l6_htbl_ikey_remove) (void *ph, int32_t fid,      void **field)  = __l6_htbl_ikey_remove;
int (*l6_htbl_skey_put)    (void *ph, _const_char *key, void  *field)  = __l6_htbl_skey_put;
int (*l6_htbl_skey_get)    (void *ph, _const_char *key, void **field)  = __l6_htbl_skey_get;
int (*l6_htbl_skey_remove) (void *ph, _const_char *key, void **field)  = __l6_htbl_skey_remove;

//Plugin-able dynamic array methods
int   (*l6_mvec_init)       (void **v, int elem_size, int start)  = __l6_mvec_init;
int   (*l6_mvec_free)       (void *v)                             = __l6_mvec_free;
int   (*l6_mvec_resize)     (void *v, float factor)               = __l6_mvec_resize;
int   (*l6_mvec_push)       (void *v, void  *field)               = __l6_mvec_push;
int   (*l6_mvec_size)       (void *v)                             = __l6_mvec_size;
void *(*l6_mvec_get)        (void *v, int   index)                = __l6_mvec_get;
void *(*l6_mvec_remove)     (void *v, int   index)                = __l6_mvec_remove;
int   (*l6_mvec_index_of)   (void *v, void  *field)               = __l6_mvec_index_of;
int   (*l6_mvec_remove_item)(void *v, void  *field)               = __l6_mvec_remove_item;
void *(*l6_mvec_pop)        (void *v)                             = __l6_mvec_pop;


//User-space override, e.g. to provide more optimized hash put/get methods
void l6_override_hashtable_methods( int (*ovrr_htbl_init)        (void *vpmap, int opt, int start), 
                                    int (*ovrr_htbl_free)        (void *pmap),
                                    int (*ovrr_htbl_ikey_put)    (void *ph, int32_t fid,     void *field),
                                    int (*ovrr_htbl_ikey_get)    (void *ph, int32_t fid,     void **field),
                                    int (*ovrr_htbl_ikey_remove) (void *ph, int32_t fid,     void **field),
                                    int (*ovrr_htbl_skey_put)    (void *ph, _const_char *key,void *field),
                                    int (*ovrr_htbl_skey_get)    (void *ph, _const_char *key,void **field),
                                    int (*ovrr_htbl_skey_remove) (void *ph, _const_char *key,void **field))
{
    l6_htbl_init        = ovrr_htbl_init;
    l6_htbl_free        = ovrr_htbl_free;
    l6_htbl_ikey_put    = ovrr_htbl_ikey_put;
    l6_htbl_ikey_get    = ovrr_htbl_ikey_get;
    l6_htbl_ikey_remove = ovrr_htbl_ikey_remove;
    l6_htbl_skey_put    = ovrr_htbl_skey_put;
    l6_htbl_skey_get    = ovrr_htbl_skey_get;
    l6_htbl_skey_remove = ovrr_htbl_skey_remove;
}

void l6_override_vector_methods(    int   (*ovrr_mvec_init)         (void **q, int elem_size, int start),
                                    int   (*ovrr_mvec_free)         (void *q),
                                    int   (*ovrr_mvec_resize)       (void *q, float factor),
                                    int   (*ovrr_mvec_push)         (void *q, void  *field),
                                    int   (*ovrr_mvec_size)         (void *q),
                                    void *(*ovrr_mvec_get)          (void *q, int index),
                                    void *(*ovrr_mvec_remove)       (void *q, int index),
                                    int   (*ovrr_mvec_index_of)     (void *q, void *field),
                                    int   (*ovrr_mvec_remove_item)  (void *q, void *field),
                                    void *(*ovrr_mvec_pop)          (void *q))
{
    l6_mvec_init        = ovrr_mvec_init;
    l6_mvec_free        = ovrr_mvec_free;
    l6_mvec_resize      = ovrr_mvec_resize;
    l6_mvec_push        = ovrr_mvec_push;
    l6_mvec_size        = ovrr_mvec_size;
    l6_mvec_get         = ovrr_mvec_get;
    l6_mvec_remove      = ovrr_mvec_remove;
    l6_mvec_index_of    = ovrr_mvec_index_of;
    l6_mvec_remove_item = ovrr_mvec_remove_item;
    l6_mvec_pop         = ovrr_mvec_pop;
}


//Internal structs
typedef union _l6_data_type
{
    /* primitives */
    char            c;
    int16_t         s;    //short
    int32_t         i;    //int
    int64_t         l;    //long long int
    
    float           f;
    double          d;

    /* arrays */
    char            *ptr;
    int16_t         *sa;    //short
    int32_t         *ia;    //int
    int64_t         *la;    //long long int
    float           *fa;
    double          *da;

    unsigned char   uc;
    uint16_t        us;     //short
    uint32_t        ui;     //int
    uint64_t        ul;     //long long int
    unsigned char   *uca;
    uint16_t        *usa;   //unsigned short
    uint32_t        *uia;   //unsigned int
    uint64_t        *ula;   //unsigned long long int


    /* sub-message */
    void            *msg;

    /* custom data */
    void            *data;
} l6_data_type;

typedef struct _l6msg_field
{
    /* metadata */
                
    short       fid;
    char        *name;
    int         namelen;  //limited to 256
    int         namemem;
    int         type;
    int         elem_size;   //0 - 8 bytes
    short       count;    //limited to 2^16 - 1
    short       mdlength;
    int         index;
    int         total_size;

    int         offset_data;
    int         offset_metadata;
    
    int         serialized;
    int         internally_allocated;

    /* data */
    l6_data_type data;
    void         *buffer;
    int          datalen;
    int          buflen;
} l6msg_field;


//Field extension hook methods
typedef struct _l6msg_field_type_handler
{
    int element_size_bytes;
    int (*set_data   )  (l6msg_field *field, void *data,   int copy);
    int (*get_data   )  (l6msg_field *field, void **data,  int offset, int count);
    int (*serialize  )  (l6msg_field *field, char *buffer, int bswap,  int copy);
    int (*deserialize)  (l6msg_field *field, char *buffer, int bswap,  int copy);
} l6msg_field_type_handler;

typedef struct _l6msg_struct
{
    //* flags */
    int          has_subject;
    int          has_metadata;

    /* serializing / deserializing modes */
    int          deep_copy;
    int          auto_byte_mode;
    int          deserialize_lazily;
    //int        hasFieldnames;

    /* metadata */
    unsigned char code;
    const char   *subject;        //un-supported
    unsigned int total_length;
    unsigned int metadata_length;
    int          data_length;
    int          error_code;
    char         *debug;
    unsigned int idgen;

    /* serializing / deserializing modes */
    int           internally_allocated;
    int           do_byte_swap;
    int           is_locked;
    int           chk_submsg_cyc;
    int           offset_md;
    int           offset_d;
    int           left;
    int           done;
    int           itr;
    unsigned int  nfields;
    int           is_partial;
    char          *tmpbuffer;
    int           tmpbuffermem;
    int           tmpbufferage;

    /* templates (not yet implemented) */
    int16_t       template_id;
    
    l6msg_field_type_handler    *ext_handler;

    /* resources */
    int           htbl_opt;
    int           poolsize;
    void          *qpool;

    /* data */
    void          *qfields;
    
    /* sub msgs */
    /* data */
    void          *qsub_msgs;    

    void          *htbl_fld_name;
    void          *htbl_fld_id;
} l6msg_struct;


#define debug(X) //printf(X);fflush(stdout);

void dbgprint(char *buf, int len)
{
    int i;
    printf("\n dbug:");
    if(len < 0)
    {
        return;
    }
    for(i =0; i< len; i++)
    {
        //if(isalpha(*(buf+i)))
        //    printf("%c,",*(buf+i));
        //else
            printf("%.2x ",(unsigned char)*(buf+i));
    }
}

static void debug_field(l6msg_field *field)
{
    printf("\n\t fid=%d name=[%s]: type='%s' size=[%d*%d=%d + %d]=%d data=[%d]", field->fid, (field->namelen ? field->name : ""),
        l6_datatype(field->type), field->elem_size, field->count, field->datalen, field->mdlength, field->total_size, *((int*)&(field->data))); fflush(stdout);
}


static int add_field_data_in_msg(l6msg_struct *msg, int type, void *data, int length, int count, int copy);

static int set_field_data_in_msg_ret(l6msg_struct *msg, l6msg_field **field, 
            _const_char *key, int namelen, int id, int index, int type, 
            void *data, int length, int count, int copy);

static int find_get_ret_field_data_in_msg(l6msg_struct *msg, _const_char *key, int id, int index, int type, void **data,
            int length, int offset, int count, int *fieldcount);

static int get_field_data_in_msg_ret(l6msg_struct *msg, l6msg_field *field, int type, void **data, 
            int length, int offset, int count, int *fieldcount);
static int remove_field_in_msg(l6msg_struct *msg, l6msg_field *field, int index);

static int get_field_at_index(l6msg_struct *msg, int index, l6msg_field **field);

static int get_field_by_id(l6msg_struct *msg, int32_t fid, l6msg_field **field);

static int get_field_named(l6msg_struct *msg, _const_char *key, l6msg_field **field);

static int l6msg_deserialize_opt(l6msg_struct* msg, char *buffer, int length, int *left, int copy);

//static int l6msg_get_field(l6msg *msg, int id, l6msg_field *field);

//static int l6msg_get_field_named(l6msg *msg, char *fieldname, l6msg_field *field);
static l6msg_field *l6msg_alloc_field();
static int          l6msg_free_field(l6msg_field *field);
static int          l6msg_reset_field(l6msg_field *field);

static int __l6msg_reset(l6msg_struct *msg, int reset_err);

static int l6msg_check_cyclic_submsg(l6msg_struct *msg);


static int l6msg_serialize_field_metadata(l6msg_struct *msg, l6msg_field *field, char *buffer);
static int l6msg_serialize_field_data(l6msg_struct *msg, l6msg_field *field, char *buffer, int *left);
static int l6msg_deserialize_field_metadata(l6msg_struct *msg, l6msg_field **field, char *buffer, int length);
static int l6msg_deserialize_field_data(l6msg_struct *msg, l6msg_field *field, char *buffer, int length, int *left, int copy);

/************** l6msg implementation ***************/
static int alloc_field_mem(l6msg_field *field) 
{
    if(field->datalen > field->buflen)
    {
        field->buflen   = field->datalen;
        field->buffer   = realloc(field->buffer, field->buflen);
        field->data.ptr = (char*)(field->buffer);
    }
    return 0;
}

//field handlers
//no-ops for undefined fields to avoid NULL-dereferencing errors
static int set_noop(l6msg_field *field, void *data, int copy) { return 0; }
static int get_noop(l6msg_field *field, void **data, int offset, int copy) { return 0; }
static int srlz_dsrlz_noop(l6msg_field *field, char *buffer, int bswap,  int copy) { return 0; }

//uint8
static int set_uint8(l6msg_field *field, void *data, int copy)
{
    field->data.uc = *(unsigned char*)data;
    return 0;
}

static int get_uint8(l6msg_field *field, void **data, int offset, int count)
{
    *(unsigned char*)(*data) = field->data.uc; //memcpy(*data, &field->data, field->elem_size);
    return 0;
}

static int serialize_uint8(l6msg_field *field, char *buffer, int bswap,  int copy)
{
    ((uint8_t*)buffer)[0] = field->data.uc;
    return 0;
}

static int deserialize_uint8(l6msg_field *field, char *buffer, int bswap,  int copy)
{
    field->data.uc = ((uint8_t*)buffer)[0];
    return 0;
}

//int8
static int set_int8(l6msg_field *field, void *data, int copy)
{
    field->data.c = *(char*)data;
    return 0;
}

static int get_int8(l6msg_field *field, void **data, int offset, int count)
{
    *(char*)(*data) = field->data.c; //memcpy(*data, &field->data, field->elem_size);
    return 0;
}

static int serialize_int8(l6msg_field *field, char *buffer, int bswap,  int copy)
{
    ((int8_t*)buffer)[0] = field->data.c;
    return 0;
}

static int deserialize_int8(l6msg_field *field, char *buffer, int bswap,  int copy)
{
    field->data.c = ((int8_t*)buffer)[0];
    return 0;
}

//Type-dependent operations defining macros
//Primitives
#define DEF_PRIMITIVE_SET(TYPENAME, DATATYPE, UNIONMEMBER)                          \
static int set_##TYPENAME(l6msg_field *field, void *data, int copy)                 \
{                                                                                   \
    field->data.UNIONMEMBER = *(DATATYPE *)data;                                    \
    return 0;                                                                       \
}

#define DEF_PRIMITIVE_GET(TYPENAME, DATATYPE, UNIONMEMBER)                          \
static int get_##TYPENAME(l6msg_field *field, void **data, int offset, int count)   \
{                                                                                   \
    *(DATATYPE *)(*data) = field->data.UNIONMEMBER;                                 \
    return 0;                                                                       \
}

#define DEF_PRIMITIVE_SERIALIZE(TYPENAME, DATATYPE, UNIONMEMBER, BYTESWAP, BSWAPDTYPE) \
static int serialize_##TYPENAME(l6msg_field *field, char *buffer, int bswap,  int copy)\
{                                                                                   \
    *(DATATYPE *)(buffer) = field->data.UNIONMEMBER;                                \
    if(bswap) BYTESWAP((BSWAPDTYPE *)(buffer));                                     \
    return 0;                                                                       \
}

#define DEF_PRIMITIVE_DESERIALIZE(TYPENAME, DATATYPE, UNIONMEMBER, BYTESWAP, BSWAPDTYPE)    \
static int deserialize_##TYPENAME(l6msg_field *field, char *buffer, int bswap,  int copy)   \
{                                                                                    \
    field->data.UNIONMEMBER = *(DATATYPE *)(buffer);                                 \
    if(bswap) BYTESWAP((BSWAPDTYPE *)&(field->data.UNIONMEMBER));                    \
    return 0;                                                                        \
}

DEF_PRIMITIVE_SET(uint16, uint16_t, us)
DEF_PRIMITIVE_GET(uint16, uint16_t, us)
DEF_PRIMITIVE_SERIALIZE(uint16, uint16_t, us, byte_swap_16, uint16_t)
DEF_PRIMITIVE_DESERIALIZE(uint16, uint16_t, us, byte_swap_16, uint16_t)

DEF_PRIMITIVE_SET(int16, int16_t, s)
DEF_PRIMITIVE_GET(int16, int16_t, s)
DEF_PRIMITIVE_SERIALIZE(int16, int16_t, s, byte_swap_16, uint16_t)
DEF_PRIMITIVE_DESERIALIZE(int16, int16_t, s, byte_swap_16, uint16_t)

DEF_PRIMITIVE_SET(uint32, uint32_t, ui)
DEF_PRIMITIVE_GET(uint32, uint32_t, ui)
DEF_PRIMITIVE_SERIALIZE(uint32, uint32_t, ui, byte_swap_32, uint32_t)
DEF_PRIMITIVE_DESERIALIZE(uint32, uint32_t, ui, byte_swap_32, uint32_t)

DEF_PRIMITIVE_SET(int32, int32_t, i)
DEF_PRIMITIVE_GET(int32, int32_t, i)
DEF_PRIMITIVE_SERIALIZE(int32, int32_t, i, byte_swap_32, uint32_t)
DEF_PRIMITIVE_DESERIALIZE(int32, int32_t, i, byte_swap_32, uint32_t)

DEF_PRIMITIVE_SET(uint64, uint64_t, ul)
DEF_PRIMITIVE_GET(uint64, uint64_t, ul)
DEF_PRIMITIVE_SERIALIZE(uint64, uint64_t, ul, byte_swap_64, uint64_t)
DEF_PRIMITIVE_DESERIALIZE(uint64, uint64_t, ul, byte_swap_64, uint64_t)

DEF_PRIMITIVE_SET(int64, int64_t, l)
DEF_PRIMITIVE_GET(int64, int64_t, l)
DEF_PRIMITIVE_SERIALIZE(int64, int64_t, l, byte_swap_64, uint64_t)
DEF_PRIMITIVE_DESERIALIZE(int64, int64_t, l, byte_swap_64, uint64_t)

DEF_PRIMITIVE_SET(float, float, f)
DEF_PRIMITIVE_GET(float, float, f)
DEF_PRIMITIVE_SERIALIZE(float, float, f, byte_swap_32, uint32_t)
DEF_PRIMITIVE_DESERIALIZE(float, float, f, byte_swap_32, uint32_t)

DEF_PRIMITIVE_SET(double, double, d)
DEF_PRIMITIVE_GET(double, double, d)
DEF_PRIMITIVE_SERIALIZE(double, double, d, byte_swap_64, uint64_t)
DEF_PRIMITIVE_DESERIALIZE(double, double, d, byte_swap_64, uint64_t)


//Arrays
static int get_array(l6msg_field *field, void **data, int offset, int count)
{
    if(count > 1)
    {
        if((offset + count) > field->count)
            count = field->count - offset;
        memcpy(*data, (field->data.ptr + (offset * field->elem_size)), count * field->elem_size);
    }
    else
        *data = field->data.ptr;
    return 0;
}

#define DEF_ARRAY_SET_DATA(TYPENAME, DATASIZE)                                          \
static int set_##TYPENAME##s(l6msg_field *field, void *data, int copy)                  \
{                                                                                       \
    if(copy)                                                                            \
    {                                                                                   \
        alloc_field_mem(field);                                                         \
        memcpy(field->data.ptr, data, field->count * DATASIZE);                         \
    }                                                                                   \
    else                                                                                \
        field->data.ptr = (char *)data;                                                 \
    return 0;                                                                           \
}

#define DEF_ARRAY_GET_DATA(TYPENAME, DATASIZE)                                          \
static int get_##TYPENAME##s(l6msg_field *field, void **data, int offset, int count)    \
{                                                                                       \
    if(count > 1)                                                                       \
    {                                                                                   \
        if((offset + count) > field->count)                                             \
            count = field->count - offset;                                              \
        memcpy(*data, (field->data.ptr + (offset * DATASIZE)), (count * DATASIZE));     \
    }                                                                                   \
    else                                                                                \
        *data = field->data.ptr;                                                        \
    return 0;                                                                           \
}

#define DEF_ARRAY_SERIALIZE(TYPENAME, DATATYPE, UNIONMEMBER, BYTESWAP, BSWAPDTYPE)          \
static int serialize_##TYPENAME##s(l6msg_field *field, char *buffer, int bswap,  int copy)  \
{                                                                                       \
    memcpy(buffer, field->data.ptr, field->count * sizeof(DATATYPE));                   \
    DATATYPE *data_a = (DATATYPE *) (buffer);                                           \
    int j = 0;                                                                          \
    if(bswap) while(j < field->count) { BYTESWAP((BSWAPDTYPE*) (data_a + j++)); }       \
    return 0;                                                                           \
}

#define DEF_ARRAY_DESERIALIZE(TYPENAME, DATATYPE, UNIONMEMBER, BYTESWAP, BSWAPDTYPE)    \
static int deserialize_##TYPENAME##s(l6msg_field *field, char *buffer, int bswap,  int copy) \
{                                                                                       \
    if(copy)                                                                            \
    {                                                                                   \
        alloc_field_mem(field);                                                         \
        memcpy(field->data.UNIONMEMBER, buffer, field->count * sizeof(DATATYPE));       \
    }                                                                                   \
    else                                                                                \
        field->data.UNIONMEMBER = (DATATYPE *)buffer;                                   \
    int j = 0;                                                                          \
    if(bswap) while(j < field->count) { BYTESWAP((BSWAPDTYPE*)(field->data.UNIONMEMBER + j++));} \
    return 0;                                                                           \
}

DEF_ARRAY_SET_DATA (8bit, 1)
DEF_ARRAY_GET_DATA (8bit, 1)
static int serialize_8bits(l6msg_field *field, char *buffer, int bswap,  int copy)
{ 
    memcpy(buffer, field->data.ptr, field->count); 
    return 0; 
}

static int deserialize_8bits(l6msg_field *field, char *buffer, int bswap,  int copy)
{ 
    if(copy)
    {
        alloc_field_mem(field);
        memcpy(field->data.ptr, buffer, field->count);
    }
    else
        field->data.ptr = buffer;
    return 0; 
}

DEF_ARRAY_SET_DATA      (16bit, 2)
DEF_ARRAY_GET_DATA      (16bit, 2)
DEF_ARRAY_SERIALIZE     (16bit, uint16_t, usa, byte_swap_16, uint16_t)
DEF_ARRAY_DESERIALIZE   (16bit, uint16_t, usa, byte_swap_16, uint16_t)

DEF_ARRAY_SET_DATA      (32bit, 4)
DEF_ARRAY_GET_DATA      (32bit, 4)
DEF_ARRAY_SERIALIZE     (32bit, uint32_t, uia, byte_swap_32, uint32_t)
DEF_ARRAY_DESERIALIZE   (32bit, uint32_t, uia, byte_swap_32, uint32_t)

DEF_ARRAY_SET_DATA      (64bit, 8)
DEF_ARRAY_GET_DATA      (64bit, 8)
DEF_ARRAY_SERIALIZE     (64bit, uint64_t, ula, byte_swap_64, uint64_t)
DEF_ARRAY_DESERIALIZE   (64bit, uint64_t, ula, byte_swap_64, uint64_t)

DEF_ARRAY_SET_DATA      (float, 4)
DEF_ARRAY_GET_DATA      (float, 4)
DEF_ARRAY_SERIALIZE     (float, float, fa, byte_swap_32, uint32_t)
DEF_ARRAY_DESERIALIZE   (float, float, fa, byte_swap_32, uint32_t)

DEF_ARRAY_SET_DATA      (double, 8)
DEF_ARRAY_GET_DATA      (double, 8)
DEF_ARRAY_SERIALIZE     (double, double, da, byte_swap_64, uint64_t)
DEF_ARRAY_DESERIALIZE   (double, double, da, byte_swap_64, uint64_t)

//special case for sub msgs

static int set_l6msg(l6msg_field *field, void *data, int copy)
{
    int ret = 0;
    if(copy)
    {
        l6msg sub_msg = (l6msg)data;
        l6msg dup_msg;
        l6msg_init(&dup_msg);
        ret = l6msg_dup(&sub_msg, &dup_msg);
        if(ret >= 0)
        {
            field->data.msg = dup_msg;
            field->internally_allocated = 1;
        }
        else
            l6msg_free(&dup_msg);
    }
    else
        field->data.msg = (l6msg_struct*)data;
    //field->datalen = field->count = l6msg_size((l6msg)(field->data.msg));
    return ret;
}

static int get_l6msg(l6msg_field *field, void **data, int offset, int count)
{
    int ret = 0;
    l6msg *ret_msg = (l6msg*)(*data);
    if(count > 0)
    {
        l6msg sub_msg = (l6msg)(field->data.msg);
        ret = l6msg_dup(&sub_msg, ret_msg);
    }
    else
       *ret_msg = field->data.msg;
    return ret;
}

static int serialize_l6msg(l6msg_field *field, char *buffer, int bswap, int copy)
{
    int left = 0;
    l6msg sub_msg =(l6msg)(field->data.msg);
    l6msg_set_auto_byte_order_mode(&sub_msg, bswap);
    int ret = l6msg_serialize(&sub_msg, buffer, field->datalen, &left);
    if(ret >= 0) field->serialized = 1;
    return ret;
}

static int deserialize_l6msg(l6msg_field *field, char *buffer, int bswap, int copy)
{
    if(field->serialized)
    {
        l6msg sub_msg;
        l6msg_init(&sub_msg);
        l6msg_set_auto_byte_order_mode(&sub_msg, bswap);
    
        field->data.msg = sub_msg;
        
        //deserialize shallow
        int left = 0;
        l6msg_set_deep_copy_default(&sub_msg, 0);
        field->internally_allocated = 1;
        field->serialized = 0;
        return l6msg_deserialize(&sub_msg, buffer, field->count, &left);
    }
    return 0;
}

static int get_l6msgz(l6msg_field *field, void **data, int offset, int copy)
{
    l6msg *sub_msg = (l6msg*)(*data);
    //l6msg_set_auto_byte_order_mode(*sub_msg, bswap);   
    int left = 0;
    return l6msg_deserialize(sub_msg, field->data.ptr, field->datalen, &left);
}

static int serialize_l6msgz(l6msg_field *field, char *buffer, int bswap,  int copy)
{
    memcpy(buffer, field->data.ptr, field->datalen);
    //field->type = L6_DATATYPE_L6MSG_SERIALIZED;
    return 0;
}

//field handlers
static l6msg_field_type_handler l6_field_type_handlers[30] = {
    { 0, set_noop,      get_noop,    srlz_dsrlz_noop,   srlz_dsrlz_noop     },    // 0
    { 1, set_uint8,     get_uint8,   serialize_uint8,   deserialize_uint8   },
    { 1, set_int8,      get_int8,    serialize_int8,    deserialize_int8    },
    { 2, set_uint16,    get_uint16,  serialize_uint16,  deserialize_uint16  },
    { 2, set_int16,     get_int16,   serialize_int16,   deserialize_int16   },
    { 4, set_uint32,    get_uint32,  serialize_uint32,  deserialize_uint32  },
    { 4, set_int32,     get_int32,   serialize_int32,   deserialize_int32   },
    { 8, set_uint64,    get_uint64,  serialize_uint64,  deserialize_uint64  },
    { 8, set_int64,     get_int64,   serialize_int64,   deserialize_int64   },
    { 4, set_float,     get_float,   serialize_float,   deserialize_float   },
    { 8, set_double,    get_double,  serialize_double,  deserialize_double  }, // 10
    { 0, set_noop,      get_noop,    srlz_dsrlz_noop,   srlz_dsrlz_noop     },
    { 0, set_noop,      get_noop,    srlz_dsrlz_noop,   srlz_dsrlz_noop     },
    { 0, set_noop,      get_noop,    srlz_dsrlz_noop,   srlz_dsrlz_noop     },
    { 0, set_noop,      get_noop,    srlz_dsrlz_noop,   srlz_dsrlz_noop     },
    { 0, set_noop,      get_noop,    srlz_dsrlz_noop,   srlz_dsrlz_noop     },
    { 1, set_8bits,     get_array,   serialize_8bits,   deserialize_8bits   }, // 16    
    { 1, set_8bits,     get_8bits,   serialize_8bits,   deserialize_8bits   },
    { 1, set_8bits,     get_8bits,   serialize_8bits,   deserialize_8bits   },
    { 2, set_16bits,    get_16bits,  serialize_16bits,  deserialize_16bits  }, // 19
    { 2, set_16bits,    get_16bits,  serialize_16bits,  deserialize_16bits  }, 
    { 4, set_32bits,    get_32bits,  serialize_32bits,  deserialize_32bits  },
    { 4, set_32bits,    get_32bits,  serialize_32bits,  deserialize_32bits  },
    { 8, set_64bits,    get_64bits,  serialize_64bits,  deserialize_64bits  },
    { 8, set_64bits,    get_64bits,  serialize_64bits,  deserialize_64bits  },
    { 4, set_floats, 	get_floats,  serialize_floats,  deserialize_floats  }, // 25
    { 8, set_doubles,	get_doubles, serialize_doubles, deserialize_doubles }, // 26
    { 1, set_l6msg,  	get_l6msg,   serialize_l6msg,   deserialize_l6msg   }, // 27
    { 1, set_l6msg,  	get_l6msgz,  serialize_l6msgz,  deserialize_l6msg   },
    { 0, set_noop,      get_noop,    srlz_dsrlz_noop,   srlz_dsrlz_noop     },
};

static int add_field_data_in_msg(l6msg_struct *msg, int type, 
                                void *data, int length, int count, int copy)
{
    l6msg_field *field = NULL;
    return set_field_data_in_msg_ret(msg, &field, NULL, 0, -1, -1, 
                                type, data, length, count, copy);
}


#define DEBUG_BUF_SIZE 256
static void sprintf_debug_info(l6msg_struct *msg, int ln, const char *fmt, ...) 
{
    if(!msg->debug) 
        msg->debug = malloc(DEBUG_BUF_SIZE);
    char *buf = msg->debug;
    int ret = sprintf(buf, "%d-%s", msg->error_code, 
                        l6msg_get_error_str((void**)&msg));
    if(fmt)
    {
        ret += sprintf(buf + ret, " (");
        va_list args;
        va_start(args, fmt);
        ret += vsnprintf(buf + ret, (DEBUG_BUF_SIZE - ret - 32), fmt, args);
        if(ret >= (DEBUG_BUF_SIZE - 32))
            ret += snprintf(buf + ret, 3, "...");
        va_end(args);
        ret += sprintf(buf + ret, ")");
    }
    sprintf(buf + ret, " @ line %d in %s.", ln, __FILE__);
    return;
}

static int find_set_ret_field_data_in_msg(l6msg_struct *msg, const char *name, 
                int namelen, int32_t fid, int index, int type, void *data, 
                    int length, int count, int copy, l6msg_field **retfield)
{
    l6msg_field *field = NULL;
    msg->error_code = 0;

    if((index >= 0) && (index < l6_mvec_size(msg->qfields)))
    {
        field = (l6msg_field*)l6_mvec_get(msg->qfields, index);
        if(DISALLOW_TRANSPARENT_OVERWRITE && field)
        {
            msg->error_code = L6_ERR_FIELD_EXISTS;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "index=%d", index);
            return -1;    //field not found
        }
    }
    else if(name)
    {
        if(namelen <= 0) 
            namelen = strlen(name);  //(name, (L6_MSG_FIELD_NAME_MAX_LEN + 2));
        //cannot be longer than L6_MSG_FIELD_NAME_MAX_LEN
        if(namelen > L6_MSG_FIELD_NAME_MAX_LEN)
        {
            msg->error_code = L6_ERR_FIELD_NAME_TOO_LONG;
            *retfield = NULL;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "namelen=%d", namelen);
            return -1;
        }

        get_field_named(msg, name, &field);
        if(DISALLOW_TRANSPARENT_OVERWRITE && field)
        {
            msg->error_code = L6_ERR_FIELD_EXISTS;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "name=%s", name);
            return -1;    //field not found
        }
    }
    else if(fid >= 0)
    {
        get_field_by_id(msg, fid, &field);
        if(DISALLOW_TRANSPARENT_OVERWRITE && field)
        {
            msg->error_code = L6_ERR_FIELD_EXISTS;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "id=%d", fid);
            return -1;    //field not found
        }
    }
    
    *retfield = field;
    msg->error_code = 0;    //reset because it would be set by the get_field_ methods 
    return set_field_data_in_msg_ret(msg, retfield, name, namelen, fid, index, 
                                    type, data, length, count, copy);
}


static int set_field_data_in_msg_at_index(l6msg_struct *msg, int index, 
                int type, void *data, int length, int count, int copy)
{
    l6msg_field *field = NULL;
    msg->error_code = 0;
    if((index >= 0) && (index < l6_mvec_size(msg->qfields)))
    {
        field = (l6msg_field*)l6_mvec_get(msg->qfields, index);
    }

    if(DISALLOW_TRANSPARENT_OVERWRITE && field)
    {
        msg->error_code = L6_ERR_FIELD_NOT_FOUND;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "index=%d",index);
        return -1;    //field not found
    }

    return set_field_data_in_msg_ret(msg, &field, NULL, 0, -1, 
                                    index, type, data, length, count, copy);
}


static int set_field_data_in_msg_named(l6msg_struct *msg, const char *name, /* int namelen,*/
                int type, void *data, int length, int count, int copy)
{
    l6msg_field *field = NULL;
    int namelen = 0;

    if(name)
    {
        namelen = strlen(name);  //(name, (L6_MSG_FIELD_NAME_MAX_LEN + 2));

        //cannot be longer than L6_MSG_FIELD_NAME_MAX_LEN
        if(namelen > L6_MSG_FIELD_NAME_MAX_LEN)
        {
            msg->error_code = L6_ERR_FIELD_NAME_TOO_LONG;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "namelen=%d", namelen);
            return -1;
        }

        get_field_named(msg, name, &field);
        if(DISALLOW_TRANSPARENT_OVERWRITE && field == NULL)
        {
            msg->error_code = L6_ERR_FIELD_NOT_FOUND;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "name=%s", name);
            return -1;    //field not found
        }
        
    }
    else
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "name=NULL");


    return set_field_data_in_msg_ret(msg, &field, name, namelen, -1, -1, 
                                    type, data, length, count, copy);
}
    
static int set_field_data_in_msg_with_id(l6msg_struct *msg, 
            int32_t fid, int type, void *data, int length, int count, int copy)
{
    l6msg_field *field = NULL;

    if(fid >= 0) get_field_by_id(msg, fid, &field);

    if(DISALLOW_TRANSPARENT_OVERWRITE && field == NULL)
    {
        msg->error_code = L6_ERR_FIELD_NOT_FOUND;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "id=%d", fid);
        return -1;    //field not found
    }

    return set_field_data_in_msg_ret(msg, &field, NULL, 0, fid, -1, 
                                    type, data, length, count, copy);
}

static l6msg_field *l6msg_alloc_field()
{
    l6msg_field *field = (l6msg_field*)malloc(sizeof(l6msg_field));
    field->name      = NULL;
    field->namemem   = 0;
    field->buffer    = NULL;
    field->buflen    = 0;
    field->fid       = -1;
    field->mdlength  = 1;   //basic length: type = byte
    field->elem_size = 0;
    field->namelen   = 0;

    field->serialized = 0;
    field->internally_allocated = 0;
    return field;
}


static int l6msg_reset_field(l6msg_field *field)
{
    field->namelen   = 0;
    field->datalen   = 0;
    field->count     = 0;
    field->elem_size = 0;
    field->serialized = 0;
    field->internally_allocated = 0;
    field->fid = -1;
    return 0;
}

static int l6msg_free_field(l6msg_field *field)
{
    if(field->name)   free(field->name);
    if(field->buffer) free(field->buffer);
    free(field);
    return 0;
}

//assumes ftype is not malicious, else could dereference invalid memory
static l6msg_field_type_handler *get_field_type_handler(l6msg_struct *msg, int ftype)
{
    l6msg_field_type_handler *handler = NULL;
    if(ftype & L6_FLAG_FIELD_IS_EXTENSION)        //case L6_DATATYPE_CUSTOM:
        handler = msg->ext_handler;
    else
        handler = l6_field_type_handlers + ftype;

    if((handler == NULL) || (handler->element_size_bytes < 1))
    {
        msg->error_code = L6_ERR_UNHANDLED_FIELD_TYPE;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "type=%d", ftype);
        return NULL;
    }
    return handler;
}

static int set_field_data_in_msg_ret(l6msg_struct *msg, l6msg_field **pfield,
                const char *name, int namelen, int32_t fid, int index, 
                int type, void *data, int length, int count, int copy){
    int ret = 0;
    msg->error_code = 0;
    if(L6_CHECK_MSG_LOCKS && (msg->is_locked))
    {
        msg->error_code = L6_ERR_MSG_LOCKED;
        *pfield = NULL;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
        return -1;
    }

    if(count > L6_FIELD_MAX_COUNT)
    {
        msg->error_code = L6_ERR_FIELD_MAX_COUNT_EXCEEDED;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "count=%d", count);
        return -1;
    }

    l6msg_field *field = *pfield;
    if(field) 
    {
        if(type != field->type) /* || (count != field->count)) */
        {
            msg->error_code = L6_ERR_FIELD_TYPE_INCOMPATIBLE;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "req=%d, found=%d", type, field->type);
            return -1;
        }
        if(count != field->count)
        {
            if(TRACK_MSG_SIZE_CONTINUALLY)
            {
                msg->total_length    -= field->total_size;
                msg->data_length     -= field->datalen;
            }
            field->count         =  count;
            field->datalen       =  field->elem_size * field->count;
            field->total_size    =  field->mdlength  + field->datalen;
            if(TRACK_MSG_SIZE_CONTINUALLY)
            {
                msg->total_length    += field->total_size;
                msg->data_length     += field->datalen;
            }
        }
    }
    else
    {
        if(l6_mvec_size(msg->qfields) >= L6_MAX_NUM_FIELDS)
        {
            msg->error_code = L6_ERR_MAX_NUM_FIELDS_EXCEEDED;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "nfields=%d", 
                                                    l6_mvec_size(msg->qfields));
            return -1;   
        }
        if(l6_mvec_size(msg->qpool))
            field = (l6msg_field*)l6_mvec_pop(msg->qpool);
        else
            field = l6msg_alloc_field();
            
        field->mdlength = 1;

        if (fid >= 0) 
        {
            field->fid = (short)fid;
            msg->idgen += (field->fid + 1);
            field->mdlength += 2;
            if(!msg->htbl_fld_id)
            {
                l6_htbl_init(&(msg->htbl_fld_id), 
                    L6_MSG_ID_MAP_TYPE_INT, L6_MSG_ID_MAP_INIT_CAP);
            }
            l6_htbl_ikey_put(msg->htbl_fld_id, field->fid, field);
        }
    
    
        if(name != NULL)
        {
            field->namelen = namelen + 1;
            if(field->namemem < field->namelen)
            {
                field->namemem = field->namelen;
                field->name = (char*)realloc(field->name, field->namemem);
            }
            field->mdlength += (field->namelen + 1);    //strlen + 1 null byte + 1 byte length)
            strncpy(field->name, name, namelen);
            field->name[namelen] = '\0';

            if(!msg->htbl_fld_name) 
            {
                l6_htbl_init(&(msg->htbl_fld_name), 
                    L6_MSG_ID_MAP_TYPE_STR, L6_MSG_NAME_MAP_INIT_CAP);
            }
            l6_htbl_skey_put(msg->htbl_fld_name, field->name, field);
        }

        field->type = type;

        field->index = l6_mvec_size(msg->qfields);
        l6_mvec_push(msg->qfields, field);
    
        if(field->type == L6_DATATYPE_L6MSG) /* || (field->type == L6_DATATYPE_L6MSG_SERIALIZED)) */
        {
            if(msg->qsub_msgs == NULL)
                l6_mvec_init(&(msg->qsub_msgs), sizeof(l6msg_field), 2);
            l6_mvec_push(msg->qsub_msgs, field);
        }
    
        if(field->type & L6_FLAG_FIELD_IS_ARRAY)
        {
            field->mdlength += sizeof(int16_t);        //size of count of array elements = short
        }
        field->elem_size     =  length;
        field->count         =  count;
        field->datalen       =  field->elem_size * field->count;
        field->total_size    =  field->mdlength  + field->datalen;

        //TRACK_MSG_SIZE_CONTINUALLY does not apply here because this is also used during deserialization
        msg->total_length    += field->total_size;
        msg->metadata_length += field->mdlength;
        msg->data_length     += field->datalen;

        *pfield = field;
    }

    if(data)
    {
        l6msg_field_type_handler *handler = get_field_type_handler(msg, field->type);
        if(handler)
            ret = handler->set_data(field, data, copy);
        else
            ret = -1;
    }
    if(ret != 0) 
    {
        //l6msg_free_field(field);
        *pfield = NULL;
    }
    //debug_field(field);
    return ret;
}

static int find_get_ret_field_data_in_msg(l6msg_struct *msg, const char *name, int32_t fid, 
    int index, int type, void **data, int length, int offset, int count, int *fieldcount)
{
    l6msg_field *field = NULL;
    msg->error_code = 0;

    if((index >= 0) && (index < l6_mvec_size(msg->qfields)))
    {
        field = (l6msg_field*)l6_mvec_get(msg->qfields, index);
        if(field == NULL)
        {
            msg->error_code = L6_ERR_FIELD_NOT_FOUND;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "index=%d", index);
            return -1;
        }
    }
    else if(name)
    {
        get_field_named(msg, name, &field);
        if(field == NULL)
        {
            msg->error_code = L6_ERR_FIELD_NOT_FOUND;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "name=%s", name);
            return -1;
        }        
    }
    else if(fid >= 0)
    {
        get_field_by_id(msg, fid, &field);
        if(field == NULL)
        {
            msg->error_code = L6_ERR_FIELD_NOT_FOUND;
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "id=%d", fid);
            return -1;
        }
    }

    return get_field_data_in_msg_ret(msg, field, type, data, length, offset, count, fieldcount);
}

static int get_field_data_in_msg_at_index(l6msg_struct *msg, int index, 
    int type, void **data, int length, int offset, int count, int *fieldcount)
{
    l6msg_field *field = NULL;
    msg->error_code = 0;

    if((index >= 0) && (index < l6_mvec_size(msg->qfields)))
    {
        field = (l6msg_field*)l6_mvec_get(msg->qfields, index);
    }
    if(field==NULL)
    {
        msg->error_code = L6_ERR_FIELD_NOT_FOUND;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "index=%d", index);
        return -1;    //field not found
    }
    return get_field_data_in_msg_ret(msg, field, type, data, length, offset, count, fieldcount);
}
    
static int get_field_data_in_msg_named(l6msg_struct *msg, const char *name, 
        int type, void **data, int length, int offset, int count, int *fieldcount)
{
    l6msg_field *field = NULL;
    msg->error_code = 0;
    
    if(name) 
        get_field_named(msg, name, &field);

    if(field==NULL)
    {
        msg->error_code = L6_ERR_FIELD_NOT_FOUND;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "name=%s", (name ? name : "NULL"));
        return -1;    //field not found
    }
    return get_field_data_in_msg_ret(msg, field, type, data, length, offset, count, fieldcount);
}

static int get_field_data_in_msg_by_id(l6msg_struct *msg, int32_t fid, 
    int type, void **data, int length, int offset, int count, int *fieldcount)
{
    l6msg_field *field = NULL;
    msg->error_code = 0;
    
    if(fid >= 0)
        get_field_by_id(msg, fid, &field);

    if(field==NULL)
    {
        msg->error_code = L6_ERR_FIELD_NOT_FOUND;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "id=%d", fid);
        return -1;    //field not found
    }
    return get_field_data_in_msg_ret(msg, field, type, data, length, offset, count, fieldcount);
}
    
static int get_field_data_in_msg_ret(l6msg_struct *msg, l6msg_field *field, 
    int type, void **data, int length, int offset, int count, int *fieldcount)
{
    int ret = 0;
    if(L6_CHECK_MSG_LOCKS && (msg->is_locked))
    {
        msg->error_code = L6_ERR_MSG_LOCKED;
        *data = NULL;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
        return -1;
    }

    if(type != field->type) /* || ((count >= 0) && (count != field->count))) */
    {
        msg->error_code = L6_ERR_FIELD_TYPE_INCOMPATIBLE;
        ret = -1;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "req=%d, found=%d", type, field->type);
        return -1;
    }

    *fieldcount = field->count;
    l6msg_field_type_handler *handler = get_field_type_handler(msg, field->type);
    if(handler)
        ret = handler->get_data(field, data, offset, count);
    else
        ret = -1;
    return ret;
}


static int remove_field_in_msg(l6msg_struct *msg, l6msg_field *field, int index)
{
    msg->error_code = 0;

    if(index >= 0)
        l6_mvec_remove(msg->qfields, index);
    else
        l6_mvec_remove_item(msg->qfields, field);

    if(TRACK_MSG_SIZE_CONTINUALLY)
    {
        msg->total_length    -= field->total_size;
        msg->metadata_length -= field->mdlength;
        msg->data_length     -= field->datalen;
    }
    
    if(field->type == L6_DATATYPE_L6MSG)
    {
        //remove from qsubMsgs
        int len = l6_mvec_size(msg->qsub_msgs);
        int i = 0;
        for(i = 0; i < len; i++)
        {
            l6msg_field *sub_msg_field = (l6msg_field*)l6_mvec_get(msg->qsub_msgs, i);
            if(sub_msg_field == field) 
            {
                //remove this one
                l6_mvec_remove(msg->qsub_msgs, i);
                break;
            }
        }
        //is this internally allocated?
        if(field->internally_allocated)
        {
            //then it must also be freed internally
            l6msg sub_msg = (l6msg)field->data.msg;
            l6msg_free(&sub_msg);
            field->data.msg = NULL;
            field->internally_allocated = 0;
        }
    }
    
    l6msg_field *retfield = NULL;
    if(field->namelen && msg->htbl_fld_name)
        l6_htbl_skey_remove(msg->htbl_fld_name, field->name, (void**)&retfield);

    if((field->fid >= 0) && msg->htbl_fld_id) 
        l6_htbl_ikey_remove(msg->htbl_fld_id, field->fid, (void**)&retfield);

    if(l6_mvec_size(msg->qpool) > msg->poolsize)
    {
        //cleanup
        l6msg_free_field(field);
    }
    else
    {
        //recycle
        l6msg_reset_field(field);
        l6_mvec_push(msg->qpool, field); 
    }
    return 0;
}

int l6msg_size(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    msg->error_code        = 0;
    msg->total_length      = L6_BASE_MSG_HDR_LENGTH;
    msg->metadata_length   = L6_BASE_MSG_HDR_LENGTH;
    msg->data_length       = 0;
    int len = l6_mvec_size(msg->qfields);
    int i = 0;
    for(i = 0; i < len; i++) {
        l6msg_field *field = (l6msg_field*)l6_mvec_get(msg->qfields, i);

        if(field->type == L6_DATATYPE_L6MSG ) 
        {
            //check if it has been changed recently
            l6msg sub_msg = (l6msg)field->data.msg;
            int sub_msg_size = l6msg_size(&sub_msg);

            if(field->datalen != sub_msg_size) {
                //update new field count and size
                field->datalen = sub_msg_size; 
                field->count = (short)field->datalen;
                field->total_size = field->mdlength + field->datalen;
            }
        }
        //updateAggregateLengthByDelta(field.size, field.mdlength, field.datalen);
        msg->total_length    += field->total_size;
        msg->metadata_length += field->mdlength;
        msg->data_length     += field->datalen;
    }
    return msg->total_length;
}


int l6msg_serialize(l6msg *lmsg, char *buffer, int length, int *left)
{
    int ret = 0;
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);

    if(length < L6_BASE_MSG_HDR_LENGTH)
    {
        msg->error_code = L6_ERR_INSUFF_BUFFER_SIZE;
        if(_SET_DEBUG_INFO) 
            sprintf_debug_info(msg, __LINE__, 
                "buffer size %d < %d", length, L6_BASE_MSG_HDR_LENGTH);
        return -1;    //field not found
    }

    l6msg_field *field;

    if(msg->is_locked  == 0)
    {
        msg->is_locked = 1;
        if(msg->chk_submsg_cyc && l6msg_check_cyclic_submsg(msg) < 0)
        {
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
            return -1;
        }
    
        msg->error_code = 0;
        l6msg_size(lmsg);
        msg->offset_md  = 0;
        msg->offset_d   = 0;
        msg->itr        = 0;
        msg->left       = msg->total_length;

        msg->nfields = l6_mvec_size(msg->qfields);
        
        buffer[0] = msg->code;
        buffer[1] = (unsigned char)(msg->nfields);
        msg->offset_md = 2;
        
        //set unused bytes to 0
        *((short*)(buffer + msg->offset_md)) = 0;
        //memcpy(buffer + msg->offset_md, &s, sizeof(int16_t));
        msg->offset_md += sizeof(int16_t);

        short s = htons(msg->total_length);
        *((short*)(buffer + msg->offset_md)) = s;
        //memcpy(buffer + msg->offset_md, &s, sizeof(int16_t));
        msg->offset_md += sizeof(int16_t);

        s = htons(msg->metadata_length);
        *((short*)(buffer + msg->offset_md)) = s;        
        //memcpy(buffer + msg->offset_md, &s, sizeof(int16_t));
        msg->offset_md += sizeof(int16_t);

        msg->offset_d = msg->metadata_length;
        if(msg->total_length > length)
        {
            msg->is_partial = 1;
        }
    }

    if(msg->is_partial)
    {
        if(msg->done == 0)
        {
            if(msg->tmpbuffermem == 0)
            {
                msg->tmpbuffermem = msg->total_length;
                msg->tmpbuffer = (char*)malloc(msg->tmpbuffermem);
            }
            else if(msg->tmpbuffermem < msg->total_length)
            {
                msg->tmpbuffermem = msg->total_length;
                msg->tmpbuffer = (char*)realloc(msg->tmpbuffer, msg->tmpbuffermem);
            }
            if(msg->tmpbuffer == NULL)
            {
                msg->tmpbuffermem = 0;
                msg->error_code = L6_ERR_MEM_ALLOC;
                if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
                return -1;
            }
            msg->tmpbufferage = 0;

            msg->is_partial = 0;
            msg->is_locked = 0;
            ret = l6msg_serialize(lmsg, msg->tmpbuffer, msg->total_length, NULL);
            msg->is_partial = 1;
            msg->is_locked = 1;
            msg->done = 0;
            if(ret < 0)
            {
                return ret;
            }
        }
        if((msg->done + length) > msg->total_length)
        {
            ret = msg->total_length - msg->done;
        }
        else
        {
            ret = length;
        }
        memcpy(buffer, msg->tmpbuffer + msg->done, ret);
        msg->done += ret;
    }
    else
    {
        if(msg->tmpbuffermem)
        {
            msg->tmpbufferage++;
            if(msg->tmpbufferage > L6_MAX_UNUSED_SERIALIZATIONS)
            {
                //free tmpbuffer if unused for L6_MAX_UNUSED_SERIALIZATIONS+ serializations
                free((char*)msg->tmpbuffer);
                msg->tmpbuffermem = 0;
                msg->tmpbuffer       = NULL;
            }
        }

        for(msg->itr = 0; msg->itr < msg->nfields; msg->itr++)
        {
            field = (l6msg_field*)l6_mvec_get(msg->qfields, msg->itr);
                        
            ret = l6msg_serialize_field_metadata(msg, field, buffer);
            if(ret < 0) return ret;

            ret = l6msg_serialize_field_data(msg, field, buffer, left);
            if(ret < 0) return ret;
        }
        msg->done = msg->offset_d;
    }
    msg->left = msg->total_length - msg->done;
    if(left)
    {
        *left = msg->left;
    }
    if(msg->left == 0)
    {
        msg->is_partial = 0;
        msg->is_locked = 0;
    }

    //dbgprint(buffer, msg->done);
    return msg->done;
}


int l6msg_serialize_field_metadata(l6msg_struct *msg, l6msg_field *field, char *buffer)
{
    //write metadata
    int type = field->type | (field->namelen  ? L6_FLAG_FIELD_HAS_NAME : 0) 
                           | (field->fid >= 0 ? L6_FLAG_FIELD_HAS_ID   : 0);
    buffer[msg->offset_md++] = (unsigned char)type;
    
    //write count, if non-primitive
    if(field->type & L6_FLAG_FIELD_IS_ARRAY)
    {
        //write metadata
        short s = htons(field->count);
        *(int16_t*)(buffer + msg->offset_md) = s;
        //memcpy(buffer + msg->offset_md, &s, sizeof(int16_t));
        msg->offset_md += sizeof(int16_t);
    }

    if(field->fid >= 0)
    {
        uint16_t s = htons(field->fid);
        *(int16_t*)(buffer + msg->offset_md) = s;
        //memcpy(buffer + msg->offset_md, &s, sizeof(int16_t));
        msg->offset_md += sizeof(int16_t);
    }
    
    if(field->namelen)
    {
        //buffer[msg->offset_md++] = (unsigned char)(field->type | L6_FLAG_FIELD_HAS_NAME);
        buffer[msg->offset_md++] = (unsigned char)(field->namelen);
        memcpy(buffer + msg->offset_md, field->name, field->namelen);
        msg->offset_md += field->namelen;
    }
    return 0;
}

int l6msg_serialize_field_data(l6msg_struct *msg, l6msg_field *field, char *buffer, int *left)
{
    int ret = 0;

    //serialize fields
    l6msg_field_type_handler *handler = get_field_type_handler(msg, field->type);
    if(handler)
        ret = handler->serialize(field, (buffer + msg->offset_d), msg->do_byte_swap, 1);
    else
        ret = -1;

    if(ret < 0) 
    {
        //l6msg_free_field(field);
        msg->error_code = ret;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
    }
    else
    {
        field->serialized = 1;        
        msg->offset_d += field->datalen;
    }
    return 0;
}

int l6msg_deserialize(l6msg *lmsg, char *buffer, int length, int *left)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return l6msg_deserialize_opt(msg, buffer, length, left, msg->deep_copy);
}

int l6msg_deserialize_in_place(l6msg *lmsg, char *buffer, int length, int *left)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return l6msg_deserialize_opt(msg, buffer, length, left, 0);
}

static int l6msg_deserialize_opt(l6msg_struct *msg, char *buffer, int length, int *left, int copy)
{
    int ret = 0;
    if(length < L6_BASE_MSG_HDR_LENGTH)
    {
        msg->error_code = L6_ERR_INSUFF_BUFFER_SIZE;
        if(_SET_DEBUG_INFO) 
            sprintf_debug_info(msg, __LINE__, 
                "buffer size %d < %d", length, L6_BASE_MSG_HDR_LENGTH);
        return -1;    //field not found
    }
    
    //read base msg header
    if(msg->is_locked == 0)
    {
        __l6msg_reset(msg, 1);
        msg->left = msg->total_length;

        msg->code = buffer[0];
        if(msg->code & L6_FLAG_MSG_IS_TEMPLATE) 
        {
            msg->error_code = L6_ERR_NOT_IMPLEMENTED;
            if(_SET_DEBUG_INFO) 
                sprintf_debug_info(msg, __LINE__, 
                    "Template flag set in %d", msg->code);
            return -1;    //field not found
        }
            
        msg->nfields = (unsigned int)buffer[1];
        msg->offset_md = 4; //skip 2 unused bytes

        //memcpy(&data, buffer + msg->offset_md, sizeof(int16_t));
        uint16_t s = *((uint16_t*)(buffer + msg->offset_md));
        msg->total_length = ntohs(s);
        msg->offset_md += sizeof(int16_t);

        //memcpy(&data, buffer + msg->offset_md, sizeof(int16_t));
        s = *((uint16_t*)(buffer + msg->offset_md));
        msg->metadata_length = ntohs(s);
        msg->offset_md += sizeof(int16_t);

        msg->offset_d = msg->metadata_length;
        msg->is_partial = 0;
        if(msg->total_length > length)
        {
            msg->is_partial = 1;
            msg->is_locked = 1;
        }
    }

    if(msg->is_partial)
    {
        ret = 0;
        if(msg->done == 0)
        {
            if(msg->tmpbuffermem ==0)
            {
                msg->tmpbuffermem = msg->total_length;
                msg->tmpbuffer = (char*)malloc(msg->tmpbuffermem);
            }
            else if(msg->tmpbuffermem < msg->total_length)
            {
                msg->tmpbuffermem = msg->total_length;
                msg->tmpbuffer = (char*)realloc(msg->tmpbuffer, msg->tmpbuffermem);
            }
            if(msg->tmpbuffer == NULL)
            {
                msg->tmpbuffermem = 0;
                msg->error_code = L6_ERR_MEM_ALLOC;
                if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
                return -1;
            }
            msg->tmpbufferage = 0;
        }
        if((msg->done + length) > msg->total_length)
        {
            ret = msg->total_length - msg->done;
        }
        else
        {
            ret = length;
        }
        memcpy(msg->tmpbuffer + msg->done, buffer, ret);
        msg->done += ret;
        msg->left = msg->total_length - msg->done;
        if(left)
        {
            *left = msg->left;
        }
        if(msg->left)
        {
            return msg->done;
        }
        else
        {
            msg->is_partial = 0;
            msg->is_locked = 0;
            buffer = msg->tmpbuffer;
        }
    }
    else
    {
        if(msg->tmpbuffermem)
        {
            if(++msg->tmpbufferage > 25)
            {
                //free tmpbuffer if unused for 25+ serializations
                free((char*)msg->tmpbuffer);
            }
            msg->tmpbuffermem = 0;
            msg->tmpbuffer       = NULL;
        }
    }

    msg->offset_d        = msg->metadata_length;
    msg->total_length    = L6_BASE_MSG_HDR_LENGTH;
    msg->metadata_length = L6_BASE_MSG_HDR_LENGTH;
    msg->data_length     = 0;
    
    /*
    if(remaining < 3)
    {
        msg->errorCode = L6_ERR_INSUFF_BUFFER_SIZE;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
        return -1;
    }
    */

    //read field metadata
    for(msg->itr = 0; msg->itr < msg->nfields; ++msg->itr)
    {
        l6msg_field *field = NULL;
        ret = l6msg_deserialize_field_metadata(msg, &field, buffer, length);
        if(ret < 0) break;

        ret = l6msg_deserialize_field_data(msg, field, buffer, length, left, copy);
        if(ret < 0) break;

        //if check memory access within bounds
        if(msg->offset_d > length) //out of bounds!
        {
            msg->error_code = L6_ERR_OFFSET_OUT_OF_BOUNDS;
            //__l6msg_reset(msg, 0);
            if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "%d > %d", msg->offset_d, length);
            return -1;
        }
    }

    if(ret < 0)
    {
        //if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
        return ret;
    }

    msg->done = msg->offset_d;
    msg->left = msg->total_length - msg->done;
    if(left) 
        *left = msg->left;

    return msg->done;
}

int l6msg_deserialize_field_metadata(l6msg_struct *msg, l6msg_field **field, char *buffer, int length)
{
    unsigned char type = 0;
    char *name = NULL;
    int  namelen = 0;
    int  len = 0;
    int  fid = -1;
    int  count = 1;

    int remaining = length - msg->offset_d;

    //read metadata
    type = (unsigned char)buffer[msg->offset_md++];
    remaining--;
    if(type & L6_FLAG_FIELD_IS_ARRAY)
    {
        //memcpy(&data, buffer + msg->offset_md, sizeof(int16_t));
        uint16_t s = *(unsigned short*)(buffer + msg->offset_md);
        count = ntohs(s);
        msg->offset_md += sizeof(int16_t);
        remaining-=2;
    }

    if(type & L6_FLAG_FIELD_HAS_ID)
    {
        //memcpy(&data, buffer + msg->offset_md, sizeof(int16_t));
        //fid = ntohs(data.s);
        fid = ntohs(*(short*)(buffer + msg->offset_md));        msg->offset_md += sizeof(int16_t);
        remaining -= 2;
    }

    if(type & L6_FLAG_FIELD_HAS_NAME)
    {
        //field is named
        namelen = (unsigned int)(buffer[msg->offset_md++]);
        name = buffer + msg->offset_md;
        msg->offset_md += namelen;
        remaining -= (1 + namelen);
    }

    type  &= L6_FLAG_FIELD_TYPE_MASK;    //set_field_data_in_msg(msg, name, fid, type, &data, len, count, 1);
    len = -1;
    if(type & L6_FLAG_FIELD_IS_EXTENSION)
    {
        if(msg->ext_handler != NULL) 
            len = 1;
    }
    else if(type < L6_ALL_DATATYPES_MAX)
        len = l6_field_type_handlers[type].element_size_bytes;

    else

    if(len < 1)
    {
        msg->error_code = L6_ERR_UNHANDLED_FIELD_TYPE;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, "type=%d", type);
        return -1;
    }
    
    return set_field_data_in_msg_ret(msg, field, (_const_char *)name, 
                        (namelen - 1), fid, -1, type, NULL, len, count, 0);
}

int l6msg_deserialize_field_data(l6msg_struct *msg, l6msg_field *field, 
                            char *buffer, int length, int *left, int copy)
{
    int ret = 0;
    
    //check field type handled
    if(msg->offset_d > length)
    {
        msg->error_code = L6_ERR_INSUFF_BUFFER_SIZE;
        if(_SET_DEBUG_INFO) sprintf_debug_info(msg, __LINE__, NULL);
        return -1;
    }
    
    field->serialized = 1;
    l6msg_field_type_handler *handler = get_field_type_handler(msg, field->type);
    if(handler)
        ret = handler->deserialize(field, (buffer + msg->offset_d), msg->do_byte_swap, copy);
    else
        ret = -1;

    if(ret < 0) 
    {
        //l6msg_free_field(field);
        msg->error_code = ret;
        if(_SET_DEBUG_INFO) 
            sprintf_debug_info( msg, __LINE__, 
                "ret=%d field@%d id=%d type=%d", ret, msg->itr, field->fid, field->type);
    }
    else
    {
        msg->offset_d += field->datalen;
        field->offset_data = msg->offset_d;
    }
    return ret;
}

/*
***************************************** MSG FUNCTIONS *************************************
*/

int l6msg_init(l6msg *lmsg)
{
    int opt = 8;

    l6msg_struct *msg = (l6msg_struct *)malloc(sizeof(l6msg_struct));
    (*lmsg) = msg;
    msg->subject            = NULL;
    msg->code               = 0;
    msg->total_length       = L6_BASE_MSG_HDR_LENGTH;
    msg->metadata_length    = L6_BASE_MSG_HDR_LENGTH;
    msg->data_length        = 0;
    msg->poolsize           = opt;
    msg->error_code         = 0;
    msg->deep_copy          = 1;
    msg->ext_handler        = NULL;
    msg->do_byte_swap       = 0;
    l6msg_set_auto_byte_order_mode(lmsg, 1);

    msg->idgen              = 1024; //currently unused, but may be needed

    msg->is_locked          = 0;
    msg->chk_submsg_cyc     = 1;
    msg->is_partial         = 0;
    msg->offset_md          = 0;
    msg->offset_d           = 0;
    msg->left               = 0;
    msg->done               = 0;
    msg->tmpbuffer          = NULL;
    msg->tmpbuffermem       = 0;
    msg->htbl_opt           = opt;
    
    msg->debug              = NULL;
    
    msg->htbl_fld_name      = NULL; //initialize these lazily
    msg->htbl_fld_id        = NULL; //so that user doesn't pay if they're never used
    msg->qsub_msgs          = NULL;

    l6_mvec_init(&(msg->qfields),   sizeof(l6msg_field), L6_MSG_LIST_INIT_CAP);
    l6_mvec_init(&(msg->qpool),     sizeof(l6msg_field), L6_MSG_LIST_INIT_CAP);
    return 0;
}

const char* l6msg_get_debug_info(l6msg *lmsg) {
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return (const char* const)(msg->debug ? msg->debug : "");
}

const char* l6_get_error_str_for_code(int e)
{
    if(e == 0)
        return l6_error_msgs[0];

    if((e > L6_ERR_CODES_MIN) && (e < L6_ERR_CODES_MAX))
        return l6_error_msgs[e - L6_ERR_CODES_OFFSET];

    return l6_error_msgs[1];
}

const char* l6msg_get_error_str(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return l6_get_error_str_for_code(msg->error_code);
}

int    l6msg_get_error_code(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return msg->error_code;
}

int l6msg_free(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    l6msg_field* field=NULL;
    if(msg->subject)
        free((char*)(msg->subject));

    if(msg->tmpbuffermem)
        free((char*)(msg->tmpbuffer));

    if(msg->debug) 
        free(msg->debug);

    //empty all field lists and table entries
    //avoid l6msg_reset as it has unnecessary overhead in q and htbl management

    int i   = 0;
    int len = 0;

    //remove from qsubMsgs if any
    if(msg->qsub_msgs)
    {
        len = l6_mvec_size(msg->qsub_msgs);
        for(i = 0; i < len; i++)
        {
            field = (l6msg_field*)l6_mvec_get(msg->qsub_msgs, i);
            if(field->internally_allocated) 
                l6msg_free((l6msg*)&(field->data.msg));
        }
        l6_mvec_free(msg->qsub_msgs);
    }

    //remove fields from q
    len = l6_mvec_size(msg->qfields);
    for(i = 0; i < len; i++)
        l6msg_free_field((l6msg_field*)l6_mvec_get(msg->qfields, i));
    l6_mvec_free(msg->qfields);

    len = l6_mvec_size(msg->qpool);
    for(i = 0; i < len; i++)
        l6msg_free_field((l6msg_field*)l6_mvec_pop(msg->qpool));
    l6_mvec_free(msg->qpool);
    
    if(msg->htbl_fld_id)    l6_htbl_free(&msg->htbl_fld_id);

    if(msg->htbl_fld_name)  l6_htbl_free(&msg->htbl_fld_name);

    msg->error_code     = 0;

    free(msg);
    (*lmsg) = NULL;
    //memPtrToClassMap.clear();
    return 0;
}

int l6msg_check_cyclic_submsg(l6msg_struct *msg) 
{
    if(msg->qsub_msgs == NULL) return 0;
    int ret = 0;
    int i = 0;
    int len = l6_mvec_size(msg->qsub_msgs);
    for(i = 0; i < len; i++)
    {
        l6msg_field *field = (l6msg_field *)l6_mvec_get(msg->qsub_msgs, i);
        if(field->type == L6_DATATYPE_L6MSG) 
        {
            l6msg_struct *sub_msg = (l6msg_struct*)field->data.msg;
            if(sub_msg == msg) 
            {
                //recursion!
                msg->error_code = L6_ERR_RECURSIVE_SUB_MSG;
                return -1;
            }
            ret = l6msg_check_cyclic_submsg(sub_msg);
        }
    }
    return ret;
}

int __l6msg_reset(l6msg_struct *msg, int reset_err) {
    l6msg_field *field = NULL;

    int i = l6_mvec_size(msg->qfields);
    while(i--)
    {
        field = (l6msg_field*)l6_mvec_get(msg->qfields, i);
        if(remove_field_in_msg(msg, field, i) < 0)
        {
            //WEIRD ERROR!
            break;
        }
    }
    
    msg->code            = 0;
    msg->has_metadata    = 0;
    msg->has_subject     = 0;
    msg->total_length    = L6_BASE_MSG_HDR_LENGTH;
    msg->metadata_length = L6_BASE_MSG_HDR_LENGTH;
    msg->data_length     = 0;
    if(reset_err)
    {
        msg->error_code    = 0;
        if(msg->debug) 
            msg->debug[0]  = '\0';        
    }
    msg->deep_copy         = 1;
    msg->idgen             = 1024;
    if(msg->subject!=NULL)
    {
        free((char*)msg->subject);
        msg->subject=NULL;
    }
    msg->is_locked        = 0;
    msg->is_partial       = 0;
    msg->offset_md        = 0;
    msg->offset_d         = 0;
    msg->left             = 0;
    msg->done             = 0;
 
    msg->internally_allocated = 0;   
    return 0;
}

int l6msg_reset(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return __l6msg_reset(msg, 1);
}


int l6msg_set_deep_copy_default(l6msg *lmsg, int mode)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    msg->deep_copy = mode;
    return 0;
}

int l6msg_get_deep_copy_default(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return msg->deep_copy;
}

int l6msg_set_auto_byte_order_mode(l6msg *lmsg, int byteorder)
{
    l6msg_struct *msg     = (l6msg_struct*)(*lmsg);
    msg->auto_byte_mode   = byteorder;
    if((msg->auto_byte_mode) && (1 != htonl(1)))
    {
        change_byte_order = swap_in_place;
        msg->do_byte_swap = 1;
    }
    else
    {
        change_byte_order = keep_in_place;
        msg->do_byte_swap = 0;
    }
    return 0;
}

int l6msg_get_auto_byte_order_mode(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return msg->auto_byte_mode;
}

int l6msg_set_deserialize_lazily(l6msg *lmsg, int dsoa)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    msg->deserialize_lazily = dsoa;
    return 0;
}

int l6msg_get_deserialize_lazily(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return msg->deserialize_lazily;    
}

int l6msg_set_check_cyclic_submsg(l6msg *lmsg, int chk)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    msg->chk_submsg_cyc = chk;
    return 0;
}

int l6msg_get_check_cyclic_submsg(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return msg->chk_submsg_cyc;
}

/*********************************** GET/SET DATA FUNCTIONS *********************************/

void l6msg_set_code(l6msg *lmsg, int code) 
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    msg->code = (msg->code | ((code & 0x03) << 3));
}

int    l6msg_get_code(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return ((msg->code >> 3) & 0x03);
}

int l6msg_deserialize_template(l6msg *lmsg, int template_id)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    msg->error_code = L6_ERR_NOT_IMPLEMENTED;
    return -1;
}

int l6msg_set_templated(l6msg *lmsg, int template_id)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    msg->error_code = L6_ERR_NOT_IMPLEMENTED;
    return -1;
}

int l6msg_is_templated(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    msg->error_code = L6_ERR_NOT_IMPLEMENTED;
    return -1;
}

/*
#define __CONCAT(...) __VA_ARGS__

#define DEF_SET_FIELD_VAL(SETADD, TYPENAME, TYPECODE, TYPE, DATASIZE, BYREFORVAL, ACCESS, KEYTYPE, KEY, KEYVAL) \
int l6msg_##SETADD##BYREFORVAL##_##TYPENAME##ACCESS##(l6msg *lmsg, __CONCAT(KEYTYPE KEY, TYPE) data)            \
{                                                                                                               \
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);                                                                 \
    return set_field_data_in_msg_ret##ACCESS##(msg, __CONCAT(KEY,KEYVAL), TYPECODE, &data, DATASIZE, 1, msg->deep_copy); \
}

DEF_SET_FIELD_VAL(add, byte, L6_DATATYPE_BYTE, char, 1,,,,, -1)
*/

int l6msg_set_int16(l6msg *lmsg, int fid, int16_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_SHORT, 
                            &data, sizeof(int16_t), 1, msg->deep_copy);
}

int l6msg_set_short(l6msg *msg, int fid, short data)
{ 
    return l6msg_set_int16(msg, fid, (int16_t) data); 
}

int l6msg_set_int32(l6msg *lmsg, int fid, int32_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_INT32, 
                            &data, sizeof(int32_t), 1, msg->deep_copy);
}

int l6msg_set_int(l6msg *msg, int fid, int data)
{ 
    return l6msg_set_int32 (msg, fid, (int32_t) data); 
}

int l6msg_set_int64(l6msg *lmsg, int fid, int64_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_INT64, 
                            &data, sizeof(int64_t), 1, msg->deep_copy);
}

int l6msg_set_long(l6msg *msg, int fid, long long int data)
{ 
    return l6msg_set_int64(msg, fid, (int64_t) data); 
}

int l6msg_set_float(l6msg *lmsg, int fid, float data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_FLOAT, 
                            &data, sizeof(float), 1, msg->deep_copy);
}

int l6msg_set_double(l6msg *lmsg, int fid, double data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_DOUBLE, 
                    &data, sizeof(double), 1, msg->deep_copy);
}

//By Index
int l6msg_set_int16_at_index(l6msg *lmsg, int index, int16_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_SHORT, 
                    &data, sizeof(int16_t), 1, msg->deep_copy);
}

int l6msg_set_short_at_index(l6msg *msg, int index, short data)
{ 
    return l6msg_set_int16_at_index(msg, index, (int16_t) data); 
}

int l6msg_set_int32_at_index(l6msg *lmsg, int index, int32_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT32, 
                    &data, sizeof(int32_t), 1, msg->deep_copy);
}

int l6msg_set_int_at_index(l6msg *msg, int index, int data)
{ 
    return l6msg_set_int32_at_index (msg, index, (int32_t) data); 
}

int l6msg_set_int64_at_index(l6msg *lmsg, int index, int64_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT64, 
                            &data, sizeof(int64_t), 1, msg->deep_copy);
}

int l6msg_set_long_at_index(l6msg *msg, int index, long long int data)
{ 
    return l6msg_set_int64_at_index (msg, index, (int64_t) data); 
}

int l6msg_set_float_at_index(l6msg *lmsg, int index, float data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_FLOAT, 
                            &data, sizeof(float), 1, msg->deep_copy);
}

int l6msg_set_double_at_index(l6msg *lmsg, int index, double data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_DOUBLE, 
                            &data, sizeof(double), 1, msg->deep_copy);
}

//Arrays
int l6msg_set_string_at_index(l6msg *lmsg, int index, const char *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    //md = 1 type + 2 len
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_STRING, 
                        (char*)data, 1, strlen(data)+1, msg->deep_copy);
}

int l6msg_set_byte_array_at_index(l6msg *lmsg, int index, char *data, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_BYTES, 
                            data, sizeof(char), size, msg->deep_copy);
}

int l6msg_set_int16_array_at_index(l6msg *lmsg, int index, int16_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_SHORT_ARRAY, 
                            data, sizeof(int16_t), count, msg->deep_copy);
}

int l6msg_set_short_array_at_index(l6msg *msg, int index, short *data, int count)
{ 
    return l6msg_set_int16_array_at_index(msg, index, (int16_t*) data, count); 
}

int l6msg_set_int32_array_at_index(l6msg *lmsg, int index, int32_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT32_ARRAY, 
                            data, sizeof(int32_t), count, msg->deep_copy);
}

int l6msg_set_int_array_at_index(l6msg *msg, int index, int *data, int count)
{ 
    return l6msg_set_int32_array_at_index(msg, index, (int32_t*) data, count); 
}

int l6msg_set_int64_array_at_index(l6msg *lmsg, int index, int64_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT64_ARRAY, 
                            data, sizeof(int64_t), count, msg->deep_copy);
}

int l6msg_set_long_array_at_index(l6msg *msg, int index, long long int *data, int count) 
{ 
    return l6msg_set_int64_array_at_index(msg, index, (int64_t*) data, count); 
}

int l6msg_set_float_array_at_index(l6msg *lmsg, int index, float *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                    L6_DATATYPE_FLOAT_ARRAY, data, sizeof(float), count, msg->deep_copy);
}

int l6msg_set_double_array_at_index(l6msg *lmsg, int index, double *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                    L6_DATATYPE_DOUBLE_ARRAY, data, sizeof(double), count, msg->deep_copy);
}

int l6msg_set_layer6_msg_at_index(l6msg *lmsg, int index, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                    L6_DATATYPE_L6MSG, (*data), 1, l6msg_size(data), msg->deep_copy);
}

//Array Pointers
int l6msg_set_string_ptr_at_index(l6msg *lmsg, int index, const char *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                    L6_DATATYPE_STRING, (char*)data, 1, strlen(data)+1, 0);
}

int l6msg_set_byte_array_ptr_at_index(l6msg *lmsg, int index, char *data, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                        L6_DATATYPE_BYTES, data, 1, size, 0);
}

int l6msg_set_int16_array_ptr_at_index(l6msg *lmsg, int index, int16_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_SHORT_ARRAY, data, sizeof(int16_t), count, 0);
}

int l6msg_set_short_array_ptr_at_index(l6msg *msg, int index, short *data, int count)
{ 
    return l6msg_set_int16_array_ptr_at_index(msg, index, 
                                    (int16_t*) data, count); 
}

int l6msg_set_int32_array_ptr_at_index(l6msg *lmsg, int index, int32_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT32_ARRAY, data, sizeof(int32_t), count, 0);
}

int l6msg_set_int_array_ptr_at_index(l6msg *msg, int index, int *data, int count)
{ 
    return l6msg_set_int32_array_ptr_at_index(msg, index, 
                                    (int32_t*) data, count); 
}

int l6msg_set_int64_array_ptr_at_index(l6msg *lmsg, int index, int64_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT64_ARRAY, data, sizeof(int64_t), count, 0);
}

int l6msg_set_long_array_ptr_at_index(l6msg *msg, int index, long long int *data, int count)
{ 
    return l6msg_set_int64_array_ptr_at_index(msg, index, (int64_t*) data, count); 
}

int l6msg_set_float_array_ptr_at_index(l6msg *lmsg, int index, float *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                    L6_DATATYPE_FLOAT_ARRAY, data, sizeof(float), count, 0);
}

int l6msg_set_double_array_ptr_at_index(l6msg *lmsg, int index, double *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                    L6_DATATYPE_DOUBLE_ARRAY, data, sizeof(double), count, 0);
}

int l6msg_set_layer6_msg_ptr_at_index(l6msg *lmsg, int index, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_at_index(msg, index, 
                    L6_DATATYPE_L6MSG, (*data), 1, l6msg_size(data), 0);
}


//By Name
int l6msg_set_int16_named(l6msg *lmsg, _const_char *key, int16_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                    L6_DATATYPE_SHORT, &data, sizeof(int16_t), 1, msg->deep_copy);
}

int l6msg_set_short_named(l6msg *msg, _const_char *key, short data)
{ 
    return l6msg_set_int16_named(msg, key, (int16_t) data); 
}

int l6msg_set_int32_named(l6msg *lmsg, _const_char *key, int32_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_INT32, &data, sizeof(int32_t), 1, msg->deep_copy);
}

int l6msg_set_int_named(l6msg *msg, _const_char *key, int data)
{ 
    return l6msg_set_int32_named (msg, key, (int32_t) data); 
}

int l6msg_set_int64_named(l6msg *lmsg, _const_char *key, int64_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_INT64, &data, sizeof(int64_t), 1, msg->deep_copy);
}

int l6msg_set_long_named(l6msg *msg, _const_char *key, long long int data)
{ 
    return l6msg_set_int64_named (msg, key, (int64_t) data); 
}

int l6msg_set_float_named(l6msg *lmsg, _const_char *key, float data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_FLOAT, &data, sizeof(float), 1, msg->deep_copy);
}

int l6msg_set_double_named(l6msg *lmsg, _const_char *key, double data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_DOUBLE, &data, sizeof(double), 1, msg->deep_copy);
}

//Arrays
int l6msg_set_string(l6msg *lmsg, int fid, const char *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    //md = 1 type + 2 len
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_STRING, 
                                (char*)data, 1, strlen(data)+1, msg->deep_copy);
}

int l6msg_set_byte_array(l6msg *lmsg, int fid, char *data, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_BYTES, 
                                    data, sizeof(char), size, msg->deep_copy);
}

int l6msg_set_int16_array(l6msg *lmsg, int fid, int16_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_SHORT_ARRAY, 
                                    data, sizeof(int16_t), count, msg->deep_copy);
}

int l6msg_set_short_array(l6msg *msg, int fid, short *data, int count)
{ 
    return l6msg_set_int16_array (msg, fid, (int16_t*) data, count); 
}

int l6msg_set_int32_array(l6msg *lmsg, int fid, int32_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_INT32_ARRAY, 
                                    data, sizeof(int32_t), count, msg->deep_copy);
}

int l6msg_set_int_array(l6msg *msg, int fid, int *data, int count)
{ 
    return l6msg_set_int32_array (msg, fid, (int32_t*) data, count); 
}

int l6msg_set_int64_array(l6msg *lmsg, int fid, int64_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_INT64_ARRAY, 
                                    data, sizeof(int64_t), count, msg->deep_copy);
}

int l6msg_set_long_array(l6msg *msg, int fid, long long int *data, int count) 
{ 
    return l6msg_set_int64_array (msg, fid, (int64_t*) data, count); 
}

int l6msg_set_float_array(l6msg *lmsg, int fid, float *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_FLOAT_ARRAY, 
                                    data, sizeof(float), count, msg->deep_copy);
}

int l6msg_set_double_array(l6msg *lmsg, int fid, double *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_DOUBLE_ARRAY, 
                                    data, sizeof(double), count, msg->deep_copy);
}

int l6msg_set_layer6_msg(l6msg *lmsg, int fid, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_L6MSG, 
                                (*data), 1, l6msg_size(data), msg->deep_copy);
}

//Array Pointers
int l6msg_set_string_ptr(l6msg *lmsg, int fid, const char *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, L6_DATATYPE_STRING, 
                                            (char*)data, 1, strlen(data)+1, 0);
}

int l6msg_set_byte_array_ptr(l6msg *lmsg, int fid, char *data, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, 
                L6_DATATYPE_BYTES, data, 1, size, 0);
}

int l6msg_set_int16_array_ptr(l6msg *lmsg, int fid, int16_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, 
                L6_DATATYPE_SHORT_ARRAY, data, sizeof(int16_t), count, 0);
}

int l6msg_set_short_array_ptr(l6msg *msg, int fid, short *data, int count)
{ 
    return l6msg_set_int16_array_ptr (msg, fid, (int16_t*) data, count); 
}

int l6msg_set_int32_array_ptr(l6msg *lmsg, int fid, int32_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, 
                L6_DATATYPE_INT32_ARRAY, data, sizeof(int32_t), count, 0);
}

int l6msg_set_int_array_ptr(l6msg *msg, int fid, int *data, int count)
{ 
    return l6msg_set_int32_array_ptr (msg, fid, (int32_t*) data, count); 
}

int l6msg_set_int64_array_ptr(l6msg *lmsg, int fid, int64_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, 
                L6_DATATYPE_INT64_ARRAY, data, sizeof(int64_t), count, 0);
}

int l6msg_set_long_array_ptr(l6msg *msg, int fid, long long int *data, int count)
{ 
    return l6msg_set_int64_array_ptr (msg, fid, (int64_t*) data, count); 
}

int l6msg_set_float_array_ptr(l6msg *lmsg, int fid, float *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, 
                L6_DATATYPE_FLOAT_ARRAY, data, sizeof(float), count, 0);
}

int l6msg_set_double_array_ptr(l6msg *lmsg, int fid, double *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, 
                L6_DATATYPE_DOUBLE_ARRAY, data, sizeof(double), count, 0);
}

int l6msg_set_layer6_msg_ptr(l6msg *lmsg, int fid, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_with_id(msg, fid, 
                L6_DATATYPE_L6MSG, (*data), 1, l6msg_size(data), 0);
}


//Arrays by name
int l6msg_set_string_named(l6msg *lmsg, _const_char *key, const char *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_STRING, (char*)data, 1, strlen(data)+1, msg->deep_copy);
}

int l6msg_set_byte_array_named(l6msg *lmsg, _const_char *key, char *data, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_BYTES, data, sizeof(char), size, msg->deep_copy);
}

int l6msg_set_int16_array_named(l6msg *lmsg, _const_char *key, int16_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_SHORT_ARRAY, data, sizeof(int16_t), count, msg->deep_copy);
}

int l6msg_set_short_array_named(l6msg *msg, _const_char *key, short *data, int count)
{ 
    return l6msg_set_int16_array_named (msg, key, (int16_t*) data, count); 
}

int l6msg_set_int32_array_named(l6msg *lmsg, _const_char *key, int32_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_INT32_ARRAY, data, sizeof(int32_t), count, msg->deep_copy);
}

int l6msg_set_int_array_named(l6msg *msg, _const_char *key, int  *data, int count)
{ 
    return l6msg_set_int32_array_named (msg, key, (int32_t*) data, count); 
}

int l6msg_set_int64_array_named(l6msg *lmsg, _const_char *key, int64_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_INT64_ARRAY, data, sizeof(int64_t), count, msg->deep_copy);
}

int l6msg_set_long_array_named(l6msg *msg, _const_char *key, long long int *data, int count)
{ 
    return l6msg_set_int64_array_named (msg, key, (int64_t*) data, count); 
}

int l6msg_set_float_array_named(l6msg *lmsg, _const_char *key, float *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_FLOAT_ARRAY, data, sizeof(float), count, msg->deep_copy);
}

int l6msg_set_double_array_named(l6msg *lmsg, _const_char *key, double *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_DOUBLE_ARRAY, data, sizeof(double), count, msg->deep_copy);
}

int l6msg_set_layer6_msg_named(l6msg *lmsg, _const_char *key, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_L6MSG, (*data), 1, l6msg_size(data), msg->deep_copy);
}

//Array Pointers by name
int l6msg_set_string_ptr_named(l6msg *lmsg, _const_char *key, const char *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_STRING, (char*)data, 1, strlen(data)+1, 0);
}

int l6msg_set_byte_array_ptr_named(l6msg *lmsg, _const_char *key, char *data, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_BYTES, data, sizeof(char), size, 0);
}

int l6msg_set_int16_array_ptr_named(l6msg *lmsg, _const_char *key, int16_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_SHORT_ARRAY, data, sizeof(int16_t), count, 0);
}

int l6msg_set_short_array_ptr_named(l6msg *msg, _const_char *key, short *data, int count)
{ 
    return l6msg_set_int16_array_ptr_named (msg, key, (int16_t*)data, count); 
}

int l6msg_set_int32_array_ptr_named(l6msg *lmsg, _const_char *key, int32_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_INT32_ARRAY, data, sizeof(int32_t), count, 0);
}

int l6msg_set_int_array_ptr_named(l6msg *msg, _const_char *key, int *data, int count)     
{ 
    return l6msg_set_int32_array_ptr_named (msg, key, (int32_t*)data, count); 
}

int l6msg_set_int64_array_ptr_named(l6msg *lmsg, _const_char *key, int64_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_INT64_ARRAY, data, sizeof(int64_t), count, 0);
}

int l6msg_set_long_array_ptr_named    (l6msg *msg, _const_char *key, long long int *data, int count)  
{ 
    return l6msg_set_int64_array_ptr_named (msg, key, (int64_t*)data, count); 
}

int l6msg_set_float_array_ptr_named(l6msg *lmsg, _const_char *key, float *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_FLOAT_ARRAY, data, sizeof(float), count, 0);
}

int l6msg_set_double_array_ptr_named(l6msg *lmsg, _const_char *key, double *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_DOUBLE_ARRAY, &data, sizeof(double), count, 0);
}

int l6msg_set_layer6_msg_ptr_named(l6msg *lmsg, _const_char *key, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return set_field_data_in_msg_named(msg, key,  
                L6_DATATYPE_L6MSG, (*data), 1, l6msg_size(data), 0);
}

//Get Scalars
int l6msg_get_int16(l6msg *lmsg, int fid, int16_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_SHORT, 
                            (void**)&data, sizeof(int16_t), 0, 1, &fcount);
}

int l6msg_get_short(l6msg *msg, int fid, short *data)
{ 
    return l6msg_get_int16 (msg, fid, (int16_t*)data); 
}

int l6msg_get_int32(l6msg *lmsg, int fid, int32_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_INT32, 
                            (void**)&data, sizeof(int32_t), 0, 1, &fcount);
}

int l6msg_get_int(l6msg *msg, int fid, int *data) 
{ 
    return l6msg_get_int32 (msg, fid, (int32_t*)data); 
}

int l6msg_get_int64(l6msg *lmsg, int fid, int64_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_INT64, 
                            (void**)&data, sizeof(int64_t), 0, 1, &fcount);
}

int l6msg_get_long(l6msg *msg, int fid, long long int *data)
{ 
    return l6msg_get_int64 (msg, fid, (int64_t*)data); 
}

int l6msg_get_float(l6msg *lmsg, int fid, float *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_FLOAT, 
                            (void**)&data, sizeof(float), 0, 1, &fcount);
}

int l6msg_get_double(l6msg *lmsg, int fid, double *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_DOUBLE, 
                            (void**)&data, sizeof(double), 0, 1, &fcount);
}

//Get Scalars by name
int l6msg_get_int16_named(l6msg *lmsg, _const_char *key, int16_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_SHORT, 
                            (void**)&data, sizeof(int16_t), 0, 1, &fcount);
}

int l6msg_get_short_named(l6msg *msg, _const_char *key, short *data)
{ 
    return l6msg_get_int16_named (msg, key, (int16_t*)data); 
}

int l6msg_get_int32_named(l6msg *lmsg, _const_char *key, int32_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_INT32, 
                            (void**)&data, sizeof(int32_t), 0, 1, &fcount);
}

int l6msg_get_int_named(l6msg *msg, _const_char *key, int *data)
{ 
    return l6msg_get_int32_named (msg, key, (int32_t*)data); 
}

int l6msg_get_int64_named(l6msg *lmsg, _const_char *key, int64_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_INT64, 
                            (void**)&data, sizeof(int64_t), 0, 1, &fcount);
}

int l6msg_get_long_named(l6msg *msg, _const_char *key, long long int *data)
{ 
    return l6msg_get_int64_named (msg, key, (int64_t*)data); 
}

int l6msg_get_float_named(l6msg *lmsg, _const_char *key, float *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_FLOAT, 
                            (void**)&data, sizeof(float), 0, 1, &fcount);
}

int l6msg_get_double_named(l6msg *lmsg, _const_char *key, double *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_DOUBLE, 
                            (void**)&data, sizeof(double), 0, 1, &fcount);
}


// Get Arrays
int l6msg_get_string(l6msg *lmsg, int fid, char *data, int len)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    int ret = get_field_data_in_msg_by_id(msg, fid, 
                L6_DATATYPE_STRING, (void**)&data, 1, 0, len, &fcount);
    if(ret >= 0)
    {
        data[len-1] = '\0';
    }
    return ret;
}

int l6msg_get_byte_array(l6msg *lmsg, int fid, char *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_BYTES, 
                        (void**)&data, sizeof(char), 0, count, &fcount);
}

int l6msg_get_int16_array(l6msg *lmsg, int fid, int16_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_SHORT_ARRAY, 
                        (void**)&data, sizeof(int16_t), offset, count, &fcount);
}

int l6msg_get_short_array(l6msg *msg, int fid, short *data, int offset, int count)  
{ 
    return l6msg_get_int16_array (msg, fid, (int16_t*)data, offset, count); 
}

int l6msg_get_int32_array(l6msg *lmsg, int fid, int32_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_INT32_ARRAY, 
                        (void**)&data, sizeof(int32_t), offset, count, &fcount);
}

int l6msg_get_int_array(l6msg *msg, int fid, int *data, int offset, int count)  
{ 
    return l6msg_get_int32_array (msg, fid, (int32_t*)data, offset, count); 
}

int l6msg_get_int64_array(l6msg *lmsg, int fid, int64_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_INT64_ARRAY, 
                        (void**)&data, sizeof(int64_t), offset, count, &fcount);
}

int l6msg_get_long_array(l6msg *msg, int fid, 
                        long long int *data, int offset, int count)
{ 
    return l6msg_get_int64_array (msg, fid, (int64_t*)data, offset, count); 
}

int l6msg_get_float_array(l6msg *lmsg, int fid, float *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_FLOAT_ARRAY, 
                        (void**)&data, sizeof(float), offset, count, &fcount);
}

int l6msg_get_double_array(l6msg *lmsg, int fid, double *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_DOUBLE_ARRAY, 
                        (void**)&data, sizeof(double), offset, count, &fcount);
}

int l6msg_get_layer6_msg(l6msg *lmsg, int fid, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_L6MSG, 
                        (void**)&data, 1, 0, 1, &fcount);
}

int l6msg_get_layer6_msg_ptr(l6msg *lmsg, int fid, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_L6MSG, 
                        (void**)&data, 1, 0, -1, &fcount);
}

//Get Arrays By Name
int l6msg_get_string_named(l6msg *lmsg, _const_char *key, char *data, int len)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    int ret = get_field_data_in_msg_named(msg, key, 
                L6_DATATYPE_STRING, (void**)&data, 1, 0, len, &fcount);
    if(ret >= 0)
    {
        data[len-1] = '\0';
    }
    return ret;
}

int l6msg_get_byte_array_named(l6msg *lmsg, _const_char *key, 
                                char *data, int offset, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_BYTES, 
                        (void**)&data, sizeof(char), offset, size, &fcount);
}

int l6msg_get_int16_array_named(l6msg *lmsg, _const_char *key, 
                                int16_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_SHORT_ARRAY, 
                        (void**)&data, sizeof(int16_t), offset, count, &fcount);
}

int l6msg_get_short_array_named(l6msg *msg, _const_char *key, 
                                short *data, int offset, int count) 
{ 
    return l6msg_get_int16_array_named(msg,key,(int16_t*)data,offset,count); 
}

int l6msg_get_int32_array_named(l6msg *lmsg, _const_char *key, 
                                int32_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_INT32_ARRAY, 
                        (void**)&data, sizeof(int32_t), offset, count, &fcount);
}

int l6msg_get_int_array_named(l6msg *msg, _const_char *key, 
                                int *data, int offset, int count) 
{ 
    return l6msg_get_int32_array_named(msg,key,(int32_t*)data,offset,count); 
}

int l6msg_get_int64_array_named(l6msg *lmsg, _const_char *key, 
                                int64_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_INT64_ARRAY, 
                        (void**)&data, sizeof(int64_t), offset, count, &fcount);
}

int l6msg_get_long_array_named(l6msg *msg, _const_char *key, 
                                long long int *data, int offset, int count)
{ 
    return l6msg_get_int64_array_named(msg,key,(int64_t*)data, offset, count); 
}

int l6msg_get_float_array_named(l6msg *lmsg, _const_char *key, 
                                float *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_FLOAT_ARRAY, 
                        (void**)&data, sizeof(float), offset, count, &fcount);
}

int l6msg_get_double_array_named(l6msg *lmsg, _const_char *key, 
                                double *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_DOUBLE_ARRAY, 
                        (void**)&data, sizeof(double), offset, count, &fcount);
}

int l6msg_get_layer6_msg_named(l6msg *lmsg, _const_char *key, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_L6MSG, 
                        (void**)&data, sizeof(char), 0, 0, &fcount);
}

int l6msg_get_layer6_msg_ptr_named(l6msg *lmsg, _const_char *key, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_L6MSG, 
                        (void**)&data, sizeof(char), 0, -1, &fcount);
}

//Get Array Pointers
int l6msg_get_string_ptr(l6msg *lmsg, int fid, const char **data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int size;
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_STRING, 
                        (void**)data, sizeof(char), 0, -1, &size);
}

int l6msg_get_byte_array_ptr(l6msg *lmsg, int fid, char **data, int *size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_BYTES, 
                        (void**)data, sizeof(char), 0, -1, size);
}

int l6msg_get_int16_array_ptr(l6msg *lmsg, int fid, short **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_SHORT_ARRAY, 
                        (void**)data, sizeof(int16_t), 0, -1, count);
}

int l6msg_get_short_array_ptr(l6msg *msg, int fid, short **data, int *count)     
{ 
    return l6msg_get_int16_array_ptr (msg, fid, (int16_t**)data, count); 
}

int l6msg_get_int32_array_ptr(l6msg *lmsg, int fid, int **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_INT32_ARRAY, 
                        (void**)data, sizeof(int32_t), 0, -1, count);
}

int l6msg_get_int_array_ptr(l6msg *msg, int fid, int **data, int *count)
{ 
    return l6msg_get_int32_array_ptr (msg, fid, (int32_t**)data, count); 
}

int l6msg_get_int64_array_ptr(l6msg *lmsg, int fid, long long int **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_INT64_ARRAY, 
                        (void**)data, sizeof(int64_t), 0, -1, count);
}

int l6msg_get_long_array_ptr(l6msg *msg, int fid, long long int **data, int *count)
{ 
    return l6msg_get_int64_array_ptr (msg, fid, (int64_t**)data, count); 
}

int l6msg_get_float_array_ptr(l6msg *lmsg, int fid, float **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_FLOAT_ARRAY, 
                        (void**)data, sizeof(float), 0, -1, count);
}

int l6msg_get_double_array_ptr(l6msg *lmsg, int fid, double **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_by_id(msg, fid, L6_DATATYPE_DOUBLE_ARRAY, 
                        (void**)data, sizeof(double), 0, -1, count);
}

//Get Array Pointers by name
int l6msg_get_string_ptr_named(l6msg *lmsg, _const_char *key, 
                                const char **data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int size;
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_STRING, 
                        (void**)data, sizeof(char), 0, -1, &size);
}

int l6msg_get_byte_array_ptr_named(l6msg *lmsg, _const_char *key, 
                                    char **data, int *size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_BYTES, 
                        (void**)data, sizeof(char), 0, -1, size);
}

int l6msg_get_int16_array_ptr_named(l6msg *lmsg, _const_char *key, 
                                    short **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_SHORT_ARRAY, 
                        (void**)data, sizeof(int16_t), 0, -1, count);
}

int l6msg_get_short_array_ptr_named(l6msg *msg, _const_char *key, 
                                    short **data, int *count)
{ 
    return l6msg_get_int16_array_ptr_named (msg, key, (int16_t**)data, count); 
}

int l6msg_get_int32_array_ptr_named(l6msg *lmsg, _const_char *key, 
                                    int **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_INT32_ARRAY, 
                        (void**)data, sizeof(int32_t), 0, -1, count);
}

int l6msg_get_int_array_ptr_named(l6msg *msg, _const_char *key, 
                                    int **data, int *count)
{ 
    return l6msg_get_int32_array_ptr_named (msg, key, (int32_t**)data, count); 
}

int l6msg_get_int64_array_ptr_named(l6msg *lmsg, _const_char *key, 
                                    long long int **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_INT64_ARRAY, 
                        (void**)data, sizeof(int64_t), 0, -1, count);
}

int l6msg_get_long_array_ptr_named(l6msg *msg, _const_char *key, 
                                    long long int **data, int *count)
{ 
    return l6msg_get_int64_array_ptr_named (msg, key, (int64_t**)data, count); 
}

int l6msg_get_float_array_ptr_named(l6msg *lmsg, _const_char *key, 
                                    float **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_FLOAT_ARRAY, 
                        (void**)data, sizeof(float), 0, -1, count);
}

int l6msg_get_double_array_ptr_named(l6msg *lmsg, _const_char *key, 
                                    double **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_named(msg, key, L6_DATATYPE_DOUBLE_ARRAY, 
                        (void**)data, sizeof(double), 0, -1, count);
}


//Add fields
int l6msg_add_int16(l6msg *lmsg, int16_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_SHORT, 
                        &data, sizeof(int16_t), 1, msg->deep_copy);
}

int l6msg_add_short(l6msg *msg, short data)
{ 
    return l6msg_add_int16(msg, (int16_t) data); 
}

int l6msg_add_int32(l6msg *lmsg, int32_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_INT32, 
                        &data, sizeof(int32_t), 1, msg->deep_copy);
}

int l6msg_add_int(l6msg *msg, int data)
{ 
    return l6msg_add_int32 (msg, (int32_t) data); 
}

int l6msg_add_int64(l6msg *lmsg, int64_t data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_INT64, 
                        &data, sizeof(int64_t), 1, msg->deep_copy);
}

int l6msg_add_long(l6msg *msg, long long int data)
{ 
    return l6msg_add_int64(msg, (int64_t) data); 
}

int l6msg_add_float(l6msg *lmsg, float data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_FLOAT, 
                        &data, sizeof(float), 1, msg->deep_copy);
}

int l6msg_add_double(l6msg *lmsg, double data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_DOUBLE, 
                        &data, sizeof(double), 1, msg->deep_copy);
}


int l6msg_add_string(l6msg *lmsg, char* data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    //md = 1 type + 2 len
    return add_field_data_in_msg(msg, L6_DATATYPE_STRING, 
                        data, sizeof(char), strlen(data)+1, msg->deep_copy);
}

int l6msg_add_byte_array(l6msg *lmsg, char *data, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_BYTES, 
                        data, sizeof(char), size, msg->deep_copy);
}

int l6msg_add_int16_array(l6msg *lmsg, int16_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_SHORT_ARRAY, 
                        data, sizeof(int16_t), count, msg->deep_copy);
}

int l6msg_add_short_array(l6msg *msg, short *data, int count)
{ 
    return l6msg_add_int16_array (msg, (int16_t*) data, count); 
}

int l6msg_add_int32_array(l6msg *lmsg, int32_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_INT32_ARRAY, 
                        data, sizeof(int32_t), count, msg->deep_copy);
}

int l6msg_add_int_array(l6msg *msg, int *data, int count)
{ 
    return l6msg_add_int32_array (msg, (int32_t*) data, count); 
}

int l6msg_add_int64_array(l6msg *lmsg, int64_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_INT64_ARRAY, 
                        data, sizeof(int64_t), count, msg->deep_copy);
}

int l6msg_add_long_array(l6msg *msg, long long int *data, int count) 
{ 
    return l6msg_add_int64_array (msg, (int64_t*) data, count); 
}

int l6msg_add_float_array(l6msg *lmsg, float *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_FLOAT_ARRAY, 
                        data, sizeof(float), count, msg->deep_copy);
}

int l6msg_add_double_array(l6msg *lmsg, double *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_DOUBLE_ARRAY, 
                        data, sizeof(double), count, msg->deep_copy);
}

int l6msg_add_layer6_msg(l6msg *lmsg, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_L6MSG, 
                        (*data), 1, l6msg_size(data), msg->deep_copy);
}

//Array Pointers
int l6msg_add_string_ptr(l6msg *lmsg, char *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_STRING, 
                        data, 1, strlen(data)+1, 0);
}

int l6msg_add_byte_array_ptr(l6msg *lmsg, char *data, int size)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_BYTES, 
                        data, 1, size, 0);
}

int l6msg_add_int16_array_ptr(l6msg *lmsg, int16_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_SHORT_ARRAY, 
                        data, sizeof(int16_t), count, 0);
}

int l6msg_add_short_array_ptr(l6msg *msg, short *data, int count)
{ 
    return l6msg_add_int16_array_ptr (msg, (int16_t*) data, count); 
}

int l6msg_add_int32_array_ptr(l6msg *lmsg, int32_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_INT32_ARRAY, 
                        data, sizeof(int32_t), count, 0);
}

int l6msg_add_int_array_ptr(l6msg *msg, int *data, int count)
{ 
    return l6msg_add_int32_array_ptr(msg, (int32_t*) data, count); 
}

int l6msg_add_int64_array_ptr(l6msg *lmsg, int64_t *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_INT64_ARRAY, 
                        data, sizeof(int64_t), count, 0);
}

int l6msg_add_long_array_ptr(l6msg *msg, long long int *data, int count)
{ 
    return l6msg_add_int64_array_ptr (msg, (int64_t*) data, count); 
}

int l6msg_add_float_array_ptr(l6msg *lmsg, float *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_FLOAT_ARRAY, 
                        data, sizeof(float), count, 0);
}

int l6msg_add_double_array_ptr(l6msg *lmsg, double *data, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_DOUBLE_ARRAY, 
                        data, sizeof(double), count, 0);
}

int l6msg_add_layer6_msg_ptr(l6msg *lmsg, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return add_field_data_in_msg(msg, L6_DATATYPE_L6MSG, 
                        (*data), 1, l6msg_size(data), 0);
}

//Get Scalars by index
int l6msg_get_int16_at_index(l6msg *lmsg, int index, int16_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT16, 
                        (void**)&data, sizeof(int16_t), 0, 1, &fcount);
}

int l6msg_get_short_at_index(l6msg *lmsg, int index, short *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_SHORT, 
                        (void**)&data, sizeof(int16_t), 0, 1, &fcount);
}

int l6msg_get_int32_at_index(l6msg *lmsg, int index, int32_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT32, 
                        (void**)&data, sizeof(int32_t), 0, 1, &fcount);
}

int l6msg_get_int_at_index(l6msg *lmsg, int index, int *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT32, 
                        (void**)&data, sizeof(int32_t), 0, 1, &fcount);
}

int l6msg_get_int64_at_index(l6msg *lmsg, int index, int64_t *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT64, 
                        (void**)&data, sizeof(int64_t), 0, 1, &fcount);
}

int l6msg_get_long_at_index(l6msg *lmsg, int index, long long int  *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT64, 
                        (void**)&data, sizeof(int64_t), 0, 1, &fcount);
}

int l6msg_get_float_at_index(l6msg *lmsg, int index, float *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount = 0;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_FLOAT, 
                        (void**)&data, sizeof(float), 0, 1, &fcount);
}

int l6msg_get_double_at_index(l6msg *lmsg, int index, double *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount = 0;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_DOUBLE, 
                        (void**)&data, sizeof(double), 0, 1, &fcount);
}

int l6msg_get_string_at_index(l6msg *lmsg, int index, char *data, int len)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    int ret = get_field_data_in_msg_at_index(msg, index, 
                    L6_DATATYPE_STRING, (void**)&data, 1, 0, len, &fcount);
    if(ret >= 0)
    {
        data[len-1] = '\0';
    }
    return ret;
}


// Get Arrays By index
int l6msg_get_byte_array_at_index(l6msg *lmsg, int index, 
                                    char *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_BYTES, 
                        (void**)&data, sizeof(char), 0, count, &fcount);
}

int l6msg_get_int16_array_at_index(l6msg *lmsg, int index, 
                                    int16_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT16, 
                        (void**)&data, sizeof(int16_t), 0, count, &fcount);
}


int l6msg_get_short_array_at_index(l6msg *lmsg, int index, 
                                    short *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT16_ARRAY, 
                        (void**)&data, sizeof(int16_t), 0, count, &fcount);
}


int l6msg_get_int32_array_at_index(l6msg *lmsg, int index, 
                                    int32_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT32_ARRAY, 
                        (void**)&data, sizeof(int32_t), 0, count, &fcount);
}


int l6msg_get_int_array_at_index(l6msg *lmsg, int index, 
                                    int *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT32_ARRAY, 
                        (void**)&data, sizeof(int32_t), 0, count, &fcount);
}


int l6msg_get_int64_array_at_index(l6msg *lmsg, int index, 
                                    int64_t *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT64_ARRAY, 
                        (void**)&data, sizeof(int64_t), 0, count, &fcount);
}


int l6msg_get_long_array_at_index(l6msg *lmsg, int index, 
                                    long long int *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, L6_DATATYPE_INT64_ARRAY, 
                        (void**)&data, sizeof(int64_t), 0, count, &fcount);
}


int l6msg_get_float_array_at_index(l6msg *lmsg, int index, 
                                    float *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, 
            L6_DATATYPE_FLOAT_ARRAY, (void**)&data, sizeof(float), 0, count, &fcount);
}


int l6msg_get_double_array_at_index(l6msg *lmsg, int index, 
                                    double *data, int offset, int count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, 
            L6_DATATYPE_DOUBLE_ARRAY, (void**)&data, sizeof(double), 0, count, &fcount);
}

int l6msg_get_layer6_msg_at_index(l6msg *lmsg, int index, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount;
    return get_field_data_in_msg_at_index(msg, index, 
                        L6_DATATYPE_L6MSG, (void**)&data, 1, 0, 0, &fcount);
}

//Get Array Pointers by index
int l6msg_get_string_ptr_at_index(l6msg *lmsg, int index, const char **data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount = 0;
    return get_field_data_in_msg_at_index(msg, index, 
                        L6_DATATYPE_STRING, (void**)data, sizeof(char), 0, -1, &fcount);
}

int l6msg_get_byte_array_ptr_at_index(l6msg *lmsg, int index, 
                                        char **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT8_ARRAY, (void**)data, sizeof(char), 0, -1, count);
}

int l6msg_get_int16_array_ptr_at_index(l6msg *lmsg, int index, 
                                        int16_t **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT16_ARRAY, (void**)data, sizeof(int16_t), 0, -1, count);
}

int l6msg_get_short_array_ptr_at_index(l6msg *lmsg, int index, 
                                        short  **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT16_ARRAY, (void**)data, sizeof(int16_t), 0, -1, count);
}

int l6msg_get_int32_array_ptr_at_index(l6msg *lmsg, int index, 
                                        int32_t **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT32_ARRAY, (void**)data, sizeof(int32_t), 0, -1, count);
}

int l6msg_get_int_array_ptr_at_index(l6msg *lmsg, int index, 
                                        int **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT32_ARRAY, (void**)data, sizeof(int32_t), 0, -1, count);
}

int l6msg_get_int64_array_ptr_at_index(l6msg *lmsg, int index, 
                                        int64_t **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT64_ARRAY, (void**)data, sizeof(int64_t), 0, -1, count);
}

int l6msg_get_long_array_ptr_at_index(l6msg *lmsg, int index, 
                                        long long int **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_INT64_ARRAY, (void**)data, sizeof(int64_t), 0, -1, count);
}

int l6msg_get_float_array_ptr_at_index(l6msg *lmsg, int index, 
                                        float **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_FLOAT_ARRAY, (void**)data, sizeof(float), 0, -1, count);
}

int l6msg_get_double_array_ptr_at_index(l6msg *lmsg, int index, 
                                        double **data, int *count)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_DOUBLE_ARRAY, (void**)data, sizeof(double), 0, -1, count);
}

int l6msg_get_layer6_msg_ptr_at_index(l6msg *lmsg, int index, l6msg *data)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    int fcount = 0;
    return get_field_data_in_msg_at_index(msg, index, 
                L6_DATATYPE_L6MSG, (void**)&data, sizeof(char), 0, -1, &fcount);
}


int l6msg_dup(l6msg *lmsrc, l6msg *lmdst) 
{
    l6msg_struct *msg1 = (l6msg_struct *)(*lmsrc);
    l6msg_struct *msg2 = (l6msg_struct *)(*lmdst);

    //reset destination msg
    __l6msg_reset(msg2, 1);

    int ret = 0;
    //iterate thru fields and set
    int i = 0;
    int len = l6_mvec_size(msg1->qfields);
    for(i = 0; i < len; i++)
    {
        l6msg_field *field = (l6msg_field *) l6_mvec_get(msg1->qfields, i);
        //debug_field(field);
        void *ptr = NULL;
        switch(field->type)
        {
            case L6_DATATYPE_INT8:
            case L6_DATATYPE_UINT8:
                ptr = &(field->data.s);
            case L6_DATATYPE_INT16:
            case L6_DATATYPE_UINT16:
            //case L6_DATATYPE_SHORT:
                ptr = &(field->data.s);
                break;
            case L6_DATATYPE_UINT32:
            case L6_DATATYPE_INT32:
                ptr = &(field->data.i);
                break;
            case L6_DATATYPE_UINT64:
            case L6_DATATYPE_INT64:
            //case L6_DATATYPE_LONG:
                ptr = &(field->data.l);
                break;
            case L6_DATATYPE_FLOAT:
                ptr = &(field->data.f);
                break;
            case L6_DATATYPE_DOUBLE:
                ptr = &(field->data.d);
                break;
            case L6_DATATYPE_L6MSG:
                ptr = field->data.msg;
                break;
            default:    //all non-primitive fields
                ptr = field->data.ptr;
                break;
        }
        /* *** Must decrement field->namelen, else leads to weird bugs!! 
        It is incremented by 1 while setting in field */
        l6msg_field *retfield = NULL;
        ret = set_field_data_in_msg_ret(msg2, &retfield,
                (field->namelen ? field->name : NULL),
                    (field->namelen - 1),  field->fid, i, field->type, ptr, 
                            field->elem_size, field->count, msg2->deep_copy);
    }
    return ret;
}

/*********************************** FIELD FUNCTIONS *********************************/

int l6msg_remove_field_at_index(l6msg *lmsg, int index)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    l6msg_field *field = NULL;
    int ret = get_field_at_index(msg, index, &field);
    if(field) ret = remove_field_in_msg(msg, field, index);
    return ret;
}


int l6msg_remove_field(l6msg *lmsg, int fid)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    l6msg_field *field = NULL;
    int ret = get_field_by_id(msg, fid, &field);
    if(field) ret = remove_field_in_msg(msg, field, -1);
    return ret;
}

int l6msg_remove_field_named(l6msg *lmsg, _const_char *key)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    l6msg_field *field = NULL;
    int ret = get_field_named(msg, key, &field);
    if(field) ret = remove_field_in_msg(msg, field, -1);
    return ret;
}

//field ops
int l6msg_get_num_fields(l6msg *lmsg)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return l6_mvec_size(msg->qfields);
}

static int get_field_id(l6msg_struct *msg, l6msg_field *field, int *fid)
{
    if(field->fid >= 0)
    {
        *fid = field->fid;
    }
    else
    {
        *fid = -1;
        msg->error_code = L6_ERR_FIELD_NO_ID;
        return -1;
    }
    return 0;
}

int l6msg_get_field_id_at_index(l6msg *lmsg, int index, int *fid)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    l6msg_field *field = NULL;
    int ret = get_field_at_index(msg, index, &field);
    if(field) ret = get_field_id(msg, field, fid);
    return ret;
}

int l6msg_get_field_id_named(l6msg *lmsg, _const_char *key, int *fid)
{
    l6msg_struct *msg = msg;
    l6msg_field *field = NULL;
    int ret = get_field_named((l6msg_struct*)(*lmsg), key, &field);
    if(field) ret = get_field_id(msg, field, fid);
    return ret;
}

static int get_field_name(l6msg_struct *msg, l6msg_field *field, const char **key)
{
    if(field->namelen)
    {
        *key = field->name;
    }
    else
    {
        msg->error_code = L6_ERR_FIELD_UNNAMED;
        return -1;
    }
    return 0;
}

int l6msg_get_field_name_at_index(l6msg *lmsg, int index, const char **key)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    l6msg_field *field = NULL;
    int ret = get_field_at_index(msg, index, &field);
    if(field) ret = get_field_name(msg, field, key);
    return ret;
}

int l6msg_get_field_name(l6msg *lmsg, int fid, const char **key)
{
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    l6msg_field *field = NULL;
    int ret = get_field_by_id(msg, fid, &field);
    if(field) ret = get_field_name(msg, field, key);
    return ret;
}

int l6msg_is_field_named(l6msg *lmsg, int fid)
{
    l6msg_field *field = NULL;
    int ret = get_field_by_id((l6msg_struct*)(*lmsg), fid, &field);
    if(field) ret = (field->namelen > 0);
    return ret;
}

int l6msg_is_field_named_at_index(l6msg *lmsg, int index)
{
    l6msg_field *field = NULL;
    int ret = get_field_at_index((l6msg_struct*)(*lmsg), index, &field);
    if(field) ret = (field->namelen > 0);
    return ret;
}

int l6msg_get_field_unit_size_at_index(l6msg *lmsg, int index, int *size)
{
    l6msg_field *field = NULL;
    int ret = get_field_at_index((l6msg_struct*)(*lmsg), index, &field);
    if(field) *size = field->elem_size;
    return ret;
}

int l6msg_get_field_unit_size(l6msg *lmsg, int fid, int *size)
{
    l6msg_field *field = NULL;
    int ret = get_field_by_id((l6msg_struct*)(*lmsg), fid, &field);
    if(field) *size = field->elem_size;
    return ret;
}

int l6msg_get_field_unit_size_named(l6msg *lmsg, _const_char *key, int *size) 
{
    l6msg_field *field = NULL;
    int ret = get_field_named((l6msg_struct*)(*lmsg), key, &field);
    if(field) *size = field->elem_size;
    return ret;
}

int l6msg_get_field_size_bytes_at_index(l6msg *lmsg, int index, int *size)
{
    l6msg_field *field = NULL;
    int ret = get_field_at_index((l6msg_struct*)(*lmsg), index, &field);
    if(field) *size = field->elem_size * field->count;
    return ret;
}

int l6msg_get_field_size_bytes(l6msg *lmsg, int fid, int *size)
{
    l6msg_field *field = NULL;
    int ret = get_field_by_id((l6msg_struct*)(*lmsg), fid, &field);
    if(field) *size = field->elem_size * field->count;
    return ret;}

int l6msg_get_field_size_bytes_named(l6msg *lmsg, const char *key, int *size)
{
    l6msg_field *field = NULL;
    int ret = get_field_named((l6msg_struct*)(*lmsg), key, &field);
    if(field) *size = field->elem_size * field->count;
    return ret;
}

int l6msg_get_field_count_at_index(l6msg *lmsg, int index, int *count)
{
    l6msg_field *field = NULL;
    int ret = get_field_at_index((l6msg_struct*)(*lmsg), index, &field);
    if(field) *count = field->count;
    return ret;}


int l6msg_get_field_count(l6msg *lmsg, int fid, int *count)
{
    l6msg_field *field = NULL;
    int ret = get_field_by_id((l6msg_struct*)(*lmsg), fid, &field);
    if(field) *count = field->count;
    return ret;
}

int l6msg_get_field_count_named(l6msg *lmsg, const char *key, int *count)
{
    l6msg_field *field = NULL;
    int ret = get_field_named((l6msg_struct*)(*lmsg), key, &field);
    if(field) *count = field->count;
    return ret;
}

/*int get_field(l6msg_struct *msg, int index, l6msg_field **field)
{
    //allow negative indexing (i.e. from the end of the array)
    *field = l6_mvec_get(msg->qfields, index);
    if(*field == NULL)
    {
           msg->error_code = L6_ERR_FIELD_NOT_FOUND;
        return -1;    //field not found
    }
    return 0;
}*/

static int get_field_at_index(l6msg_struct *msg, int index, l6msg_field **field)
{
    if((index >= 0) && (index < l6_mvec_size(msg->qfields)))
    {
        *field = (l6msg_field*)l6_mvec_get(msg->qfields, index);
        return 0;
    }
    *field = NULL;
    msg->error_code = L6_ERR_FIELD_NOT_FOUND;
    return -1;    //field not found
}

static int get_field_by_id(l6msg_struct *msg, int fid, l6msg_field **field)
{
    if(msg->htbl_fld_id    /* if this is null, a field with id has never been put in in the first place */
        && l6_htbl_ikey_get(msg->htbl_fld_id, fid, (void**)field))
    {
        return 0;
    }
    *field = NULL;
    msg->error_code = L6_ERR_FIELD_NOT_FOUND;
    return -1;    //field not found
}

static int get_field_named(l6msg_struct *msg, const char *key, l6msg_field **field)
{
    if(msg->htbl_fld_name    /* if this is null, a field with a name has never been put in in the first place */
        && l6_htbl_skey_get(msg->htbl_fld_name, key, (void**)field))
    {
        return 0;
    }
    *field = NULL;
    msg->error_code = L6_ERR_FIELD_NOT_FOUND;
    return -1;    //field not found
}

//keep this static for now
static int l6msg_get_field_by_id(l6msg *lmsg, int fid, l6msg_field *field)
{
    l6msg_field **fld = (l6msg_field **)field;
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_by_id(msg, fid, fld);  //hope you got the right fonts <Dr-Evil-laugh/>
}

/*static int l6msg_get_field_named(l6msg *lmsg, const char *key, l6msg_field *field)
{
    l6msg_field **fld = (l6msg_field **)field;
    l6msg_struct *msg = (l6msg_struct*)(*lmsg);
    return get_field_named(msg, key, fld);
}*/

int l6msg_get_field_type_at_index(l6msg *lmsg, int index, int *type)
{
    l6msg_field *field = NULL;
    int ret = get_field_at_index((l6msg_struct*)(*lmsg), index, &field);
    if(field) *type = field->type;
    return ret;

}

int l6msg_get_field_type(l6msg *lmsg, int fid, int *type)
{
    l6msg_field *field = NULL;
    int ret = get_field_by_id((l6msg_struct*)(*lmsg), fid, &field);
    if(field) *type = field->type;
    return ret;
}

int l6msg_get_field_type_named(l6msg *lmsg, const char *key, int *type)
{
    l6msg_field *field = NULL;
    int ret = get_field_named((l6msg_struct*)(*lmsg), key, &field);
    if(field) *type = field->type;
    return ret;
}

int l6msg_is_field_array(l6msg *lmsg, int fid)
{
    l6msg_field *field = NULL;
    int ret = get_field_by_id((l6msg_struct*)(*lmsg), fid, &field);
    if(field) ret = (field->type & L6_FLAG_FIELD_IS_ARRAY);
    return ret;
}

int l6msg_is_field_array_named(l6msg *lmsg, const char *key)
{
    l6msg_field *field = NULL;
    int ret = get_field_named((l6msg_struct*)(*lmsg), key, &field);
    if(field) ret = (field->type & L6_FLAG_FIELD_IS_ARRAY);
    return ret;
}

int l6msg_is_field_array_at_index(l6msg *lmsg, int index)
{
    l6msg_field *field = NULL;
    int ret = get_field_at_index((l6msg_struct*)(*lmsg), index, &field);
    if(field) ret = (field->type & L6_FLAG_FIELD_IS_ARRAY);
    return ret;
}


