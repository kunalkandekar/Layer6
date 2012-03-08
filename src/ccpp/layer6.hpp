#ifndef LAYER6_HPP__
#define LAYER6_HPP__

extern "C" {
#include "layer6.h"
}

#include <map>
#include <string>
#include <vector>

class Layer6Msg {
private:
    l6msg msg;
    int     enableExceptions;
    std::map<l6msg, Layer6Msg*> subMsgPtrs;

    Layer6Msg(void *m) {
        this->msg = (l6msg)m;
    } 

    void **getl6msg() {
        return &(this->msg);
    }

    int throwExceptionIfEnabled() {
        int ret = l6msg_get_error_code(&msg);
        if(areExceptionsEnabled()) {
            throw ret;
        }
        return ret;
    }

public:
    Layer6Msg();

    Layer6Msg(int opt);
    
    ~Layer6Msg();

    void setExceptionsEnabled(int exc) {
        enableExceptions = exc;
    }
    
    int areExceptionsEnabled() {
        return enableExceptions;
    }

    int setAutoByteOrderMode(int byteorder);
    int getAutoByteOrderMode();

    int setDeserializeLazily(int dsoa);
    int getDeserializeLazily();

    int reset();
    
    int setDeepCopyDefault(int mode);
    int getDeepCopyDefault();

    const char* const getErrorStr();
    const char* const getDebugInfo();
    int               getErrorCode();

    void setCode(int c);    
    int  getCode();
/*
    void setSubject(const char *sub);

    const char*    getSubject();
*/
//serializing
    int size();
    int getSize();
    int minBufferSize();

    int serialize           (char *buffer, int length, int *left);
    int deserialize         (char *buffer, int length, int *left);
    int deserializeInPlace  (char *buffer, int length, int *left);


    int serializeHeader     (char *buffer, int length, int *left);
    int serializeMetadata   (char *buffer, int length, int *left);
    int serializeData       (char *buffer, int length, int *left);

