package org.layer6;

import java.io.*;
import java.util.*;

public class Layer6 {

    public static final int L6_STATUS_OK                     = 0;
    public static final int L6_STATUS_ERROR                  = -1;
    public static final int L6_STATUS_WARNING                = 1;

    public static final int L6_DATATYPE_UNSIGNED_BYTE        = 1;
    public static final int L6_DATATYPE_UINT8                = 1;
    public static final int L6_DATATYPE_BYTE                 = 2;
    public static final int L6_DATATYPE_INT8                 = 2;
    public static final int L6_DATATYPE_UINT16               = 3;
    public static final int L6_DATATYPE_UNSIGNED_SHORT       = 3;
    public static final int L6_DATATYPE_SHORT                = 4;
    public static final int L6_DATATYPE_INT16                = 4;
    public static final int L6_DATATYPE_UINT32               = 5;
    public static final int L6_DATATYPE_UNSIGNED_INT         = 5;
    public static final int L6_DATATYPE_INT                  = 6;
    public static final int L6_DATATYPE_INT32                = 6;
    public static final int L6_DATATYPE_UINT64               = 7;
    public static final int L6_DATATYPE_UNSIGNED_LONG        = 7;
    public static final int L6_DATATYPE_INT64                = 8;
    public static final int L6_DATATYPE_LONG                 = 8;
    public static final int L6_DATATYPE_FLOAT                = 9;
    public static final int L6_DATATYPE_DOUBLE               = 10;

    public static final int L6_DATATYPE_STRING              = 16;
    public static final int L6_DATATYPE_UINT8_ARRAY         = 17;
    public static final int L6_DATATYPE_UNSIGNED_BYTES      = 17;
    public static final int L6_DATATYPE_BYTES               = 18;
    public static final int L6_DATATYPE_INT8_ARRAY          = 18;
    public static final int L6_DATATYPE_UINT16_ARRAY        = 19;
    public static final int L6_DATATYPE_UNSIGNED_SHORT_ARRAY= 19;
    public static final int L6_DATATYPE_INT16_ARRAY         = 20;
    public static final int L6_DATATYPE_SHORT_ARRAY         = 20;
    public static final int L6_DATATYPE_UINT32_ARRAY        = 21;
    public static final int L6_DATATYPE_UNSIGNED_INT_ARRAY  = 21;
    public static final int L6_DATATYPE_INT32_ARRAY         = 22;
    public static final int L6_DATATYPE_INT_ARRAY           = 22;
    public static final int L6_DATATYPE_UINT64_ARRAY        = 23;
    public static final int L6_DATATYPE_UNSIGNED_LONG_ARRAY = 23;
    public static final int L6_DATATYPE_INT64_ARRAY         = 24;
    public static final int L6_DATATYPE_LONG_ARRAY          = 24;
    public static final int L6_DATATYPE_FLOAT_ARRAY         = 25;
    public static final int L6_DATATYPE_DOUBLE_ARRAY        = 26;

    public static final int L6_DATATYPE_L6MSG               = 27;
    public static final int L6_DATATYPE_UTF8_STRING         = 28;
    public static final int L6_DATATYPE_EXTENSION           = 32;

    public static final int L6_FIELD_LENGTH_SCALAR          = 1;
    public static final int L6_FIELD_LENGTH_ARRAY           = 3;
    public static final int L6_FIELD_LENGTH_STRING          = 3;

