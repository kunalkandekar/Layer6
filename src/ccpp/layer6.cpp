#include "layer6.hpp"
#include <iostream>

Layer6Msg::Layer6Msg() {
	l6msg_init(&msg);
	setExceptionsEnabled(1);
	//std::cout<<"\nL6: Allocated msg="<<msg<<std::flush;
}

Layer6Msg::Layer6Msg(int opt) {
	l6msg_init(&msg);
	setExceptionsEnabled(1);
}

Layer6Msg::~Layer6Msg() {
	//clean up any references to sub-msgs
	std::map<l6msg, Layer6Msg*>::iterator itr = subMsgPtrs.begin();
	while(itr != subMsgPtrs.end()) {
		itr->second->msg = NULL;		//to avoid double-free in embedded sub-msgs
		delete itr->second;
		itr++;
	}
	subMsgPtrs.clear();
	//std::cout<<"\nL6: Freeing msg="<<msg<<std::flush;
	if(msg) {
		//std::cout<<"...  "<<std::flush;
		l6msg_free(&msg);
		msg = NULL;
	}
}

int Layer6Msg::reset() {
	return l6msg_reset(&msg);
}

int Layer6Msg::setDeepCopyDefault(int mode) {
	return l6msg_set_deep_copy_default(&msg, mode);
}

int Layer6Msg::getDeepCopyDefault() {
	return l6msg_get_deep_copy_default(&msg);
}

int Layer6Msg::setAutoByteOrderMode(int byteorder) {
	return l6msg_set_auto_byte_order_mode(&msg, byteorder);
}

int Layer6Msg::getAutoByteOrderMode() {
	return l6msg_get_auto_byte_order_mode(&msg);
}

int Layer6Msg::setDeserializeLazily(int dsoa)
{
	return l6msg_set_deserialize_lazily(&msg, dsoa);
}

int Layer6Msg::getDeserializeLazily()
{
	return l6msg_get_deserialize_lazily(&msg);
}

const char*	const Layer6Msg::getErrorStr() {
	return l6msg_get_error_str(&msg);
}

int Layer6Msg::getErrorCode() {
	return l6msg_get_error_code(&msg);
}

const char*	const Layer6Msg::getDebugInfo() {
	return l6msg_get_debug_info(&msg);
}

/*
void Layer6Msg::setSubject(const char *sub) {
	return l6msg_set_subject(&msg, sub);
}
const char*	Layer6Msg::getSubject() {
	return l6msg_get_subject(&msg);
}
*/

//serializing
int Layer6Msg::size() {
	return l6msg_size(&msg);
}

int Layer6Msg::getSize() {
	return l6msg_get_size(&msg);
}

int Layer6Msg::minBufferSize() {
	return l6msg_min_buffer_size(&msg);
}