    int deserializeHeader   (char *buffer, int length, int *left);
    int deserializeMetadata (char *buffer, int length, int *left);
    int deserializeData     (char *buffer, int length, int *left);

//Setters
//Add Scalars
    void addInt16           (int16_t     data);
    void addShort           (short       data);
    void addInt32           (int32_t     data);
    void addInt             (int         data);
    void addInt64           (int64_t     data);    
    void addLong            (long long int data);
    void addFloat           (float       data);
    void addDouble          (double      data);
    
//Add arrays
    void addString          (char        *data);
    void addByteArray       (char        *data, int size);    
    void addInt16Array      (int16_t     *data, int count);
    void addShortArray      (short       *data, int count);
    void addInt32Array      (int32_t     *data, int count);
    void addIntArray        (int         *data, int count);
    void addInt64Array      (int64_t     *data, int count);    
    void addLongArray       (long long int *data, int count);
    void addFloatArray      (float       *data, int count);
    void addDoubleArray     (double      *data, int count);
    void addLayer6Msg       (Layer6Msg   *subMsg);
    void addLayer6Msg       (Layer6Msg   &subMsg);

//Add array pointers
    void addStringPtr       (char        *data);
    void addByteArrayPtr    (char        *data, int size);    
    void addInt16ArrayPtr   (int16_t     *data, int count);
    void addShortArrayPtr   (short       *data, int count);
    void addInt32ArrayPtr   (int32_t     *data, int count);
    void addIntArrayPtr     (int         *data, int count);
    void addInt64ArrayPtr   (int64_t     *data, int count);    
    void addLongArrayPtr    (long long int *data, int count);
    void addFloatArrayPtr   (float       *data, int count);
    void addDoubleArrayPtr  (double      *data, int count);
    void addLayer6MsgPtr    (Layer6Msg   *subMsg);
    void addLayer6MsgPtr    (Layer6Msg   &subMsg);

//Scalars
    void setInt16           (int fid,    int16_t    data);
    void setShort           (int fid,    short      data);
    void setInt32           (int fid,    int32_t    data);
    void setInt             (int fid,    int        data);
    void setInt64           (int fid,    int64_t    data);    
    void setLong            (int fid,    long long int data);
    void setFloat           (int fid,    float      data);
    void setDouble          (int fid,    double     data);

//Scalars By Name
    void setInt16           (const char *key, int16_t       data);
    void setShort           (const char *key, short         data);
    void setInt32           (const char *key, int32_t       data);
    void setInt             (const char *key, int           data);
    void setInt64           (const char *key, int64_t       data);    
    void setLong            (const char *key, long long int data);
    void setFloat           (const char *key, float         data);
    void setDouble          (const char *key, double        data);

//Arrays
    void setString          (int fid,    const char     *data);
    void setByteArray       (int fid,    char           *data, int size);
    void setInt16Array      (int fid,    int16_t        *data, int count);
    void setShortArray      (int fid,    short          *data, int count)    { setInt16Array (fid, (int16_t *)data, count); }
    void setInt32Array      (int fid,    int32_t        *data, int count);
    void setIntArray        (int fid,    int            *data, int count)    { setInt32Array (fid, (int32_t *)data, count); }
    void setInt64Array      (int fid,    int64_t        *data, int count);
    void setLongArray       (int fid,    long long int  *data, int count)    { setInt64Array (fid, (int64_t *)data, count); }
    void setFloatArray      (int fid,    float          *data, int count);
    void setDoubleArray     (int fid,    double         *data, int count);
    void setLayer6Msg       (int fid,    Layer6Msg      *subMsg);
    void setLayer6Msg       (int fid,    Layer6Msg      &subMsg);

//Scalars By Index
    void setInt16At         (int index,  int16_t    data);
    void setShortAt         (int index,  short      data);
    void setInt32At         (int index,  int32_t    data);
    void setIntAt           (int index,  int        data);
    void setInt64At         (int index,  int64_t    data);    
    void setLongAt          (int index,  long long int data);
    void setFloatAt         (int index,  float      data);
    void setDoubleAt        (int index,  double     data);

//Arrays By index
    void setStringAt        (int index,  const char     *data);
    void setByteArrayAt     (int index,  char           *data, int size);
    void setInt16ArrayAt    (int index,  int16_t        *data, int count);
    void setShortArrayAt    (int index,  short          *data, int count)    { setInt16Array (index, (int16_t *)data, count); }
    void setInt32ArrayAt    (int index,  int32_t        *data, int count);
    void setIntArrayAt      (int index,  int            *data, int count)    { setInt32Array (index, (int32_t *)data, count); }
    void setInt64ArrayAt    (int index,  int64_t        *data, int count);
    void setLongArrayAt     (int index,  long long int  *data, int count)    { setInt64Array (index, (int64_t *)data, count); }
    void setFloatArrayAt    (int index,  float          *data, int count);
    void setDoubleArrayAt   (int index,  double         *data, int count);
    void setLayer6MsgAt     (int index,  Layer6Msg      *subMsg);
    void setLayer6MsgAt     (int index,  Layer6Msg      &subMsg);

//Array Pointers By index
    void setStringPtrAt     (int index,  const char     *data);
    void setByteArrayPtrAt  (int index,  char           *data, int size);
    void setInt16ArrayPtrAt (int index,  int16_t        *data, int count);
    void setShortArrayPtrAt (int index,  short          *data, int count)    { setInt16ArrayPtr (index, (int16_t *)data, count); }
    void setInt32ArrayPtrAt (int index,  int32_t        *data, int count);
    void setIntArrayPtrAt   (int index,  int            *data, int count)    { setInt32ArrayPtr (index, (int32_t *)data, count); }
    void setInt64ArrayPtrAt (int index,  int64_t        *data, int count);
    void setLongArrayPtrAt  (int index,  long long int  *data, int count)    { setInt64ArrayPtr (index, (int64_t *)data, count); }
    void setFloatArrayPtrAt (int index,  float          *data, int count);
    void setDoubleArrayPtrAt(int index,  double         *data, int count);
    void setLayer6MsgPtrAt  (int index,  Layer6Msg      *subMsg);
    void setLayer6MsgPtrAt  (int index,  Layer6Msg      &subMsg);


//Array Pointers
    void setStringPtr       (int fid,    const char     *data);
    void setByteArrayPtr    (int fid,    char           *data, int size);
    void setInt16ArrayPtr   (int fid,    int16_t        *data, int count);
    void setShortArrayPtr   (int fid,    short          *data, int count)    { setInt16ArrayPtr (fid, (int16_t *)data, count); }
    void setInt32ArrayPtr   (int fid,    int32_t        *data, int count);
    void setIntArrayPtr     (int fid,    int            *data, int count)    { setInt32ArrayPtr (fid, (int32_t *)data, count); }
    void setInt64ArrayPtr   (int fid,    int64_t        *data, int count);
    void setLongArrayPtr    (int fid,    long long int  *data, int count)    { setInt64ArrayPtr (fid, (int64_t *)data, count); }
    void setFloatArrayPtr   (int fid,    float          *data, int count);
    void setDoubleArrayPtr  (int fid,    double         *data, int count);
    void setLayer6MsgPtr    (int fid,    Layer6Msg      *subMsg);
    void setLayer6MsgPtr    (int fid,    Layer6Msg      &subMsg);

//Arrays by name
    void setString          (const char *key, const char    *data);
    void setByteArray       (const char *key, char          *data, int size);
    void setInt16Array      (const char *key, int16_t       *data, int count);
    void setShortArray      (const char *key, short         *data, int count)    { setInt16Array (key, (int16_t *)data, count); }
    void setInt32Array      (const char *key, int32_t       *data, int count);
    void setIntArray        (const char *key, int           *data, int count)    { setInt32Array (key, (int32_t *)data, count); }
    void setInt64Array      (const char *key, int64_t       *data, int count);
    void setLongArray       (const char *key, long long int *data, int count)    { setInt64Array (key, (int64_t *)data, count); }
    void setFloatArray      (const char *key, float         *data, int count);
    void setDoubleArray     (const char *key, double        *data, int count);
    void setLayer6Msg       (const char *key, Layer6Msg     *subMsg);
    void setLayer6Msg       (const char *key, Layer6Msg     &subMsg);

//Array Pointers by name
    void setStringPtr       (const char *key, const char    *data);
    void setByteArrayPtr    (const char *key, char          *data, int size);
    void setInt16ArrayPtr   (const char *key, int16_t       *data, int count);
    void setShortArrayPtr   (const char *key, short         *data, int count)    { setInt16ArrayPtr (key, (int16_t *)data, count); }
    void setInt32ArrayPtr   (const char *key, int32_t       *data, int count);
    void setIntArrayPtr     (const char *key, int           *data, int count)    { setInt32ArrayPtr (key, (int32_t *)data, count); }
    void setInt64ArrayPtr   (const char *key, int64_t       *data, int count);
    void setLongArrayPtr    (const char *key, long long int *data, int count)    { setInt64ArrayPtr (key, (int64_t *)data, count); }
    void setFloatArrayPtr   (const char *key, float         *data, int count);
    void setDoubleArrayPtr  (const char *key, double        *data, int count);
    void setLayer6MsgPtr    (const char *key, Layer6Msg     *subMsg);
    void setLayer6MsgPtr    (const char *key, Layer6Msg     &subMsg);

//Getters
//Get Scalars
    int16_t     getInt16    (int fid);
    short       getShort    (int fid) { return (short)getInt16(fid); }
    int32_t     getInt32    (int fid);
    int         getInt      (int fid) { return (int)getInt32(fid); }
    int64_t     getInt64    (int fid);
    long long int getLong   (int fid) { return (long long int)getInt64(fid); }
    float       getFloat    (int fid);
    double      getDouble   (int fid);