    public static final int L6_ERR_MEM_ALLOC                = 32;
    public static final int L6_ERR_INSUFF_BUFFER_SIZE       = 33;
    public static final int L6_ERR_INCORRECT_MSG_SIZE       = 34;
    public static final int L6_ERR_FIELD_EXISTS             = 35;
    public static final int L6_ERR_FIELD_NOT_FOUND          = 36;
    public static final int L6_ERR_FIELDNAME_TOO_LONG       = 37;
    public static final int L6_ERR_FIELD_UNNAMED            = 38;
    public static final int L6_ERR_FIELD_NO_ID              = 39;
    public static final int L6_ERR_FIELD_ID_INVALID         = 40;
    public static final int L6_ERR_FIELD_TYPE_INCOMPATIBLE  = 41;
    public static final int L6_ERR_FIELD_MAX_COUNT_EXCEEDED = 42;
    public static final int L6_ERR_FIELD_MAX_SIZE_EXCEEDED  = 43;
    public static final int L6_ERR_MAX_NUM_FIELDS_EXCEEDED  = 44;
    public static final int L6_ERR_MAX_MSG_SIZE_EXCEEDED    = 45;
    public static final int L6_ERR_MSG_LOCKED               = 46;
    public static final int L6_ERR_RECURSIVE_SUB_MSG        = 47;
    public static final int L6_ERR_SUB_MSG_DESERIALIZE      = 48;
    public static final int L6_ERR_OFFSET_OUT_OF_BOUNDS     = 49;
    public static final int L6_ERR_UNHANDLED_FIELD_TYPE     = 50;
    public static final int L6_ERR_NOT_IMPLEMENTED          = 51;

    public static final int L6_WARN_INDEX_OUT_OF_BOUNDS     = 53;
    public static final int L6_WARN_FIELD_REPLACED          = 54;

    private static final int L6_ERR_CODES_MIN               = 32;
    private static final int L6_ERR_CODES_MAX               = 52;
    private static final int L6_ERR_CODES_OFFSET            = 28;

    static final int         L6_FLAG_FIELD_TYPE_MASK        = 0x1F;
    static final int         L6_PRIMITIVE_DATATYPE_MAX      = 12;
    static final int         L6_FLAG_FIELD_IS_EXTENSION     = 32;
    
    public static final int L6_MSG_FIELD_NAME_MAX_LEN      = 128;

    public static final String[] l6ErrorMsgs = {
        "No Error.",
        "Error: Unkown Error.",
        "Reserved.",
        "Reserved.",
        "Error: L6_ERR_MEM_ALLOC: Memory Allocation Failure.",
        "Error: L6_ERR_INSUFF_BUFFER_SIZE - Insufficient buffer size for seriliazation.",
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

    public static String getErrorMsg(int errorCode) {
        if(errorCode == 0) {
            return l6ErrorMsgs[0];
        }
        else if((errorCode > Layer6.L6_ERR_CODES_MIN) && (errorCode < Layer6.L6_ERR_CODES_MAX)) {
            return l6ErrorMsgs[errorCode - Layer6.L6_ERR_CODES_OFFSET];
        }
        return l6ErrorMsgs[1];
    }
    
    public static String getTypeString(int type) {
        String ret = "UNKNOWN";
        //if(type > 100) {
        //    type -= 100;
        //}
        switch(type & Layer6.L6_FLAG_FIELD_TYPE_MASK) {
        case Layer6.L6_DATATYPE_SHORT:
            ret = "short";
            break;
        case Layer6.L6_DATATYPE_INT32:
            ret = "int32";
            break;
/*        case Layer6.L6_DATATYPE_INT64:
            ret = "int64";
            break;
*/
        case Layer6.L6_DATATYPE_LONG:
            ret = "long";
            break;
        case Layer6.L6_DATATYPE_FLOAT:
            ret = "float";
            break;
        case Layer6.L6_DATATYPE_DOUBLE:
            ret = "double";
            break;
        case Layer6.L6_DATATYPE_STRING:
            ret = "string";
            break;
        case Layer6.L6_DATATYPE_BYTES:
            ret = "bytes";
            break;
        case Layer6.L6_DATATYPE_SHORT_ARRAY:
            ret = "short[]";
            break;
        case Layer6.L6_DATATYPE_INT32_ARRAY:
            ret = "int32[]";
            break;
        case Layer6.L6_DATATYPE_INT64_ARRAY:
            ret = "int64[]";
            break;
/*        case Layer6.L6_DATATYPE_LONG_ARRAY:
            ret = "long[]";
            break;
*/
        case Layer6.L6_DATATYPE_FLOAT_ARRAY:
            ret = "float[]";
            break;
        case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
            ret = "double[]";
            break;
/*        case Layer6.L6_DATATYPE_STRING_ARRAY:
            ret = "string[]";
            break;
*/
        case Layer6.L6_DATATYPE_L6MSG:
            ret = "l6msg";
            break;
        }
        return ret;
    }
}
