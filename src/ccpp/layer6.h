#ifndef LAYER6_H__
#define LAYER6_H__

#include <stdint.h>

#define L6_STATUS_OK                      0
#define L6_STATUS_ERROR                  -1
#define L6_STATUS_WARNING                 1

#define L6_DATATYPE_UNSIGNED_BYTE         1
#define L6_DATATYPE_UINT8                 1
#define L6_DATATYPE_BYTE                  2
#define L6_DATATYPE_INT8                  2
#define L6_DATATYPE_UINT16                3
#define L6_DATATYPE_UNSIGNED_SHORT        3
#define L6_DATATYPE_SHORT                 4
#define L6_DATATYPE_INT16                 4
#define L6_DATATYPE_UINT32                5
#define L6_DATATYPE_UNSIGNED_INT          5
#define L6_DATATYPE_INT                   6
#define L6_DATATYPE_INT32                 6
#define L6_DATATYPE_UINT64                7
#define L6_DATATYPE_UNSIGNED_LONG         7
#define L6_DATATYPE_INT64                 8
#define L6_DATATYPE_LONG                  8
#define L6_DATATYPE_FLOAT                 9
#define L6_DATATYPE_DOUBLE               10

#define L6_DATATYPE_STRING               16
#define L6_DATATYPE_UINT8_ARRAY          17
#define L6_DATATYPE_UNSIGNED_BYTES       17
#define L6_DATATYPE_BYTES                18
#define L6_DATATYPE_BYTE_ARRAY           18
#define L6_DATATYPE_INT8_ARRAY           18
#define L6_DATATYPE_UINT16_ARRAY         19
#define L6_DATATYPE_UNSIGNED_SHORT_ARRAY 19
#define L6_DATATYPE_INT16_ARRAY          20
#define L6_DATATYPE_SHORT_ARRAY          20
#define L6_DATATYPE_UINT32_ARRAY         21
#define L6_DATATYPE_UNSIGNED_INT_ARRAY   21
#define L6_DATATYPE_INT32_ARRAY          22
#define L6_DATATYPE_INT_ARRAY            22
#define L6_DATATYPE_UINT64_ARRAY         23
#define L6_DATATYPE_UNSIGNED_LONG_ARRAY  23
#define L6_DATATYPE_INT64_ARRAY          24
#define L6_DATATYPE_LONG_ARRAY           24
#define L6_DATATYPE_FLOAT_ARRAY          25
#define L6_DATATYPE_DOUBLE_ARRAY         26
#define L6_DATATYPE_L6MSG                27

#define L6_DATATYPE_UTF8_STRING          28
#define L6_DATATYPE_EXTENSION            32

//#define L6_DATATYPE_CUSTOM              31
//#define L6_DATATYPE_STRING_ARRAY        28
//#define L6_DATATYPE_BITFIELD            17

#define L6_ERR_MEM_ALLOC                 32
#define L6_ERR_INSUFF_BUFFER_SIZE        33
#define L6_ERR_INCORRECT_MSG_SIZE        34
#define L6_ERR_FIELD_EXISTS              35
#define L6_ERR_FIELD_NOT_FOUND           36
#define L6_ERR_FIELD_NAME_TOO_LONG       37
#define L6_ERR_FIELD_UNNAMED             38
#define L6_ERR_FIELD_NO_ID               39
#define L6_ERR_FIELD_ID_INVALID          40
#define L6_ERR_FIELD_TYPE_INCOMPATIBLE   41
#define L6_ERR_FIELD_MAX_COUNT_EXCEEDED  42
#define L6_ERR_FIELD_MAX_SIZE_EXCEEDED   43
#define L6_ERR_MAX_NUM_FIELDS_EXCEEDED   44
#define L6_ERR_MAX_MSG_SIZE_EXCEEDED     45
#define L6_ERR_MSG_LOCKED                46
#define L6_ERR_RECURSIVE_SUB_MSG         47
#define L6_ERR_SUB_MSG_DESERIALIZE       48
#define L6_ERR_OFFSET_OUT_OF_BOUNDS      49
#define L6_ERR_UNHANDLED_FIELD_TYPE      50
#define L6_ERR_NOT_IMPLEMENTED           51

#define L6_WARN_INDEX_OUT_OF_BOUNDS      53
#define L6_WARN_FIELD_REPLACED           54

#define L6_VERSION_NUM                   00

#define L6_MSG_ID_MAP_TYPE_INT            1
#define L6_MSG_ID_MAP_TYPE_STR            0
#define L6_MSG_MAP_INIT_CAP              11
#define L6_MSG_ID_MAP_INIT_CAP           11
#define L6_MSG_NAME_MAP_INIT_CAP         11
#define L6_MSG_LIST_INIT_CAP              8

#define L6_MSG_FIELD_NAME_MAX_LEN       128

#define L6_MSG_FIELD_ID_NONE             -1

#define L6_CHECK_MSG_LOCKS               1
/*
//It's 2009, we probably don't need all this
#ifdef WIN32
#define DllExport extern "C" __declspec( dllexport )
#else
#define DllExport
#define __cdecl
#endif
*/

#ifdef WIN32
typedef __int32             int32_t;
typedef unsigned __int32    uint32_t; 
typedef __int16             int16_t;
typedef unsigned __int16    uint16_t; 
typedef __int64             int64_t;
typedef unsigned __int64    uint64_t; 
typedef __int8              int8_t;
typedef unsigned __int8     uint8_t;
#endif

typedef void* l6msg;
typedef void* l6msg_t;