    int         getString   (int fid, char *data, int len);

//Get Scalars by name
    int16_t     getInt16    (const char *key);
    short       getShort    (const char *key)  { return (short)getInt16(key); }
    int32_t     getInt32    (const char *key);
    int         getInt      (const char *key)  { return (int)getInt32(key); }
    int64_t     getInt64    (const char *key);
    long long int getLong   (const char *key)  { return (long long int)getInt64(key); }
    float       getFloat    (const char *key);
    double      getDouble   (const char *key);
    int         getString   (const char *key, char *data, int len);

// Get Arrays
    int         getByteArray    (int fid,    char       *data, int offset, int count);
    int         getInt16Array   (int fid,    int16_t    *data, int offset, int count);
    int         getShortArray   (int fid,    short      *data, int offset, int count)    { return getInt16Array(fid, (int16_t*)data, offset, count); }
    int         getInt32Array   (int fid,    int32_t    *data, int offset, int count);
    int         getIntArray     (int fid,    int        *data, int offset, int count)    { return getInt32Array(fid, (int32_t*)data, offset, count); }
    int         getInt64Array   (int fid,    int64_t    *data, int offset, int count);    
    int         getLongArray    (int fid,    long long int *data, int offset, int count) { return getInt64Array(fid, (int64_t*)data, offset, count); }
    int         getFloatArray   (int fid,    float      *data, int offset, int count);
    int         getDoubleArray  (int fid,    double     *data, int offset, int count);
    
//Get Arrays By Name
    int         getByteArray    (const char *key, char          *data, int offset, int size);
    int         getInt16Array   (const char *key, int16_t       *data, int offset, int count);    
    int         getShortArray   (const char *key, short         *data, int offset, int count)    { return getInt16Array(key, (int16_t*)data, offset, count); }
    int         getInt32Array   (const char *key, int32_t       *data, int offset, int count);
    int         getIntArray     (const char *key, int           *data, int offset, int count)    { return getInt32Array(key, (int32_t*)data, offset, count); }
    int         getInt64Array   (const char *key, int64_t       *data, int offset, int count);
    int         getLongArray    (const char *key, long long int *data, int offset, int count)    { return getInt64Array(key, (int64_t*)data, offset, count); }
    int         getFloatArray   (const char *key, float         *data, int offset, int count);
    int         getDoubleArray  (const char *key, double        *data, int offset, int count);
        
//Get sub-message Ptr
    Layer6Msg*  getLayer6MsgPtr    (int fid);
    Layer6Msg*  getLayer6MsgPtr    (const char *key);
    Layer6Msg*  getLayer6MsgPtrAt  (int index);

//Get sub-message deep-copy
    int         getLayer6MsgAt  (int index,        Layer6Msg *toMsg);
    int         getLayer6Msg    (int fid,          Layer6Msg *toMsg);
    int         getLayer6Msg    (const char *key,  Layer6Msg *toMsg);