int Layer6Msg::serialize(char* buffer, int length, int *left) {
	int ret = l6msg_serialize(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::deserialize(char *buffer, int length, int *left) {
	int ret = l6msg_deserialize(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::deserializeInPlace(char *buffer, int length, int *left) {
	int ret = l6msg_deserialize_in_place(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::serializeHeader(char *buffer, int length, int *left) {
	int ret = l6msg_serialize_header(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::serializeMetadata(char *buffer, int length, int *left) {
	int ret = l6msg_serialize_metadata(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::serializeData(char *buffer, int length, int *left) {
	int ret = l6msg_serialize_data(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::deserializeHeader(char *buffer, int length, int *left) {
	int ret = l6msg_deserialize_header(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::deserializeMetadata(char *buffer, int length, int *left) {
	int ret = l6msg_deserialize_metadata(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::deserializeData(char *buffer, int length, int *left) {
	int ret = l6msg_deserialize_data(&msg, buffer, length, left);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

//Metadata
void Layer6Msg::setCode(int code) {
	l6msg_set_code(&msg, code);
}

int Layer6Msg::getCode() {
	return l6msg_get_code(&msg);
}


//Scalars
void Layer6Msg::setShort(int fid, short data) {
    setInt16(fid, data);
}

void Layer6Msg::setInt(int fid, int data) {
    setInt32(fid, data);
}

void Layer6Msg::setLong(int fid, long long int data) {
    setInt64(fid, data);
}

void Layer6Msg::setInt16(int id, int16_t data) {
	int ret = l6msg_set_int16(&msg, id, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32(int id, int32_t data) {
	int ret = l6msg_set_int32(&msg, id, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64(int id, int64_t data) {
	int ret = l6msg_set_int64(&msg, id, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloat(int id, float data) {
	int ret = l6msg_set_float(&msg, id, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDouble(int id, double data) {
	int ret = l6msg_set_double(&msg, id, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//Scalars By Name
void Layer6Msg::setShort(const char *key, short data) {
    setInt16(key, data);
}

void Layer6Msg::setInt(const char *key, int data) {
    setInt32(key, data);
}

void Layer6Msg::setLong(const char *key, long long int data) {
    setInt64(key, data);
}

void Layer6Msg::setInt16(const char *key, int16_t data) {
	int ret = l6msg_set_int16_named(&msg, key, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32(const char *key, int32_t data) {
	int ret = l6msg_set_int32_named(&msg, key, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64(const char *key, int64_t data) {
	int ret = l6msg_set_int64_named(&msg, key, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloat(const char *key, float data) {
	int ret = l6msg_set_float_named(&msg, key, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDouble(const char *key, double data) {
	int ret = l6msg_set_double_named(&msg, key, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//Arrays
void Layer6Msg::setString(int id, const char *data) {
	int ret = l6msg_set_string(&msg, id, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setByteArray(int id, char *data, int size) {
	int ret = l6msg_set_byte_array(&msg, id, data, size);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt16Array(int id, int16_t *data, int count) {
	int ret = l6msg_set_int16_array(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32Array(int id, int32_t *data, int count) {
	int ret = l6msg_set_int32_array(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64Array(int id, int64_t *data, int count) {
	int ret = l6msg_set_int64_array(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloatArray(int id, float *data, int count) {
	int ret = l6msg_set_float_array(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDoubleArray(int id, double *data, int count) {
	int ret = l6msg_set_double_array(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//Array Pointers
void Layer6Msg::setStringPtr(int id, const char *data) {
	int ret = l6msg_set_string_ptr(&msg, id, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setByteArrayPtr(int id, char *data, int size) {
	int ret = l6msg_set_byte_array_ptr(&msg, id, data, size);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt16ArrayPtr(int id, int16_t *data, int count) {
	int ret = l6msg_set_int16_array_ptr(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32ArrayPtr(int id, int32_t *data, int count) {
	int ret = l6msg_set_int32_array_ptr(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64ArrayPtr(int id, int64_t *data, int count) {
	int ret = l6msg_set_int64_array_ptr(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloatArrayPtr(int id, float *data, int count) {
	int ret = l6msg_set_float_array_ptr(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDoubleArrayPtr(int id, double *data, int count) {
	int ret = l6msg_set_double_array_ptr(&msg, id, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//Arrays by name
void Layer6Msg::setString(const char *key, const char *data) {
	int ret = l6msg_set_string_named(&msg, key, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setByteArray(const char *key, char *data, int size) {
	int ret = l6msg_set_byte_array_named(&msg, key, data, size);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt16Array(const char *key, int16_t *data, int count) {
	int ret = l6msg_set_int16_array_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32Array(const char *key, int32_t *data, int count) {
	int ret = l6msg_set_int32_array_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64Array(const char *key, int64_t *data, int count) {
	int ret = l6msg_set_int64_array_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloatArray(const char *key, float *data, int count) {
	int ret = l6msg_set_float_array_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDoubleArray(const char *key, double *data, int count) {
	int ret = l6msg_set_double_array_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//Array Pointers by name
void Layer6Msg::setStringPtr(const char *key, const char *data) {
	int ret = l6msg_set_string_ptr_named(&msg, key, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setByteArrayPtr(const char *key, char *data, int size) {
	int ret = l6msg_set_byte_array_ptr_named(&msg, key, data, size);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt16ArrayPtr(const char *key, int16_t *data, int count) {
	int ret = l6msg_set_int16_array_ptr_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32ArrayPtr(const char *key, int32_t *data, int count) {
	int ret = l6msg_set_int32_array_ptr_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64ArrayPtr(const char *key, int64_t *data, int count) {
	int ret = l6msg_set_int64_array_ptr_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloatArrayPtr(const char *key, float *data, int count) {
	int ret = l6msg_set_float_array_ptr_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDoubleArrayPtr(const char *key, double *data, int count) {
	int ret = l6msg_set_double_array_ptr_named(&msg, key, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}


//Set by index
//Scalars By Index
void Layer6Msg::setInt16At(int index, int16_t data) {
	int ret = l6msg_set_int16_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setShortAt(int index, short data) {
	int ret = l6msg_set_short_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32At(int index, int32_t data) {
	int ret = l6msg_set_int32_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setIntAt(int index, int data) {
	int ret = l6msg_set_int_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64At(int index, int64_t data) {
	int ret = l6msg_set_int64_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}
    
void Layer6Msg::setLongAt(int index, long long int data) {
	int ret = l6msg_set_long_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloatAt(int index, float data) {
	int ret = l6msg_set_float_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDoubleAt(int index, double data) {
	int ret = l6msg_set_double_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//Arrays By index
void Layer6Msg::setStringAt(int index, const char *data) {
	int ret = l6msg_set_string_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setByteArrayAt(int index, char *data, int size) {
	int ret = l6msg_set_byte_array_at_index(&msg, index, data, size);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt16ArrayAt(int index, int16_t *data, int count) {
	int ret = l6msg_set_int16_array_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32ArrayAt(int index, int32_t *data, int count) {
	int ret = l6msg_set_int32_array_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64ArrayAt(int index, int64_t *data, int count) {
	int ret = l6msg_set_int64_array_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloatArrayAt(int index, float *data, int count) {
	int ret = l6msg_set_float_array_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDoubleArrayAt(int index, double *data, int count) {
	int ret = l6msg_set_double_array_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6MsgAt(int index, Layer6Msg *subMsg) {
	int ret = l6msg_set_layer6_msg_at_index(&msg, index, subMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6MsgAt(int index, Layer6Msg &subMsg) {
	int ret = l6msg_set_layer6_msg_at_index(&msg, index, subMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//Array Pointers By index
void Layer6Msg::setStringPtrAt(int index, const char *data) {
	int ret = l6msg_set_string_ptr_at_index(&msg, index, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setByteArrayPtrAt(int index, char *data, int size) {
	int ret = l6msg_set_byte_array_ptr_at_index(&msg, index, data, size);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt16ArrayPtrAt(int index, int16_t *data, int count) {
	int ret = l6msg_set_int16_array_ptr_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt32ArrayPtrAt(int index, int32_t *data, int count) {
	int ret = l6msg_set_int32_array_ptr_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setInt64ArrayPtrAt(int index, int64_t *data, int count) {
	int ret = l6msg_set_int64_array_ptr_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setFloatArrayPtrAt(int index, float *data, int count) {
	int ret = l6msg_set_float_array_ptr_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setDoubleArrayPtrAt(int index, double *data, int count) {
	int ret = l6msg_set_double_array_ptr_at_index(&msg, index, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6MsgPtrAt(int index, Layer6Msg *subMsg) {
	int ret = l6msg_set_layer6_msg_ptr_at_index(&msg, index, subMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6MsgPtrAt(int index, Layer6Msg &subMsg) {
	int ret = l6msg_set_layer6_msg_ptr_at_index(&msg, index, subMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}


//Get
//Get Scalars
int16_t Layer6Msg::getInt16(int id) {
	int16_t data = 0;
	if(l6msg_get_int16(&msg, id, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

int32_t Layer6Msg::getInt32(int id) {
	int32_t data = 0;
	if(l6msg_get_int32(&msg, id, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

int64_t Layer6Msg::getInt64(int id) {
	int64_t data = 0;
	if(l6msg_get_int64(&msg, id, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

float Layer6Msg::getFloat(int id) {
	float data = 0;
	if(l6msg_get_float(&msg, id, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

double Layer6Msg::getDouble(int id) {
	double data = 0;
	if(l6msg_get_double(&msg, id, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

//Get Scalars by name
int16_t Layer6Msg::getInt16(const char *key) {
	int16_t data = 0;
	if(l6msg_get_int16_named(&msg, key, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

int32_t Layer6Msg::getInt32(const char *key) {
	int32_t data = 0;
	if(l6msg_get_int32_named(&msg, key, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

int64_t Layer6Msg::getInt64(const char *key) {
	int64_t data = 0;
	if(l6msg_get_int64_named(&msg, key, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

float Layer6Msg::getFloat(const char *key) {
	float data = 0;
	if(l6msg_get_float_named(&msg, key, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

double Layer6Msg::getDouble(const char *key) {
	double data = 0;
	if(l6msg_get_double_named(&msg, key, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

//get String
int Layer6Msg::getString(int id, char *data, int len) {
	int ret = 0;
	ret = l6msg_get_string(&msg, id, data, len);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

// Get Arrays
int Layer6Msg::getByteArray(int id, char *data, int offset, int count) {
	int ret = l6msg_get_byte_array(&msg, id, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getInt16Array(int id, int16_t *data, int offset, int count) {
	int ret = l6msg_get_int16_array(&msg, id, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getInt32Array(int id, int32_t *data, int offset, int count) {
	int ret = l6msg_get_int32_array(&msg, id, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getInt64Array(int id, int64_t *data, int offset, int count) {
	int ret = l6msg_get_int64_array(&msg, id, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getFloatArray(int id, float *data, int offset, int count) {
	int ret = l6msg_get_float_array(&msg, id, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;	
}

int Layer6Msg::getDoubleArray(int id, double *data, int offset, int count) {
	int ret = l6msg_get_double_array(&msg, id, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;	
}

//Get String By Name
int Layer6Msg::getString(const char *key, char *data, int len) {
	int ret = 0;
	ret = l6msg_get_string_named(&msg, key, data, len);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

//Get Arrays By Name
int Layer6Msg::getByteArray(const char *key, char *data, int offset, int size) {
	int ret = l6msg_get_byte_array_named(&msg, key, data, offset, size);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;	
}

int Layer6Msg::getInt16Array(const char *key, int16_t *data, int offset, int count) {
	int ret = l6msg_get_int16_array_named(&msg, key, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getInt32Array(const char *key, int32_t *data, int offset, int count) {
	int ret = l6msg_get_int32_array_named(&msg, key, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getInt64Array(const char *key, int64_t *data, int offset, int count) {
	int ret = l6msg_get_int64_array_named(&msg, key, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getFloatArray(const char *key, float *data, int offset, int count) {
	int ret = l6msg_get_float_array_named(&msg, key, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getDoubleArray(const char *key, double *data, int offset, int count) {
	int ret = l6msg_get_double_array_named(&msg, key, data, offset, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

//Get String Pointer
const char* const Layer6Msg::getStringPtr(int id) {
	const char *data = NULL;
	if(l6msg_get_string_ptr(&msg, id, &data) < 0) {
		throwExceptionIfEnabled();
	}
	//return l6msg_getString(&msg, id, data, len);
	return (const char* const)data;
}

//Get Array Pointers
const char* const Layer6Msg::getByteArrayPtr(int id, int *size) {
	char *data = NULL;
	int ret = l6msg_get_byte_array_ptr(&msg, id, &data, size);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return (const char* const) data;
}

const int16_t* const Layer6Msg::getInt16ArrayPtr(int id, int *count) {
	int16_t *data = NULL;
	int ret = l6msg_get_int16_array_ptr(&msg, id, &data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return (const int16_t* const)data;
}

const int32_t* const Layer6Msg::getInt32ArrayPtr(int id, int *count) {
	int32_t *data = NULL;
	int ret = l6msg_get_int32_array_ptr(&msg, id, &data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return (const int32_t* const)data;
}

const int64_t* const Layer6Msg::getInt64ArrayPtr(int id, int *count) {
	int64_t *data = NULL;
	int ret = l6msg_get_int64_array_ptr(&msg, id, &data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return (const int64_t* const)data;
}

const float* const Layer6Msg::getFloatArrayPtr(int id, int *count) {
	float *data = NULL;
	int ret = l6msg_get_float_array_ptr(&msg, id, &data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return (const float* const)data;
}

const double* const Layer6Msg::getDoubleArrayPtr(int id, int *count) {
	double *data = NULL;
	int ret = l6msg_get_double_array_ptr(&msg, id, &data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return (const double* const)data;
}

//Get String Pointer by name
const char* const Layer6Msg::getStringPtr(const char *key) {
	const char *data = NULL;
	if(l6msg_get_string_ptr_named(&msg, key, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return (const char* const)data;
	//return l6msg_getString(&msg, id, data, len);
}


//Get Array Pointers by name
const char* const Layer6Msg::getByteArrayPtr(const char *key, int *size) {
	char *data = NULL;
	if(l6msg_get_byte_array_ptr_named(&msg, key, &data, size) < 0) {
		throwExceptionIfEnabled();
	}
	return (const char* const)data;
}

const int16_t* const Layer6Msg::getInt16ArrayPtr(const char *key, int *count) {
	int16_t *data = NULL;
	if(l6msg_get_int16_array_ptr_named(&msg, key, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	return (const int16_t* const)data;
}

const int32_t* const Layer6Msg::getInt32ArrayPtr(const char *key, int *count) {
	int *data = NULL;
	if(l6msg_get_int32_array_ptr_named(&msg, key, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	return (const int32_t* const)data;
}

const int64_t* const Layer6Msg::getInt64ArrayPtr(const char *key, int *count) {
	int64_t *data = NULL;
	if(l6msg_get_int64_array_ptr_named(&msg, key, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	return (const int64_t* const)data;
}

const float* const Layer6Msg::getFloatArrayPtr(const char *key, int *count) {
	float *data = NULL;
	if(l6msg_get_float_array_ptr_named(&msg, key, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	return (const float* const)data;
}

const double* const Layer6Msg::getDoubleArrayPtr(const char *key, int *count) {
	double *data = NULL;
	if(l6msg_get_double_array_ptr_named(&msg, key, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	return (const double* const)data;
}

//sub-msgs
void Layer6Msg::addLayer6Msg(Layer6Msg *subMsg) {
	int ret = l6msg_add_layer6_msg(&msg, subMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6Msg(int id, Layer6Msg *subMsg) {
	int ret = l6msg_set_layer6_msg(&msg, id, subMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6Msg(const char *key, Layer6Msg *subMsg) {
	int ret = l6msg_set_layer6_msg_named(&msg, key, subMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addLayer6Msg(Layer6Msg &subMsg) {
	int ret = l6msg_add_layer6_msg(&msg, subMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6Msg(int id, Layer6Msg &subMsg) {
	int ret = l6msg_set_layer6_msg(&msg, id, subMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6Msg(const char *key, Layer6Msg &subMsg) {
	int ret = l6msg_set_layer6_msg_named(&msg, key, subMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//sub-msg ptrs
void Layer6Msg::addLayer6MsgPtr(Layer6Msg *subMsg) {
	int ret = l6msg_add_layer6_msg_ptr(&msg, subMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6MsgPtr(int id, Layer6Msg *subMsg) {
	int ret = l6msg_set_layer6_msg_ptr(&msg, id, subMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6MsgPtr(const char *key, Layer6Msg *subMsg) {
	int ret = l6msg_set_layer6_msg_ptr_named(&msg, key, subMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addLayer6MsgPtr(Layer6Msg &subMsg) {
	int ret = l6msg_add_layer6_msg_ptr(&msg, subMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6MsgPtr(int id, Layer6Msg &subMsg) {
	int ret = l6msg_set_layer6_msg_ptr(&msg, id, subMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::setLayer6MsgPtr(const char *key, Layer6Msg &subMsg) {
	int ret = l6msg_set_layer6_msg_ptr_named(&msg, key, subMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

Layer6Msg* Layer6Msg::getLayer6MsgPtrAt(int index) {
	l6msg msg2 = NULL;
	int ret = l6msg_get_layer6_msg_ptr_at_index(&msg, index, &msg2);
	Layer6Msg *subMsg = NULL;
	if((ret >= 0) && (msg2 != 0)) {
		std::map<l6msg, Layer6Msg*>::iterator itr = subMsgPtrs.find(msg2);
	    if(itr != subMsgPtrs.end()) 
	    {
			subMsg = itr->second;
	    }
	    else
	    {
			subMsg = new Layer6Msg(msg2);
			subMsgPtrs[msg2] = subMsg;
	    }
	}
	else {
		throwExceptionIfEnabled();
	}
	return subMsg;
}

Layer6Msg* Layer6Msg::getLayer6MsgPtr(int id) {
	l6msg msg2 = NULL;
	int ret = l6msg_get_layer6_msg_ptr(&msg, id, &msg2);
	Layer6Msg *subMsg = NULL;
	if((ret >= 0) && (msg2 != 0)) {
		std::map<l6msg, Layer6Msg*>::iterator itr = subMsgPtrs.find(msg2);
	    if(itr != subMsgPtrs.end()) 
	    {
			subMsg = itr->second;
	    }
	    else
	    {
			subMsg = new Layer6Msg(msg2);
			subMsgPtrs[msg2] = subMsg;
	    }
	}
	else {
		throwExceptionIfEnabled();
	}
	return subMsg;
}

Layer6Msg* Layer6Msg::getLayer6MsgPtr(const char *key) {
	l6msg msg2 = NULL;
	int ret = l6msg_get_layer6_msg_ptr_named(&msg, key, &msg2);
	Layer6Msg *subMsg = NULL;
	if((ret >= 0) && (msg2 != 0)) {
		std::map<l6msg, Layer6Msg*>::iterator itr = subMsgPtrs.find(msg2);
	    if(itr != subMsgPtrs.end()) 
	    {
			subMsg = itr->second;
	    }
	    else
	    {
			subMsg = new Layer6Msg(msg2);
			subMsgPtrs[msg2] = subMsg;
	    }
	}
	else {
		throwExceptionIfEnabled();
	}
	return subMsg;
}

int Layer6Msg::getLayer6MsgAt(int index, Layer6Msg *toMsg) {
	int ret = l6msg_get_layer6_msg_at_index(&msg, index, toMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getLayer6Msg(const char *key, Layer6Msg *toMsg) {
	int ret = l6msg_get_layer6_msg_named(&msg, key, toMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getLayer6Msg(int id, Layer6Msg *toMsg) {
	int ret = l6msg_get_layer6_msg(&msg, id, toMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getLayer6MsgAt(int index, Layer6Msg &toMsg) {
	int ret = l6msg_get_layer6_msg_at_index(&msg, index, toMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getLayer6Msg(const char *key, Layer6Msg &toMsg) {
	int ret = l6msg_get_layer6_msg_named(&msg, key, toMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getLayer6Msg(int id, Layer6Msg &toMsg) {
	int ret = l6msg_get_layer6_msg(&msg, id, toMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::dup(Layer6Msg *toMsg) {
	int ret = l6msg_dup(&msg, toMsg->getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::dup(Layer6Msg &toMsg) {
	int ret = l6msg_dup(&msg, toMsg.getl6msg());
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}


//Add fields
//Add Scalars
void Layer6Msg::addInt16(int16_t data) {
	int ret = l6msg_add_int16(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addShort(short data) {
	int ret = l6msg_add_int16(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addInt32(int32_t data) {
	int ret = l6msg_add_int32(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addInt(int data) {
	int ret = l6msg_add_int32(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addInt64(int64_t data) {
	int ret = l6msg_add_int64(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}
	
void Layer6Msg::addLong(long long int data) {
	int ret = l6msg_add_int64(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addFloat(float data)  {
	int ret = l6msg_add_float(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addDouble(double data) {
	int ret = l6msg_add_double(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addString(char *data) {
	int ret = l6msg_add_string(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addByteArray(char *data, int count) {
	int ret = l6msg_add_byte_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}
 
void Layer6Msg::addInt16Array(int16_t *data, int count) {
	int ret = l6msg_add_int16_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}
 
void Layer6Msg::addShortArray(short *data, int count) {
	int ret = l6msg_add_int16_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addInt32Array(int32_t *data, int count) {
	int ret = l6msg_add_int32_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addIntArray(int *data, int count) {
	int ret = l6msg_add_int32_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addInt64Array(int64_t *data, int count) {
	int ret = l6msg_add_int64_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}
	
void Layer6Msg::addLongArray(long long int *data, int count) {
	int ret = l6msg_add_int64_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addFloatArray(float  *data, int count) {
	int ret = l6msg_add_float_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addDoubleArray(double *data, int count) {
	int ret = l6msg_add_double_array(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addStringPtr(char *data) {
	int ret = l6msg_add_string_ptr(&msg, data);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addByteArrayPtr(char *data, int count) {
	int ret = l6msg_add_byte_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addInt16ArrayPtr(int16_t *data, int count) {
	int ret = l6msg_add_int16_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}
 
void Layer6Msg::addShortArrayPtr(short *data, int count) {
	int ret = l6msg_add_int16_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addInt32ArrayPtr(int32_t *data, int count) {
	int ret = l6msg_add_int32_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addIntArrayPtr(int *data, int count) {
	int ret = l6msg_add_int32_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addInt64ArrayPtr(int64_t *data, int count) {
	int ret = l6msg_add_int64_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}
	
void Layer6Msg::addLongArrayPtr(long long int *data, int count) {
	int ret = l6msg_add_int64_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addFloatArrayPtr(float *data, int count) {
	int ret = l6msg_add_float_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

void Layer6Msg::addDoubleArrayPtr(double *data, int count) {
	int ret = l6msg_add_double_array_ptr(&msg, data, count);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
}

//get by index
int16_t Layer6Msg::getInt16At(int ix) {
	int16_t data = 0;
	if(l6msg_get_int16_at_index(&msg, ix, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

int32_t Layer6Msg::getInt32At (int ix) {
	int32_t data = 0;
	if(l6msg_get_int32_at_index(&msg, ix, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

int64_t Layer6Msg::getInt64At (int ix) {
	int64_t data = 0;
	if(l6msg_get_int64_at_index(&msg, ix, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

float Layer6Msg::getFloatAt (int ix) {
	float data = 0;
	if(l6msg_get_float_at_index(&msg, ix, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

double Layer6Msg::getDoubleAt(int ix) {
	double data = 0;
	if(l6msg_get_double_at_index(&msg, ix, &data) < 0) {
		throwExceptionIfEnabled();
	}
	return data;
}

int Layer6Msg::getStringAt(int ix, char *data, int len) {
	int ret = l6msg_get_string_at_index(&msg, ix, data, len);   
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

//get arrays
int Layer6Msg::getByteArrayAt (int ix, char *data, int offset, int count) {
	int ret = l6msg_get_byte_array_at_index(&msg, ix, data, offset, count);   
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getInt16ArrayAt (int ix, int16_t *data, int offset, int count) {
	int ret = l6msg_get_int16_array_at_index(&msg, ix, data, offset, count);   
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getInt32ArrayAt (int ix, int32_t  *data, int offset, int count) {
	int ret = l6msg_get_int32_array_at_index(&msg, ix, data, offset, count);   
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getInt64ArrayAt (int ix, int64_t  *data, int offset, int count) {
	int ret = l6msg_get_int64_array_at_index(&msg, ix, data, offset, count);   
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}
    
int Layer6Msg::getFloatArrayAt (int ix, float  *data, int offset, int count) {
	int ret = l6msg_get_float_array_at_index(&msg, ix, data, offset, count);   
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::getDoubleArrayAt(int ix, double *data, int offset, int count) {
	int ret = l6msg_get_double_array_at_index(&msg, ix, data, offset, count);   
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}


//get array pointers
const char* const Layer6Msg::getStringPtrAt(int ix) {
	const char *data = NULL;
	if(l6msg_get_string_ptr_at_index(&msg, ix, &data) < 0) {
		throwExceptionIfEnabled();
	}
	//return l6msg_getString(&msg, id, data, len);
	return (const char* const)data;
}

const char* const Layer6Msg::getByteArrayPtrAt(int ix, int *size) {
	char *data;
	if(l6msg_get_byte_array_ptr_at_index(&msg, ix, &data, size) < 0) {
		throwExceptionIfEnabled();
	}
	//return l6msg_getString(&msg, id, data, len);
	return (const char* const)data;
}

const int16_t* const Layer6Msg::getInt16ArrayPtrAt(int ix, int *count) {
	int16_t *data;
	if(l6msg_get_int16_array_ptr_at_index(&msg, ix, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	//return l6msg_getString(&msg, id, data, len);
	return (const int16_t* const)data;
}
	
const int32_t* const Layer6Msg::getInt32ArrayPtrAt(int ix, int *count) {
	int32_t *data;
	if(l6msg_get_int32_array_ptr_at_index(&msg, ix, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	//return l6msg_getString(&msg, id, data, len);
	return (const int32_t* const)data;
}

const int64_t* const Layer6Msg::getInt64ArrayPtrAt(int ix, int *count) {
	int64_t *data;
	if(l6msg_get_int64_array_ptr_at_index(&msg, ix, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	//return l6msg_getString(&msg, id, data, len);
	return (const int64_t* const)data;
}

const float* const Layer6Msg::getFloatArrayPtrAt(int ix, int *count) {
	float *data;
	if(l6msg_get_float_array_ptr_at_index(&msg, ix, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	//return l6msg_getString(&msg, id, data, len);
	return (const float* const)data;
}

const double* const Layer6Msg::getDoubleArrayPtrAt(int ix, int *count) {
	double *data;
	if(l6msg_get_double_array_ptr_at_index(&msg, ix, &data, count) < 0) {
		throwExceptionIfEnabled();
	}
	//return l6msg_getString(&msg, id, data, len);
	return (const double* const)data;
}

//std vector ops
void Layer6Msg::addString          (std::string                &data) { addString     ( (char*) data.c_str()); }
void Layer6Msg::addByteVector      (std::vector<char>          &data) { addByteArray  (&data[0], data.size()); }
void Layer6Msg::addInt16Vector     (std::vector<int16_t>       &data) { addInt16Array (&data[0], data.size()); }
void Layer6Msg::addShortVector     (std::vector<short>         &data) { addInt16Array (&data[0], data.size()); }
void Layer6Msg::addInt32Vector     (std::vector<int32_t>       &data) { addInt32Array (&data[0], data.size()); }
void Layer6Msg::addIntVector       (std::vector<int>           &data) { addInt32Array (&data[0], data.size()); }
void Layer6Msg::addInt64Vector     (std::vector<int64_t>       &data) { addInt64Array (&data[0], data.size()); }
void Layer6Msg::addLongVector      (std::vector<long long int> &data) { addInt64Array (&data[0], data.size()); }
void Layer6Msg::addFloatVector     (std::vector<float>         &data) { addFloatArray (&data[0], data.size()); }
void Layer6Msg::addDoubleVector    (std::vector<double>        &data) { addDoubleArray(&data[0], data.size()); }

void Layer6Msg::setString          (int fid,    std::string                &data) { setString(fid, (char*) data.c_str());       }
void Layer6Msg::setByteVector      (int fid,    std::vector<char>          &data) { setByteArray  (fid, &data[0], data.size()); }
void Layer6Msg::setInt16Vector     (int fid,    std::vector<int16_t>       &data) { setInt16Array (fid, &data[0], data.size()); }
void Layer6Msg::setShortVector     (int fid,    std::vector<short>         &data) { setInt16Array (fid, &data[0], data.size()); }
void Layer6Msg::setInt32Vector     (int fid,    std::vector<int32_t>       &data) { setInt32Array (fid, &data[0], data.size()); }
void Layer6Msg::setIntVector       (int fid,    std::vector<int>           &data) { setInt32Array (fid, &data[0], data.size()); }
void Layer6Msg::setInt64Vector     (int fid,    std::vector<int64_t>       &data) { setInt64Array (fid, &data[0], data.size()); }
void Layer6Msg::setLongVector      (int fid,    std::vector<long long int> &data) { setInt64Array (fid, &data[0], data.size()); }
void Layer6Msg::setFloatVector     (int fid,    std::vector<float>         &data) { setFloatArray (fid, &data[0], data.size()); }
void Layer6Msg::setDoubleVector    (int fid,    std::vector<double>        &data) { setDoubleArray(fid, &data[0], data.size()); }

void Layer6Msg::setString     (const char *key, std::string                &data) { setString(key, (char*) data.c_str());       }
void Layer6Msg::setByteVector (const char *key, std::vector<char>          &data) { setByteArray  (key, &data[0], data.size()); }
void Layer6Msg::setInt16Vector(const char *key, std::vector<int16_t>       &data) { setInt16Array (key, &data[0], data.size()); }
void Layer6Msg::setShortVector(const char *key, std::vector<short>         &data) { setInt16Array (key, &data[0], data.size()); }
void Layer6Msg::setInt32Vector(const char *key, std::vector<int32_t>       &data) { setInt32Array (key, &data[0], data.size()); }
void Layer6Msg::setIntVector  (const char *key, std::vector<int>           &data) { setInt32Array (key, &data[0], data.size()); }
void Layer6Msg::setInt64Vector(const char *key, std::vector<int64_t>       &data) { setInt64Array (key, &data[0], data.size()); }
void Layer6Msg::setLongVector (const char *key, std::vector<long long int> &data) { setInt64Array (key, &data[0], data.size()); }
void Layer6Msg::setFloatVector(const char *key, std::vector<float>         &data) { setFloatArray (key, &data[0], data.size()); }
void Layer6Msg::setDoubleVector(const char *key,std::vector<double>        &data) { setDoubleArray(key, &data[0], data.size()); }

void Layer6Msg::getString          (int fid,    std::string                &data) {
    const char* const s = getStringPtr(fid);
    if(s) {
        data.clear();
        data.append(s);
    }
    else
		throwExceptionIfEnabled();
}

void Layer6Msg::getByteVector      (int fid,    std::vector<char>          &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getByteArray(fid, &data[0], 0, count);
    }
}

void Layer6Msg::getInt16Vector     (int fid,    std::vector<int16_t>       &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt16Array(fid, &data[0], 0, count);
    }
}

void Layer6Msg::getShortVector     (int fid,    std::vector<short>         &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getShortArray(fid, &data[0], 0, count);
    }
}

void Layer6Msg::getInt32Vector     (int fid,    std::vector<int32_t>       &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt32Array(fid, &data[0], 0, count);
    }
}

void Layer6Msg::getIntVector       (int fid,    std::vector<int>           &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getIntArray(fid, &data[0], 0, count);
    }
}

void Layer6Msg::getInt64Vector     (int fid,    std::vector<int64_t>       &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt64Array(fid, &data[0], 0, count);
    }
}

void Layer6Msg::getLongVector      (int fid,    std::vector<long long int> &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getLongArray(fid, &data[0], 0, count);
    }
}

void Layer6Msg::getFloatVector     (int fid,    std::vector<float>         &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getFloatArray(fid, &data[0], 0, count);
    }
}

void Layer6Msg::getDoubleVector    (int fid,    std::vector<double>        &data) {
    int count = getFieldCount(fid);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getDoubleArray(fid, &data[0], 0, count);
    }
}


void Layer6Msg::getString(const char* key, std::string                &data) {
    const char* const s = getStringPtr(key);
    if(s) {
        data.clear();
        data.append(s);
    }
    else
		throwExceptionIfEnabled();
}

void Layer6Msg::getByteVector(const char* key, std::vector<char>          &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getByteArray(key, &data[0], 0, count);
    }
}

void Layer6Msg::getInt16Vector(const char* key, std::vector<int16_t>       &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt16Array(key, &data[0], 0, count);
    }
}

void Layer6Msg::getShortVector(const char* key, std::vector<short>         &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getShortArray(key, &data[0], 0, count);
    }
}

void Layer6Msg::getInt32Vector(const char* key, std::vector<int32_t>       &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt32Array(key, &data[0], 0, count);
    }
}

void Layer6Msg::getIntVector(const char* key, std::vector<int>           &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getIntArray(key, &data[0], 0, count);
    }
}

void Layer6Msg::getInt64Vector(const char* key, std::vector<int64_t>       &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt64Array(key, &data[0], 0, count);
    }
}

void Layer6Msg::getLongVector(const char* key, std::vector<long long int> &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getLongArray(key, &data[0], 0, count);
    }
}

void Layer6Msg::getFloatVector(const char* key, std::vector<float>         &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getFloatArray(key, &data[0], 0, count);
    }
}

void Layer6Msg::getDoubleVector(const char* key, std::vector<double>        &data) {
    int count = getFieldCount(key);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getDoubleArray(key, &data[0], 0, count);
    }
}

void Layer6Msg::getStringAt        (int index,    std::string                &data) {
    const char* const s = getStringPtrAt(index);
    if(s) {
        data.clear();
        data.append(s);
    }
    else
		throwExceptionIfEnabled();
}

void Layer6Msg::getByteVectorAt    (int index,  std::vector<char>          &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getByteArrayAt(index, &data[0], 0, count);
    }
}

void Layer6Msg::getInt16VectorAt   (int index,  std::vector<int16_t>       &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt16ArrayAt(index, &data[0], 0, count);
    }
}

void Layer6Msg::getShortVectorAt   (int index,  std::vector<short>         &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getShortArrayAt(index, &data[0], 0, count);
    }
}

void Layer6Msg::getInt32VectorAt   (int index,  std::vector<int32_t>       &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt32ArrayAt(index, &data[0], 0, count);
    }
}

void Layer6Msg::getIntVectorAt     (int index,  std::vector<int>           &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getIntArrayAt(index, &data[0], 0, count);
    }
}

void Layer6Msg::getInt64VectorAt   (int index,  std::vector<int64_t>       &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getInt64ArrayAt(index, &data[0], 0, count);
    }
}

void Layer6Msg::getLongVectorAt    (int index,  std::vector<long long int> &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getLongArrayAt(index, &data[0], 0, count);
    }
}

void Layer6Msg::getFloatVectorAt   (int index,  std::vector<float>         &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getFloatArrayAt(index, &data[0], 0, count);
    }
}

void Layer6Msg::getDoubleVectorAt  (int index,  std::vector<double>        &data) {
    int count = getFieldCountAt(index);
    if(count >= 0) {
        data.resize(count); //reserve memory, then copy directly to it
        getDoubleArrayAt(index, &data[0], 0, count);
    }
}


//field ops
int Layer6Msg::getNumFields() {
	return l6msg_get_num_fields(&msg);
}

int Layer6Msg::removeFieldAt(int index) {
	return l6msg_remove_field_at_index(&msg, index);
}

int Layer6Msg::removeField(int id) {
	return l6msg_remove_field(&msg, id);
}

int Layer6Msg::removeField(const char *key) {
	return l6msg_remove_field_named(&msg, key);
}

int Layer6Msg::getFieldSizeBytesAt(int index, int *size) {
	return l6msg_get_field_size_bytes_at_index(&msg, index, size);
}

int Layer6Msg::getFieldSizeBytes(int id, int *size) {
	return l6msg_get_field_size_bytes(&msg, id, size);
}

int Layer6Msg::getFieldSizeBytes(const char *key, int *size) {
	return l6msg_get_field_size_bytes_named(&msg, key, size);
}

int Layer6Msg::getFieldCountAt(int index, int *count) {
	return l6msg_get_field_count_at_index(&msg, index, count);
}

int Layer6Msg::getFieldCount(int id, int *count) {
	return l6msg_get_field_count(&msg, id, count);
}

int Layer6Msg::getFieldCount(const char *key, int *count) {
	return l6msg_get_field_count_named(&msg, key, count);
}

int Layer6Msg::getFieldUnitSizeAt(int index, int *size) {
	return l6msg_get_field_unit_size_at_index(&msg, index, size);
}

int Layer6Msg::getFieldUnitSize(int id, int *size) {
	return l6msg_get_field_unit_size(&msg, id, size);
}

int Layer6Msg::getFieldUnitSize(const char *key, int *size) {
	return l6msg_get_field_unit_size_named(&msg, key, size);
}

int Layer6Msg::getFieldType(int id, int *type) {
	return l6msg_get_field_type(&msg, id, type);
}

int Layer6Msg::getFieldType(const char *key, int *type) {
	return l6msg_get_field_type_named(&msg, key, type);
}

int Layer6Msg::getFieldTypeAt(int index, int *type) {
	return l6msg_get_field_type_at_index(&msg, index, type);
}

int Layer6Msg::getFieldIdAt(int index, int *id) {
	return l6msg_get_field_id_at_index(&msg, index, id);
}

int Layer6Msg::getFieldSizeBytesAt(int index) {
    int ret = -1;
    if(getFieldSizeBytesAt(index, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldSizeBytes(int fid) {
    int ret = -1;
    if(getFieldSizeBytes(fid, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldSizeBytes(const char *key) {
    int ret = -1;
    if(getFieldSizeBytes(key, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}
int Layer6Msg::getFieldCountAt(int index) {
    int ret = -1;
    if(getFieldCountAt(index, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldCount(int fid) {
    int ret = -1;
    if(getFieldCount(fid, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldCount(const char *key) {
    int ret = -1;
    if(getFieldCount(key, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}
int Layer6Msg::getFieldUnitSizeAt(int index) {
    int ret = -1;
    if(getFieldUnitSizeAt(index, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldUnitSize(int fid) {
    int ret = -1;
    if(getFieldUnitSize(fid, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldUnitSize(const char *key) {
    int ret = -1;
    if(getFieldUnitSize(key, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}
int Layer6Msg::getFieldTypeAt(int index) {
    int ret = -1;
    if(getFieldTypeAt(index, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldType(int fid) {
    int ret = -1;
    if(getFieldType(fid, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldType(const char *key) {
    int ret = -1;
    if(getFieldType(key, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldIdAt(int index) {
    int ret = -1;
    if(getFieldIdAt(index, &ret) < 0) {
		throwExceptionIfEnabled();
		return -1;
    }
    return ret;
}

int Layer6Msg::getFieldNameAt(int index, const char **name) {
	return l6msg_get_field_name_at_index(&msg, index, name);
}

const char* const Layer6Msg::getFieldNameAt(int index) {
	const char *name = NULL;
	if(l6msg_get_field_name_at_index(&msg, index, &name) < 0) {
		throwExceptionIfEnabled();
	}
	return (const char* const) name;
}

int Layer6Msg::isFieldArray(const char *key) {
    int ret = l6msg_is_field_array_named(&msg, key);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::isFieldArray(int fid) {
    int ret = l6msg_is_field_array(&msg, fid);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::isFieldArrayAt(int index) {
    int ret = l6msg_is_field_array_at_index(&msg, index);
	if(ret < 0) {
		throwExceptionIfEnabled();
	}
	return ret;
}

int Layer6Msg::isFieldNamed(int id) {
	return l6msg_is_field_named(&msg, id);
}

int Layer6Msg::isFieldNamedAt(int index) {
	return l6msg_is_field_named_at_index(&msg, index);
}