#ifdef __cplusplus
extern "C" {
#endif

int l6msg_init                      (l6msg *msg);
int l6msg_reset                     (l6msg *msg);
int l6msg_free                      (l6msg *msg);

int l6msg_set_deep_copy_default     (l6msg *msg, int mode);
int l6msg_get_deep_copy_default     (l6msg *msg);

int l6msg_set_auto_byte_order_mode  (l6msg *msg, int byteorder);
int l6msg_get_auto_byte_order_mode  (l6msg *msg);

int l6msg_set_deserialize_lazily    (l6msg *msg, int dsoa);
int l6msg_get_deserialize_lazily    (l6msg *msg);

int l6msg_set_check_cyclic_submsg   (l6msg *msg, int chk);
int l6msg_get_check_cyclic_submsg   (l6msg *msg);

int l6msg_get_error_code            (l6msg *msg);

const char* l6msg_get_error_str(l6msg *msg);

const char* l6msg_get_debug_info(l6msg *msg);

const char* l6_get_error_str_for_code(int e);

//convenience allocate+initialize method
l6msg l6msg_alloc();

//Not yet (and may never be) supported
void l6msg_set_code                 (l6msg *msg, int code);
int  l6msg_get_code                 (l6msg *msg);

/*
DllExport void     __cdecl l6msg_setSubject(l6msg *msg, const char *sub);
DllExport const char* const    __cdecl l6msg_getSubject(l6msg *msg);

void __cdecl l6msg_setChar(l6msg *msg, char);
*/
//Add fields
//Scalars
int l6msg_add_byte                      (l6msg *msg, char            data);
int l6msg_add_int16                     (l6msg *msg, int16_t         data);
int l6msg_add_short                     (l6msg *msg, short           data);
int l6msg_add_int32                     (l6msg *msg, int32_t         data);
int l6msg_add_int                       (l6msg *msg, int             data);
int l6msg_add_int64                     (l6msg *msg, int64_t         data);
int l6msg_add_long                      (l6msg *msg, long long int   data);
int l6msg_add_float                     (l6msg *msg, float           data);
int l6msg_add_double                    (l6msg *msg, double          data);
//Arrays
int l6msg_add_string                    (l6msg *msg, char            *data);
int l6msg_add_byte_array                (l6msg *msg, char            *data, int size);
int l6msg_add_int16_array               (l6msg *msg, int16_t         *data, int count);
int l6msg_add_short_array               (l6msg *msg, short           *data, int count);
int l6msg_add_int32_array               (l6msg *msg, int32_t         *data, int count);
int l6msg_add_int_array                 (l6msg *msg, int             *data, int count);
int l6msg_add_int64_array               (l6msg *msg, int64_t         *data, int count);
int l6msg_add_long_array                (l6msg *msg, long long int   *data, int count);
int l6msg_add_float_array               (l6msg *msg, float           *data, int count);
int l6msg_add_double_array              (l6msg *msg, double          *data, int count);
int l6msg_add_layer6_msg                (l6msg *msg, l6msg           *submsg);
//int l6msg_addStringArray(l6msg *msg, char**  data, int);

//Array Pointers
int l6msg_add_string_ptr                (l6msg *msg, char           *data);
int l6msg_add_byte_array_ptr            (l6msg *msg, char           *data, int size);
int l6msg_add_int16_array_ptr           (l6msg *msg, int16_t        *data, int count);
int l6msg_add_short_array_ptr           (l6msg *msg, short          *data, int count);
int l6msg_add_int32_array_ptr           (l6msg *msg, int32_t        *data, int count);
int l6msg_add_int_array_ptr             (l6msg *msg, int            *data, int count);
int l6msg_add_int64_array_ptr           (l6msg *msg, int64_t        *data, int count);
int l6msg_add_long_array_ptr            (l6msg *msg, long long int  *data, int count);
int l6msg_add_float_array_ptr           (l6msg *msg, float          *data, int count);
int l6msg_add_double_array_ptr          (l6msg *msg, double         *data, int count);
int l6msg_add_layer6_msg_ptr            (l6msg *msg, l6msg           *submsg);

//Scalars
int l6msg_set_byte                      (l6msg *msg, int fid, char            data);
int l6msg_set_int16                     (l6msg *msg, int fid, int16_t         data);
int l6msg_set_short                     (l6msg *msg, int fid, short           data);
int l6msg_set_int32                     (l6msg *msg, int fid, int32_t         data);
int l6msg_set_int                       (l6msg *msg, int fid, int             data);
int l6msg_set_int64                     (l6msg *msg, int fid, int64_t         data);
int l6msg_set_long                      (l6msg *msg, int fid, long long int   data);
int l6msg_set_float                     (l6msg *msg, int fid, float           data);
int l6msg_set_double                    (l6msg *msg, int fid, double          data);

//Arrays
int l6msg_set_string                    (l6msg *msg, int fid,   const char      *data);
int l6msg_set_byte_array                (l6msg *msg, int fid,   char            *data, int size);
int l6msg_set_int16_array               (l6msg *msg, int fid,   int16_t         *data, int count);
int l6msg_set_short_array               (l6msg *msg, int fid,   short           *data, int count);
int l6msg_set_int32_array               (l6msg *msg, int fid,   int32_t         *data, int count);
int l6msg_set_int_array                 (l6msg *msg, int fid,   int             *data, int count);
int l6msg_set_int64_array               (l6msg *msg, int fid,   int64_t         *data, int count);
int l6msg_set_long_array                (l6msg *msg, int fid,   long long int   *data, int count);
int l6msg_set_float_array               (l6msg *msg, int fid,   float           *data, int count);
int l6msg_set_double_array              (l6msg *msg, int fid,   double          *data, int count);
int l6msg_set_layer6_msg                (l6msg *msg, int fid,   l6msg           *submsg);
//int l6msg_setStringArray(l6msg *msg, char**  data, int);

//Array Pointers
int l6msg_set_string_ptr                (l6msg *msg, int fid,    const char     *data);
int l6msg_set_byte_array_ptr            (l6msg *msg, int fid,    char           *data, int size);
int l6msg_set_int16_array_ptr           (l6msg *msg, int fid,    int16_t        *data, int count);
int l6msg_set_short_array_ptr           (l6msg *msg, int fid,    short          *data, int count);
int l6msg_set_int32_array_ptr           (l6msg *msg, int fid,    int32_t        *data, int count);
int l6msg_set_int_array_ptr             (l6msg *msg, int fid,    int            *data, int count);
int l6msg_set_int64_array_ptr           (l6msg *msg, int fid,    int64_t        *data, int count);
int l6msg_set_long_array_ptr            (l6msg *msg, int fid,    long long int  *data, int count);
int l6msg_set_float_array_ptr           (l6msg *msg, int fid,    float          *data, int count);
int l6msg_set_double_array_ptr          (l6msg *msg, int fid,    double         *data, int count);
int l6msg_set_layer6_msg_ptr            (l6msg *msg, int fid,   l6msg           *submsg);

//Scalars By Name
int l6msg_set_byte_named                (l6msg *msg, const char *key, char            data);
int l6msg_set_int16_named               (l6msg *msg, const char *key, int16_t         data);
int l6msg_set_short_named               (l6msg *msg, const char *key, short           data);
int l6msg_set_int32_named               (l6msg *msg, const char *key, int32_t         data);
int l6msg_set_int_named                 (l6msg *msg, const char *key, int             data);
int l6msg_set_int64_named               (l6msg *msg, const char *key, int64_t         data);
int l6msg_set_long_named                (l6msg *msg, const char *key, long long int   data);
int l6msg_set_float_named               (l6msg *msg, const char *key, float           data);
int l6msg_set_double_named              (l6msg *msg, const char *key, double          data);

//Arrays by name
int l6msg_set_string_named              (l6msg *msg, const char *key,  const char     *data);
int l6msg_set_byte_array_named          (l6msg *msg, const char *key,  char           *data, int size);
int l6msg_set_int16_array_named         (l6msg *msg, const char *key,  int16_t        *data, int count);
int l6msg_set_short_array_named         (l6msg *msg, const char *key,  short          *data, int count);
int l6msg_set_int32_array_named         (l6msg *msg, const char *key,  int32_t        *data, int count);
int l6msg_set_int_array_named           (l6msg *msg, const char *key,  int            *data, int count);
int l6msg_set_int64_array_named         (l6msg *msg, const char *key,  int64_t        *data, int count);
int l6msg_set_long_array_named          (l6msg *msg, const char *key,  long long int  *data, int count);
int l6msg_set_float_array_named         (l6msg *msg, const char *key,  float          *data, int count);
int l6msg_set_double_array_named        (l6msg *msg, const char *key,  double         *data, int count);
int l6msg_set_layer6_msg_named          (l6msg *msg, const char *key,  l6msg          *submsg);

//Array Pointers by name
int l6msg_set_string_ptr_named          (l6msg *msg, const char *key,  const char     *data);
int l6msg_set_byte_array_ptr_named      (l6msg *msg, const char *key,  char           *data, int size);
int l6msg_set_int16_array_ptr_named     (l6msg *msg, const char *key,  int16_t        *data, int count);
int l6msg_set_short_array_ptr_named     (l6msg *msg, const char *key,  short          *data, int count);
int l6msg_set_int32_array_ptr_named     (l6msg *msg, const char *key,  int32_t        *data, int count);
int l6msg_set_int_array_ptr_named       (l6msg *msg, const char *key,  int            *data, int count);
int l6msg_set_int64_array_ptr_named     (l6msg *msg, const char *key,  int64_t        *data, int count);
int l6msg_set_long_array_ptr_named      (l6msg *msg, const char *key,  long long int  *data, int count);
int l6msg_set_float_array_ptr_named     (l6msg *msg, const char *key,  float          *data, int count);
int l6msg_set_double_array_ptr_named    (l6msg *msg, const char *key,  double         *data, int count);
int l6msg_set_layer6_msg_ptr_named      (l6msg *msg, const char *key,  l6msg          *submsg);

//By index
//Scalars
int l6msg_set_byte_at_index             (l6msg *msg, int index, char            data);
int l6msg_set_int16_at_index            (l6msg *msg, int index, int16_t         data);
int l6msg_set_short_at_index            (l6msg *msg, int index, short           data);
int l6msg_set_int32_at_index            (l6msg *msg, int index, int32_t         data);
int l6msg_set_int_at_index              (l6msg *msg, int index, int             data);
int l6msg_set_int64_at_index            (l6msg *msg, int index, int64_t         data);
int l6msg_set_long_at_index             (l6msg *msg, int index, long long int   data);
int l6msg_set_float_at_index            (l6msg *msg, int index, float           data);
int l6msg_set_double_at_index           (l6msg *msg, int index, double          data);

//Arrays
int l6msg_set_string_at_index           (l6msg *msg, int index, const char      *data);
int l6msg_set_byte_array_at_index       (l6msg *msg, int index, char            *data, int size);
int l6msg_set_int16_array_at_index      (l6msg *msg, int index, int16_t         *data, int count);
int l6msg_set_short_array_at_index      (l6msg *msg, int index, short           *data, int count);
int l6msg_set_int32_array_at_index      (l6msg *msg, int index, int32_t         *data, int count);
int l6msg_set_int_array_at_index        (l6msg *msg, int index, int             *data, int count);
int l6msg_set_int64_array_at_index      (l6msg *msg, int index, int64_t         *data, int count);
int l6msg_set_long_array_at_index       (l6msg *msg, int index, long long int   *data, int count);
int l6msg_set_float_array_at_index      (l6msg *msg, int index, float           *data, int count);
int l6msg_set_double_array_at_index     (l6msg *msg, int index, double          *data, int count);
int l6msg_set_layer6_msg_at_index       (l6msg *msg, int index, l6msg           *submsg);
//int l6msg_setStringArray(l6msg *msg, char**  data, int);

//Array Pointers
int l6msg_set_string_ptr_at_index       (l6msg *msg, int index, const char     *data);
int l6msg_set_byte_array_ptr_at_index   (l6msg *msg, int index, char           *data, int size);
int l6msg_set_int16_array_ptr_at_index  (l6msg *msg, int index, int16_t        *data, int count);
int l6msg_set_short_array_ptr_at_index  (l6msg *msg, int index, short          *data, int count);
int l6msg_set_int32_array_ptr_at_index  (l6msg *msg, int index, int32_t        *data, int count);
int l6msg_set_int_array_ptr_at_index    (l6msg *msg, int index, int            *data, int count);
int l6msg_set_int64_array_ptr_at_index  (l6msg *msg, int index, int64_t        *data, int count);
int l6msg_set_long_array_ptr_at_index   (l6msg *msg, int index, long long int  *data, int count);
int l6msg_set_float_array_ptr_at_index  (l6msg *msg, int index, float          *data, int count);
int l6msg_set_double_array_ptr_at_index (l6msg *msg, int index, double         *data, int count);
int l6msg_set_layer6_msg_ptr_at_index   (l6msg *msg, int index, l6msg           *submsg);


//Get
//int __cdecl l6msg_getChar(l6msg *msg, int, char*);
//Get Scalars
int l6msg_get_byte                      (l6msg *msg, int fid,     char           *data);
int l6msg_get_int16                     (l6msg *msg, int fid,     int16_t        *data);
int l6msg_get_short                     (l6msg *msg, int fid,     short          *data);
int l6msg_get_int32                     (l6msg *msg, int fid,     int32_t        *data);
int l6msg_get_int                       (l6msg *msg, int fid,     int            *data);
int l6msg_get_int64                     (l6msg *msg, int fid,     int64_t        *data);
int l6msg_get_long                      (l6msg *msg, int fid,     long long int  *data);
int l6msg_get_float                     (l6msg *msg, int fid,     float          *data);
int l6msg_get_double                    (l6msg *msg, int fid,     double         *data);
int l6msg_get_string                    (l6msg *msg, int fid,     char           *data, int len);

//Get Scalars by index
int l6msg_get_byte_at_index             (l6msg *msg, int index,   char           *data);
int l6msg_get_int16_at_index            (l6msg *msg, int index,   int16_t        *data);
int l6msg_get_short_at_index            (l6msg *msg, int index,   short          *data);
int l6msg_get_int32_at_index            (l6msg *msg, int index,   int32_t        *data);
int l6msg_get_int_at_index              (l6msg *msg, int index,   int            *data);
int l6msg_get_int64_at_index            (l6msg *msg, int index,   int64_t        *data);
int l6msg_get_long_at_index             (l6msg *msg, int index,   long long int  *data);
int l6msg_get_float_at_index            (l6msg *msg, int index,   float          *data);
int l6msg_get_double_at_index           (l6msg *msg, int index,   double         *data);
int l6msg_get_string_at_index           (l6msg *msg, int index,   char           *data, int len);

//Get Scalars by name
int l6msg_get_byte_named                (l6msg *msg, const char *key,   char          *data);
int l6msg_get_int16_named               (l6msg *msg, const char *key,   int16_t       *data);
int l6msg_get_short_named               (l6msg *msg, const char *key,   short         *data);
int l6msg_get_int32_named               (l6msg *msg, const char *key,   int32_t       *data);
int l6msg_get_int_named                 (l6msg *msg, const char *key,   int           *data);
int l6msg_get_int64_named               (l6msg *msg, const char *key,   int64_t       *data);
int l6msg_get_long_named                (l6msg *msg, const char *key,   long long int *data);
int l6msg_get_float_named               (l6msg *msg, const char *key,   float         *data);
int l6msg_get_double_named              (l6msg *msg, const char *key,   double        *data);
int l6msg_get_string_named              (l6msg *msg, const char *key,   char          *data, int len);

// Get Arrays
int l6msg_get_byte_array                (l6msg *msg, int fid,     char          *data, int offset, int count);
int l6msg_get_int16_array               (l6msg *msg, int fid,     int16_t       *data, int offset, int count);
int l6msg_get_short_array               (l6msg *msg, int fid,     short         *data, int offset, int count);
int l6msg_get_int32_array               (l6msg *msg, int fid,     int32_t       *data, int offset, int count);
int l6msg_get_int_array                 (l6msg *msg, int fid,     int           *data, int offset, int count);
int l6msg_get_int64_array               (l6msg *msg, int fid,     int64_t       *data, int offset, int count);
int l6msg_get_long_array                (l6msg *msg, int fid,     long long int *data, int offset, int count);
int l6msg_get_float_array               (l6msg *msg, int fid,     float         *data, int offset, int count);
int l6msg_get_double_array              (l6msg *msg, int fid,     double        *data, int offset, int count);
int l6msg_get_layer6_msg                (l6msg *msg, int fid,     l6msg         *submsg);

// Get Arrays By index
int l6msg_get_byte_array_at_index       (l6msg *msg, int index,   char          *data, int offset, int count);
int l6msg_get_int16_array_at_index      (l6msg *msg, int index,   int16_t       *data, int offset, int count);
int l6msg_get_short_array_at_index      (l6msg *msg, int index,   short         *data, int offset, int count);
int l6msg_get_int32_array_at_index      (l6msg *msg, int index,   int32_t       *data, int offset, int count);
int l6msg_get_int_array_at_index        (l6msg *msg, int index,   int           *data, int offset, int count);
int l6msg_get_int64_array_at_index      (l6msg *msg, int index,   int64_t       *data, int offset, int count);
int l6msg_get_long_array_at_index       (l6msg *msg, int index,   long long int *data, int offset, int count);
int l6msg_get_float_array_at_index      (l6msg *msg, int index,   float         *data, int offset, int count);
int l6msg_get_double_array_at_index     (l6msg *msg, int index,   double        *data, int offset, int count);
int l6msg_get_layer6_msg_at_index       (l6msg *msg, int index,   l6msg         *submsg);

//Get Arrays By Name
int l6msg_get_byte_array_named          (l6msg *msg, const char *key,   char          *data, int offset, int size);
int l6msg_get_int16_array_named         (l6msg *msg, const char *key,   int16_t       *data, int offset, int count);
int l6msg_get_short_array_named         (l6msg *msg, const char *key,   short         *data, int offset, int count);
int l6msg_get_int32_array_named         (l6msg *msg, const char *key,   int32_t       *data, int offset, int count);
int l6msg_get_int_array_named           (l6msg *msg, const char *key,   int           *data, int offset, int count);
int l6msg_get_int64_array_named         (l6msg *msg, const char *key,   int64_t       *data, int offset, int count);
int l6msg_get_long_array_named          (l6msg *msg, const char *key,   long long int *data, int offset, int count);
int l6msg_get_float_array_named         (l6msg *msg, const char *key,   float         *data, int offset, int count);
int l6msg_get_double_array_named        (l6msg *msg, const char *key,   double        *data, int offset, int count);
int l6msg_get_layer6_msg_named          (l6msg *msg, const char *key,   l6msg         *submsg);

//Get Array Pointers
int l6msg_get_string_ptr                (l6msg *msg, int fid,     const char    **data);
int l6msg_get_byte_array_ptr            (l6msg *msg, int fid,     char          **data, int *size);
int l6msg_get_int16_array_ptr           (l6msg *msg, int fid,     int16_t       **data, int *count);
int l6msg_get_short_array_ptr           (l6msg *msg, int fid,     short         **data, int *count);
int l6msg_get_int32_array_ptr           (l6msg *msg, int fid,     int32_t       **data, int *count);
int l6msg_get_int_array_ptr             (l6msg *msg, int fid,     int           **data, int *count);
int l6msg_get_int64_array_ptr           (l6msg *msg, int fid,     int64_t       **data, int *count);
int l6msg_get_long_array_ptr            (l6msg *msg, int fid,     long long int **data, int *count);
int l6msg_get_float_array_ptr           (l6msg *msg, int fid,     float         **data, int *count);
int l6msg_get_double_array_ptr          (l6msg *msg, int fid,     double        **data, int *count);
int l6msg_get_layer6_msg_ptr            (l6msg *msg, int fid,     l6msg         *submsg);

//Get Array Pointers by index
int l6msg_get_string_ptr_at_index       (l6msg *msg, int index,   const char    **data);
int l6msg_get_byte_array_ptr_at_index   (l6msg *msg, int index,   char          **data, int *size);
int l6msg_get_int16_array_ptr_at_index  (l6msg *msg, int index,   int16_t       **data, int *count);
int l6msg_get_short_array_ptr_at_index  (l6msg *msg, int index,   short         **data, int *count);
int l6msg_get_int32_array_ptr_at_index  (l6msg *msg, int index,   int32_t       **data, int *count);
int l6msg_get_int_array_ptr_at_index    (l6msg *msg, int index,   int           **data, int *count);
int l6msg_get_int64_array_ptr_at_index  (l6msg *msg, int index,   int64_t       **data, int *count);
int l6msg_get_long_array_ptr_at_index   (l6msg *msg, int index,   long long int **data, int *count);
int l6msg_get_float_array_ptr_at_index  (l6msg *msg, int index,   float         **data, int *count);
int l6msg_get_double_array_ptr_at_index (l6msg *msg, int index,   double        **data, int *count);
int l6msg_get_layer6_msg_ptr_at_index   (l6msg *msg, int index,   l6msg         *submsg);

//Get Array Pointers by name
int l6msg_get_string_ptr_named          (l6msg *msg, const char *key,   const char    **data);
int l6msg_get_byte_array_ptr_named      (l6msg *msg, const char *key,   char          **data, int *size);
int l6msg_get_int16_array_ptr_named     (l6msg *msg, const char *key,   int16_t       **data, int *count);
int l6msg_get_short_array_ptr_named     (l6msg *msg, const char *key,   short         **data, int *count);
int l6msg_get_int32_array_ptr_named     (l6msg *msg, const char *key,   int32_t       **data, int *count);
int l6msg_get_int_array_ptr_named       (l6msg *msg, const char *key,   int           **data, int *count);
int l6msg_get_int64_array_ptr_named     (l6msg *msg, const char *key,   int64_t       **data, int *count);
int l6msg_get_long_array_ptr_named      (l6msg *msg, const char *key,   long long int **data, int *count);
int l6msg_get_float_array_ptr_named     (l6msg *msg, const char *key,   float         **data, int *count);
int l6msg_get_double_array_ptr_named    (l6msg *msg, const char *key,   double        **data, int *count);
int l6msg_get_layer6_msg_ptr_named      (l6msg *msg, const char *key,   l6msg         *submsg);

int l6msg_size                          (l6msg *msg);

int l6msg_min_buffer_size               (l6msg *msg);
//alias
#define l6msg_get_size                  l6msg_size

int l6msg_serialize                     (l6msg *msg, char *data,    int length, int *left);
int l6msg_serialize_header              (l6msg *msg, char *data,    int length, int *left);
int l6msg_serialize_metadata            (l6msg *msg, char *data,    int length, int *left);
int l6msg_serialize_data                (l6msg *msg, char *data,    int length, int *left);

int l6msg_deserialize                   (l6msg *msg, char *buffer,  int length, int *left);
int l6msg_deserialize_copy              (l6msg *msg, char *buffer,  int length, int *left);
int l6msg_deserialize_in_place          (l6msg *msg, char *buffer,  int length, int *left);

int l6msg_deserialize_header            (l6msg *msg, char *data,    int length, int *left);
int l6msg_deserialize_metadata          (l6msg *msg, char *data,    int length, int *left);
int l6msg_deserialize_data              (l6msg *msg, char *data,    int length, int *left);

//convenience methods to directly serialize to / deserialize from a file descriptor / socket
//int l6msg_serialize_to_fd             (l6msg *msg, int fd, int *left);
//int l6msg_deserialize_from_fd         (l6msg *msg, int fd, int *left);

//field ops
int l6msg_get_num_fields                (l6msg *msg);

int l6msg_remove_field                  (l6msg *msg, int fid);
int l6msg_remove_field_at_index         (l6msg *msg, int index);
int l6msg_remove_field_named            (l6msg *msg, const char *key);

int l6msg_get_field_size_bytes          (l6msg *msg, int fid,           int *size);
int l6msg_get_field_size_bytes_named    (l6msg *msg, const char *key,   int *size);
int l6msg_get_field_size_bytes_at_index (l6msg *msg, int index,       int *size);

int l6msg_get_field_count               (l6msg *msg, int fid,           int *count);
int l6msg_get_field_count_named         (l6msg *msg, const char *key,   int *count);
int l6msg_get_field_count_at_index      (l6msg *msg, int index,         int *count);
int l6msg_get_field_unit_size           (l6msg *msg, int fid,           int *size);
int l6msg_get_field_unit_size_named     (l6msg *msg, const char *key,   int *size);
int l6msg_get_field_unit_size_at_index  (l6msg *msg, int index,         int *size);

int l6msg_get_field_type                (l6msg *msg, int fid,           int *type);
int l6msg_get_field_type_named          (l6msg *msg, const char *key,         int *type);
int l6msg_get_field_type_at_index       (l6msg *msg, int index,         int *type);

int l6msg_get_field_id_at_index         (l6msg *msg, int index,         int *fid);
int l6msg_get_field_name_at_index       (l6msg *msg, int index,   const char **name);
int l6msg_get_field_name                (l6msg *msg, int fid,     const char **name);
int l6msg_is_field_named                (l6msg *msg, int fid);
int l6msg_is_field_named_at_index       (l6msg *msg, int index);

int l6msg_is_field_array                (l6msg *msg, int fid);
int l6msg_is_field_array_named          (l6msg *msg, const char *key);
int l6msg_is_field_array_at_index       (l6msg *msg, int index);

int l6msg_dup                           (l6msg *lmsrc, l6msg *lmdst);

int l6msg_deserialize_template          (l6msg *msg, int template_id);
int l6msg_set_templated                 (l6msg *msg, int template_id);
int l6msg_is_templated                  (l6msg *msg);

//convenience variadic functions to get/set multiple fields in a single method call
int l6msg_setf                          (l6msg *msg, char *fmt, ...);
int l6msg_getf                          (l6msg *msg, char *fmt, ...);

//hooks
void l6_override_hashtable_methods (int (*ovrr_htbl_init)       (void *vpmap, int type, int cap), 
                                    int (*ovrr_htbl_free)       (void *pmap),
                                    int (*ovrr_htbl_put)        (void *ph, int32_t fid,     void *field),
                                    int (*ovrr_htbl_get)        (void *ph, int32_t fid,     void **field),
                                    int (*ovrr_htbl_remove)     (void *ph, int32_t fid,     void **field),
                                    int (*ovrr_htbl_str_put)    (void *ph, const char *key, void *field),
                                    int (*ovrr_htbl_str_get)    (void *ph, const char *key, void **field),
                                    int (*ovrr_htbl_str_remove) (void *ph, const char *key, void **field));

void l6_override_vector_methods    (int   (*ovrr_mvec_init)         (void **q, int elem_size, int start),
                                    int   (*ovrr_mvec_free)         (void *q),
                                    int   (*ovrr_mvec_resize)       (void *q, float factor),
                                    int   (*ovrr_mvec_push)         (void *q, void  *field),
                                    int   (*ovrr_mvec_size)         (void *q),
                                    void *(*ovrr_mvec_get)          (void *q, int index),
                                    void *(*ovrr_mvec_remove)       (void *q, int index),
                                    int   (*ovrr_mvec_index_of)     (void *q, void *field),
                                    int   (*ovrr_mvec_remove_item)  (void *q, void *field),
                                    void *(*ovrr_mvec_pop)          (void *q));

// ******************* aliases (using #defines) for Camel-case API
#define L6Msg                          l6msg
#define L6MsgInit                      l6msg_init
#define L6MsgAlloc                     l6msg_alloc
#define L6MsgReset                     l6msg_reset
#define L6MsgFree                      l6msg_free

#define L6MsgSetDeserializeLazily      l6msg_set_deserialize_lazily
#define L6MsgGetDeserializeLazily      l6msg_get_deserialize_lazily

#define L6MsgSetCheckCyclicSubMsg      l6msg_set_check_cyclic_submsg
#define L6MsgGetCheckCyclicSubMsg      l6msg_get_check_cyclic_submsg

#define L6MsgSetDeepCopyDefault        l6msg_set_deep_copy_default
#define L6MsgGetDeepCopyDefault        l6msg_get_deep_copy_default
#define L6MsgSetAutoByteOrderMode      l6msg_set_auto_byte_order_mode
#define L6MsgGetAutoByteOrderMode      l6msg_get_auto_byte_order_mode

#define L6MsgGetErrorCode              l6msg_get_error_code
#define L6MsgGetErrorStr               l6msg_get_error_str

#define L6MsgGetDebugInfo              l6msg_get_debug_info

#define L6MsgSetCode                   l6msg_set_code
#define L6MsgGetCode                   l6msg_get_code

/*
#define L6MsgSetSubject                l6msg_set_subject
#define L6MsgGetSubject                l6msg_get_subject
#define L6MsgSetChar;                  l6msg_set_char
*/
//Scalars
#define L6MsgSetInt16                  l6msg_set_int16
#define L6MsgSetShort                  l6msg_set_short
#define L6MsgSetInt32                  l6msg_set_int32
#define L6MsgSetInt                    l6msg_set_int
#define L6MsgSetInt64                  l6msg_set_int64
#define L6MsgSetLong                   l6msg_set_long
#define L6MsgSetFloat                  l6msg_set_float
#define L6MsgSetDouble                 l6msg_set_double

//Scalars By Name
#define L6MsgSetInt16Named             l6msg_set_int16_named
#define L6MsgSetShortNamed             l6msg_set_short_named
#define L6MsgSetInt32Named             l6msg_set_int32_named
#define L6MsgSetIntNamed               l6msg_set_int_named
#define L6MsgSetInt64Named             l6msg_set_int64_named
#define L6MsgSetLongNamed              l6msg_set_long_named
#define L6MsgSetFloatNamed             l6msg_set_float_named
#define L6MsgSetDoubleNamed            l6msg_set_double_named

//Arrays
#define L6MsgSetString                 l6msg_set_string
#define L6MsgSetByteArray              l6msg_set_byte_array
#define L6MsgSetInt16Array             l6msg_set_int16_array
#define L6MsgSetShortArray             l6msg_set_short_array
#define L6MsgSetInt32Array             l6msg_set_int32_array
#define L6MsgSetIntArray               l6msg_set_int_array
#define L6MsgSetInt64Array             l6msg_set_int64_array
#define L6MsgSetLongArray              l6msg_set_long_array
#define L6MsgSetFloatArray             l6msg_set_float_array
#define L6MsgSetDoubleArray            l6msg_set_double_array
#define L6MsgSetLayer6Msg              l6msg_set_layer6_msg

//Array Pointers
#define L6MsgSetStringPtr              l6msg_set_string_ptr
#define L6MsgSetByteArrayPtr           l6msg_set_byte_array_ptr
#define L6MsgSetInt16ArrayPtr          l6msg_set_int16_array_ptr
#define L6MsgSetShortArrayPtr          l6msg_set_short_array_ptr
#define L6MsgSetInt32ArrayPtr          l6msg_set_int32_array_ptr
#define L6MsgSetIntArrayPtr            l6msg_set_int_array_ptr
#define L6MsgSetInt64ArrayPtr          l6msg_set_int64_array_ptr
#define L6MsgSetLongArrayPtr           l6msg_set_long_array_ptr
#define L6MsgSetFloatArrayPtr          l6msg_set_float_array_ptr
#define L6MsgSetDoubleArrayPtr         l6msg_set_double_array_ptr

//Arrays by name
#define L6MsgSetStringNamed            l6msg_set_string_named
#define L6MsgSetByteArrayNamed         l6msg_set_byte_array_named
#define L6MsgSetInt16ArrayNamed        l6msg_set_int16_array_named
#define L6MsgSetShortArrayNamed        l6msg_set_short_array_named
#define L6MsgSetInt32ArrayNamed        l6msg_set_int32_array_named
#define L6MsgSetIntArrayNamed          l6msg_set_int_array_named
#define L6MsgSetInt64ArrayNamed        l6msg_set_int64_array_named
#define L6MsgSetLongArrayNamed         l6msg_set_long_array_named
#define L6MsgSetFloatArrayNamed        l6msg_set_float_array_named
#define L6MsgSetDoubleArrayNamed       l6msg_set_double_array_named
#define L6MsgSetLayer6MsgNamed         l6msg_set_layer6_msg_named

//Array Pointers by name
#define L6MsgSetStringPtrNamed         l6msg_set_string_ptr_named
#define L6MsgSetByteArrayPtrNamed      l6msg_set_byte_array_ptr_named
#define L6MsgSetInt16ArrayPtrNamed     l6msg_set_int16_array_ptr_named
#define L6MsgSetShortArrayPtrNamed     l6msg_set_short_array_ptr_named
#define L6MsgSetInt32ArrayPtrNamed     l6msg_set_int32_array_ptr_named
#define L6MsgSetIntArrayPtrNamed       l6msg_set_int_array_ptr_named
#define L6MsgSetInt64ArrayPtrNamed     l6msg_set_int64_array_ptr_named
#define L6MsgSetLongArrayPtrNamed      l6msg_set_long_array_ptr_named
#define L6MsgSetFloatArrayPtrNamed     l6msg_set_float_array_ptr_named
#define L6MsgSetDoubleArrayPtrNamed    l6msg_set_double_array_ptr_named

//Get
//int __cdecl l6msg_getChar
//Get Scalars
#define L6MsgGetInt16                  l6msg_get_int16
#define L6MsgGetShort                  l6msg_get_short
#define L6MsgGetInt32                  l6msg_get_int32
#define L6MsgGetInt                    l6msg_get_int
#define L6MsgGetInt64                  l6msg_get_int64
#define L6MsgGetLong                   l6msg_get_long
#define L6MsgGetFloat                  l6msg_get_float
#define L6MsgGetDouble                 l6msg_get_double
#define L6MsgGetString                 l6msg_get_string

//Get Scalars by name
#define L6MsgGetInt16Named             l6msg_get_int16_named
#define L6MsgGetShortNamed             l6msg_get_short_named
#define L6MsgGetInt32Named             l6msg_get_int32_named
#define L6MsgGetIntNamed               l6msg_get_int_named
#define L6MsgGetInt64Named             l6msg_get_int64_named
#define L6MsgGetLongNamed              l6msg_get_long_named
#define L6MsgGetFloatNamed             l6msg_get_float_named
#define L6MsgGetDoubleNamed            l6msg_get_double_named
#define L6MsgGetStringNamed            l6msg_get_string_named

// Get Arrays
#define L6MsgGetByteArray              l6msg_get_byte_array
#define L6MsgGetInt16Array             l6msg_get_int16_array
#define L6MsgGetShortArray             l6msg_get_short_array
#define L6MsgGetInt32Array             l6msg_get_int32_array
#define L6MsgGetIntArray               l6msg_get_int_array
#define L6MsgGetInt64Array             l6msg_get_int64_array
#define L6MsgGetLongArray              l6msg_get_long_array
#define L6MsgGetFloatArray             l6msg_get_float_array
#define L6MsgGetDoubleArray            l6msg_get_double_array
#define L6MsgGetLayer6Msg              l6msg_get_layer6msg

//Get Arrays By Name
#define L6MsgGetByteArrayNamed         l6msg_get_byte_array_named
#define L6MsgGetInt16ArrayNamed        l6msg_get_int16_array_named
#define L6MsgGetShortArrayNamed        l6msg_get_short_array_named
#define L6MsgGetInt32ArrayNamed        l6msg_get_int32_array_named
#define L6MsgGetIntArrayNamed          l6msg_get_int_array_named
#define L6MsgGetInt64ArrayNamed        l6msg_get_int64_array_named
#define L6MsgGetLongArrayNamed         l6msg_get_long_array_named
#define L6MsgGetFloatArrayNamed        l6msg_get_float_array_named
#define L6MsgGetDoubleArrayNamed       l6msg_get_double_array_named
#define L6MsgGetLayer6MsgNamed         l6msg_get_layer6msg_named

//Get Array Pointers
#define L6MsgGetStringPtr              l6msg_get_string_ptr
#define L6MsgGetByteArrayPtr           l6msg_get_byte_array_ptr
#define L6MsgGetInt16ArrayPtr          l6msg_get_int16_array_ptr
#define L6MsgGetShortArrayPtr          l6msg_get_short_array_ptr
#define L6MsgGetInt32ArrayPtr          l6msg_get_int32_array_ptr
#define L6MsgGetIntArrayPtr            l6msg_get_int_array_ptr
#define L6MsgGetInt64ArrayPtr          l6msg_get_int64_array_ptr
#define L6MsgGetLongArrayPtr           l6msg_get_long_array_ptr
#define L6MsgGetFloatArrayPtr          l6msg_get_float_array_ptr
#define L6MsgGetDoubleArrayPtr         l6msg_get_double_array_ptr
#define L6MsgGetLayer6MsgPtr           l6msg_get_layer6msg_ptr

//Get Array Pointers by name
#define L6MsgGetStringPtrNamed         l6msg_get_string_ptr_named
#define L6MsgGetByteArrayPtrNamed      l6msg_get_byte_array_ptr_named
#define L6MsgGetInt16ArrayPtrNamed     l6msg_get_int16_array_ptr_named
#define L6MsgGetShortArrayPtrNamed     l6msg_get_short_array_ptr_named
#define L6MsgGetInt32ArrayPtrNamed     l6msg_get_int32_array_ptr_named
#define L6MsgGetIntArrayPtrNamed       l6msg_get_int_array_ptr_named
#define L6MsgGetInt64ArrayPtrNamed     l6msg_get_int64_array_ptr_named
#define L6MsgGetLongArrayPtrNamed      l6msg_get_long_array_ptr_named
#define L6MsgGetFloatArrayPtrNamed     l6msg_get_float_array_ptr_named
#define L6MsgGetDoubleArrayPtrNamed    l6msg_get_double_array_ptr_named
#define L6MsgGetLayer6MsgPtrNamed      l6msg_get_layer6msg_ptr_named

//#define L6MsgSize                      l6msg_getSizeInBytes
#define L6MsgSize                      l6msg_get_size_bytes
#define L6MsgDeserializeInPlace        l6msg_deserialize_in_place

#define L6MsgSerialize                 l6msg_serialize
#define L6MsgSerializeHeader           l6msg_serialize_header
#define L6MsgSerializeMetadata         l6msg_serialize_metadata
#define L6MsgSerializeData             l6msg_serialize_data

#define L6MsgDeserialize               l6msg_deserialize
#define L6MsgDeserializeHeader         l6msg_deserialize_header
#define L6MsgDeserializeMetadata       l6msg_deserialize_metadata
#define L6MsgDeserializeData           l6msg_deserialize_data

//field ops
#define L6MsgGetNumFields              l6msg_get_num_fields

#define L6MsgRemoveFieldAtIndex        l6msg_remove_field_at_index
#define L6MsgRemoveField               l6msg_remove_field
#define L6MsgRemoveFieldNamed          l6msg_remove_field_named

#define L6MsgGetFieldSizeBytes         l6msg_get_field_size_bytes
#define L6MsgGetFieldSizeBytesNamed    l6msg_get_field_size_bytes_named
#define L6MsgGetFieldSizeBytesAtIndex  l6msg_get_field_size_bytes_at_index

#define L6MsgGetFieldCount             l6msg_get_field_count
#define L6MsgGetFieldCountNamed        l6msg_get_field_count_named

#define L6MsgGetFieldUnitSize          l6msg_get_field_unit_size
#define L6MsgGetFieldUnitSizeName      l6msg_get_field_unit_size_named

#define L6MsgGetFieldType              l6msg_get_field_type
#define L6MsgGetFieldTypeNamed         l6msg_get_field_type_named
#define L6MsgGetFieldTypeAtIndex       l6msg_get_field_type_at_index

#define L6MsgGetFieldIdAtIndex         l6msg_get_field_id_at_index
#define L6MsgGetFieldName              l6msg_get_field_name
#define L6MsgGetFieldNameAtIndex       l6msg_get_field_name_at_index
#define L6MsgGetFieldCountAtIndex      l6msg_get_field_count_at_index

#define L6MsgIsFieldNamed              l6msg_is_field_named
#define L6MsgIsFieldNamedAtIndex       l6msg_is_field_named_at_index


#ifdef __cplusplus
}
#endif

#endif