    int         getLayer6MsgAt  (int index,        Layer6Msg &toMsg);
    int         getLayer6Msg    (int fid,          Layer6Msg &toMsg);
    int         getLayer6Msg    (const char *key,  Layer6Msg &toMsg);
    
//duplicate
    int         dup (Layer6Msg *toMsg);
    int         dup (Layer6Msg &toMsg);
    //TODO: implement copy constructor for this


//Get Array Pointers
    const char* const       getStringPtr        (int fid);
    const char* const       getByteArrayPtr     (int fid, int *size);
    const int16_t* const    getInt16ArrayPtr    (int fid, int *count);    
    const short* const      getShortArrayPtr    (int fid, int *count)    { return  (const int16_t* const) getInt16ArrayPtr(fid,count); }
    const int32_t* const    getInt32ArrayPtr    (int fid, int *count);
    const int* const        getIntArrayPtr      (int fid, int *count)    { return  (const int32_t* const) getInt32ArrayPtr(fid,count); }
    const int64_t* const    getInt64ArrayPtr    (int fid, int *count);
    const long long int* const getLongArrayPtr  (int fid, int *count)    { return  (const int64_t* const) getInt64ArrayPtr(fid,count); }
    const float* const      getFloatArrayPtr    (int fid, int *count);
    const double* const     getDoubleArrayPtr   (int fid, int *count);

//Get Array Pointers by name
    const char* const       getStringPtr        (const char *key);
    const char* const       getByteArrayPtr     (const char *key, int *size);
    const int16_t* const    getInt16ArrayPtr    (const char *key, int *count);    
    const short* const      getShortArrayPtr    (const char *key, int *count)    { return  (const int16_t* const) getInt16ArrayPtr(key, count); }
    const int32_t* const    getInt32ArrayPtr    (const char *key, int *count);
    const int* const        getIntArrayPtr      (const char *key, int *count)    { return  (const int32_t* const) getInt32ArrayPtr(key, count); }
    const int64_t* const    getInt64ArrayPtr    (const char *key, int *count);
    const long long int* const getLongArrayPtr  (const char *key, int *count)    { return  (const int64_t* const) getInt64ArrayPtr(key, count); }
    const float* const      getFloatArrayPtr    (const char *key, int *count);
    const double* const     getDoubleArrayPtr   (const char *key, int *count);


//Get Scalars by index
    int16_t       getInt16At (int ix);
    short         getShortAt (int ix) { return (short)getInt16At(ix); }
    int32_t       getInt32At (int ix);
    int           getIntAt   (int ix) { return (int)getInt32At(ix); }
    int64_t       getInt64At (int ix);
    long long int getLongAt  (int ix) { return (long long int)getInt64At(ix); }
    float         getFloatAt (int ix);
    double        getDoubleAt(int ix);
    int           getStringAt(int ix, char *data, int len);
    
//get arrays
    int         getByteArrayAt  (int ix, char        *data, int offset, int count);
    int         getInt16ArrayAt (int ix, int16_t     *data, int offset, int count);
    int         getShortArrayAt (int ix, short       *data, int offset, int count)    { return getInt16ArrayAt(ix, (int16_t*)data, offset, count); }
    int         getInt32ArrayAt (int ix, int32_t     *data, int offset, int count);
    int         getIntArrayAt   (int ix, int         *data, int offset, int count)    { return getInt32ArrayAt(ix, (int32_t*)data, offset, count); }
    int         getInt64ArrayAt (int ix, int64_t     *data, int offset, int count);    
    int         getLongArrayAt  (int ix, long long int *data, int offset, int count)  { return getInt64ArrayAt(ix, (int64_t*)data, offset, count); }
    int         getFloatArrayAt (int ix, float       *data, int offset, int count);
    int         getDoubleArrayAt(int ix, double      *data, int offset, int count);

//get array pointers
    const char* const        getStringPtrAt     (int ix);
    const char* const        getByteArrayPtrAt  (int ix, int *size);
    const int16_t* const     getInt16ArrayPtrAt (int ix, int *count);    
    const short* const       getShortArrayPtrAt (int ix, int *count) { return  (const int16_t* const) getInt16ArrayPtrAt(ix, count); }
    const int32_t* const     getInt32ArrayPtrAt (int ix, int *count);
    const int* const         getIntArrayPtrAt   (int ix, int *count) { return  (const int32_t* const) getInt32ArrayPtrAt(ix, count); }
    const int64_t* const     getInt64ArrayPtrAt (int ix, int *count);
    const long long int* const getLongArrayPtrAt(int ix, int *count) { return  (const int64_t* const) getInt64ArrayPtrAt(ix, count); }
    const float* const       getFloatArrayPtrAt (int ix, int *count);
    const double* const      getDoubleArrayPtrAt(int ix, int *count);

//Vectors (handle this using template meta-programming?
    void addString          (std::string                &data);
    void addByteVector      (std::vector<char>          &data);
    void addInt16Vector     (std::vector<int16_t>       &data);
    void addShortVector     (std::vector<short>         &data);
    void addInt32Vector     (std::vector<int32_t>       &data);
    void addIntVector       (std::vector<int>           &data);
    void addInt64Vector     (std::vector<int64_t>       &data);
    void addLongVector      (std::vector<long long int> &data);
    void addFloatVector     (std::vector<float>         &data);
    void addDoubleVector    (std::vector<double>        &data);

    void setString          (int fid,           std::string                &data);
    void setByteVector      (int fid,           std::vector<char>          &data);
    void setInt16Vector     (int fid,           std::vector<int16_t>       &data);
    void setShortVector     (int fid,           std::vector<short>         &data);
    void setInt32Vector     (int fid,           std::vector<int32_t>       &data);
    void setIntVector       (int fid,           std::vector<int>           &data);
    void setInt64Vector     (int fid,           std::vector<int64_t>       &data);
    void setLongVector      (int fid,           std::vector<long long int> &data);
    void setFloatVector     (int fid,           std::vector<float>         &data);
    void setDoubleVector    (int fid,           std::vector<double>        &data);
    
    void setString          (const char *key,   std::string                &data);
    void setByteVector      (const char *key,   std::vector<char>          &data);
    void setInt16Vector     (const char *key,   std::vector<int16_t>       &data);
    void setShortVector     (const char *key,   std::vector<short>         &data);
    void setInt32Vector     (const char *key,   std::vector<int32_t>       &data);
    void setIntVector       (const char *key,   std::vector<int>           &data);
    void setInt64Vector     (const char *key,   std::vector<int64_t>       &data);
    void setLongVector      (const char *key,   std::vector<long long int> &data);
    void setFloatVector     (const char *key,   std::vector<float>         &data);
    void setDoubleVector    (const char *key,   std::vector<double>        &data);

    void getString          (int fid,           std::string                &data);
    void getByteVector      (int fid,           std::vector<char>          &data);
    void getInt16Vector     (int fid,           std::vector<int16_t>       &data);
    void getShortVector     (int fid,           std::vector<short>         &data);
    void getInt32Vector     (int fid,           std::vector<int32_t>       &data);
    void getIntVector       (int fid,           std::vector<int>           &data);
    void getInt64Vector     (int fid,           std::vector<int64_t>       &data);
    void getLongVector      (int fid,           std::vector<long long int> &data);
    void getFloatVector     (int fid,           std::vector<float>         &data);
    void getDoubleVector    (int fid,           std::vector<double>        &data);

    void getStringAt        (int index,         std::string                &data);
    void getByteVectorAt    (int index,         std::vector<char>          &data);
    void getInt16VectorAt   (int index,         std::vector<int16_t>       &data);
    void getShortVectorAt   (int index,         std::vector<short>         &data);
    void getInt32VectorAt   (int index,         std::vector<int32_t>       &data);
    void getIntVectorAt     (int index,         std::vector<int>           &data);
    void getInt64VectorAt   (int index,         std::vector<int64_t>       &data);
    void getLongVectorAt    (int index,         std::vector<long long int> &data);
    void getFloatVectorAt   (int index,         std::vector<float>         &data);
    void getDoubleVectorAt  (int index,         std::vector<double>        &data);

    void getString          (const char *key,   std::string                &data);
    void getByteVector      (const char *key,   std::vector<char>          &data);
    void getInt16Vector     (const char *key,   std::vector<int16_t>       &data);
    void getShortVector     (const char *key,   std::vector<short>         &data);
    void getInt32Vector     (const char *key,   std::vector<int32_t>       &data);
    void getIntVector       (const char *key,   std::vector<int>           &data);
    void getInt64Vector     (const char *key,   std::vector<int64_t>       &data);
    void getLongVector      (const char *key,   std::vector<long long int> &data);
    void getFloatVector     (const char *key,   std::vector<float>         &data);
    void getDoubleVector    (const char *key,   std::vector<double>        &data);

//field ops
    int getNumFields();

    int removeFieldAt       (int index);
    int removeField         (int fid);
    int removeField         (const char *key);

    int getFieldSizeBytesAt (int index,int *size);
    int getFieldSizeBytes   (int fid,int *size);
    int getFieldSizeBytes   (const char *key, int *size);

    int getFieldSizeBytesAt (int index);
    int getFieldSizeBytes   (int fid);
    int getFieldSizeBytes   (const char *key);

    int getFieldCountAt     (int index,int *count);
    int getFieldCount       (int fid,int *count);
    int getFieldCount       (const char *key, int *count);

    int getFieldCountAt     (int index);
    int getFieldCount       (int fid);
    int getFieldCount       (const char *key);

    int getFieldUnitSizeAt  (int index,int *size);
    int getFieldUnitSize    (int fid,int *size);
    int getFieldUnitSize    (const char *key, int *size);

    int getFieldUnitSizeAt  (int index);
    int getFieldUnitSize    (int fid);
    int getFieldUnitSize    (const char *key);

    int getFieldTypeAt      (int index, int *type);
    int getFieldType        (int fid,int *type);
    int getFieldType        (const char *key, int *type);
    
    int getFieldTypeAt      (int index);
    int getFieldType        (int fid);
    int getFieldType        (const char *key);

    int getFieldIdAt        (int index, int *fid);
    int getFieldIdAt        (int index);

    int getFieldNameAt      (int index, const char **name);

    const char* const getFieldNameAt(int index);

    int isFieldNamed        (int fid);
    int isFieldNamedAt      (int index);
    
    int isFieldArray        (const char *key);
    int isFieldArray        (int fid);
    int isFieldArrayAt      (int index);
    
    static const char *getErrorStrForCode(int e) {
       return l6_get_error_str_for_code(e);
    }
};

//#ifndef __cplusplus
//}
//#endif

#endif
