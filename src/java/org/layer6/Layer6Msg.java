package org.layer6;

import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.nio.ByteBuffer;

/*
 * Note that this code was written (and is maintained) for Java 1.4 (and even 
 * older, hence, the presence of the DataInput/OutputStream-based serialization
 * and deserialization implementations.) Hence the absence of typed collections.
 */

public class Layer6Msg {
    boolean     hasSubject;
    boolean     hasMetadata;
    boolean     deepCopy;
    //boolean    hasFieldnames;
    short       code;
    String      subject;
    int         totalLength;
    int         metadataLength;
    int         dataLength;
    int         errorCode;
    
    //tempalte stuff
    short       templateID;

    int         poolsize;
    int         idgen;

    List        qfields;
    List        qpool;
    List        subMsgs;

    //field names
    Map         htblFName;
    Map         htblFId;
    
    String      debugInfo = "";

    private static final int    L6_BASE_MSG_HDR_LENGTH     = 8;    
    
    private static final int    L6_FLAG_MSG_IS_TEMPLATE     = 32;

    private static final int    L6_FLAG_FIELD_IS_ARRAY      = 16;
    private static final int    L6_FLAG_FIELD_HAS_NAME      = 64;
    private static final int    L6_FLAG_FIELD_HAS_ID        = 128;
    
    private static final int    L6_MAX_NUM_FIELDS           = 255;
    private static final int    L6_FIELD_MAX_COUNT          = 65535;
    
    //serialization
    PipedOutputStream popStream;
    PipedInputStream  pipStream;
    
    static boolean useNIOBuffers = true;
    static {
        //test for presence of NIO
        try {
            useNIOBuffers = true;
            ByteBuffer buffer = ByteBuffer.allocate(1);
        }
        catch(java.lang.NoClassDefFoundError cnfEx) {
            useNIOBuffers = false;
        }
    }

    public Layer6Msg() {
        poolsize  = 5;
        init();
    }

    public Layer6Msg(int opt) {
        poolsize = opt;
        init();
    }
    
    private static final boolean DISALLOW_TRANSPARENT_OVERWRITE = false;

    private void throwExIfFieldExists(String name, int id, int index) throws Layer6Exception {
        if(name!=null) {
            if(htblFName.get(name) != null) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELD_EXISTS);
            }
        }
        if(id >= 0) {
            Integer Id = new Integer(id);
            if(htblFId.get(Id)!= null) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELD_EXISTS);
            }
        }
        if((index >= 0) && (index < qfields.size())) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_EXISTS);
        }
    }
    
    private Layer6MsgField createFieldWithMetadata(String name, int id, int type, int elemsize, int count) 
        throws Layer6Exception {
        if(qfields.size() >= Layer6Msg.L6_MAX_NUM_FIELDS) {
            throw new Layer6Exception(Layer6.L6_ERR_MAX_NUM_FIELDS_EXCEEDED);
        }

        Layer6MsgField field = new Layer6MsgField();
        field.mdlength = 1;    //sizeof id = sizeof type = byte
        field.id       = (short)id;

        if(field.id >= 0) {
            field.mdlength += 2;
            Integer Id = new Integer(field.id);
            htblFId.put(Id, field);
            /*idgen = (idgen > field.id ? idgen : field.id+1);                
            if(id < 0) {
                id = idgen;
            }*/
        }

        if(name != null) {
            if(name.length() > Layer6.L6_MSG_FIELD_NAME_MAX_LEN) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELDNAME_TOO_LONG);
            }

            field.namelen   = name.length() + 1;
            field.mdlength += (field.namelen + 1);    //strlen + 1 byte null term + 1 byte length)
            field.name      = name;
            htblFName.put(field.name, field);
        }

        field.type        = type & Layer6.L6_FLAG_FIELD_TYPE_MASK;
        if((field.type &  Layer6Msg.L6_FLAG_FIELD_IS_ARRAY) > 0) {
            field.mdlength += 2;        //size of count of array elements = short
            
            if(count > Layer6Msg.L6_FIELD_MAX_COUNT) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELD_MAX_COUNT_EXCEEDED);
            }
        }

        field.elemsize = elemsize;
        field.datalen  = elemsize * count;
        field.count    = count;
        field.size     = field.mdlength + field.datalen;

        return field;
    }

    private int setFieldInMsg(String name, int id, int index, int type, double data, int elemsize)
        throws Layer6Exception {
        Layer6MsgField field = null;
        int ret = 0;
        if(DISALLOW_TRANSPARENT_OVERWRITE) {
            throwExIfFieldExists(name, id, index);
        }
        
        field = getFieldInMsg(name, id, index, type, false);

        //this will always be null, unless we change the logic above to allow replacing fields
        if(field == null) {
            field = createFieldWithMetadata(name, id, type, elemsize, 1);
            qfields.add(field);
        }
        else {
            //undo field metadata for new data
            //updateAggregateLengthByDelta(-field.size, -field.mdlength, -field.datalen);
            /* totalLength -= field.size; metadataLength -= field.mdlength; dataLength -= field.datalen; */
            errorCode = Layer6.L6_WARN_FIELD_REPLACED;
            ret = 1;
        }

        field.nData     = data;

        //updateAggregateLengthByDelta(field.size, field.mdlength, field.datalen);
        /* totalLength += field.size; metadataLength += field.mdlength; dataLength += field.datalen; */
        return ret;
    }

    private int setFieldInMsg(String name, int id, int index, int type, Object data, int elemsize, int offset, int count, boolean copy) 
        throws Layer6Exception {
        Layer6MsgField field = null;
        int ret = 0;

        if(DISALLOW_TRANSPARENT_OVERWRITE) {
            throwExIfFieldExists(name, id, index);
        }

        field = getFieldInMsg(name, id, index, type, false);

        //this will always be null, unless we change the logic above to allow replacing fields
        if(field == null) {
            field = createFieldWithMetadata(name, id, type, elemsize, count);
            qfields.add(field);
        }
        else {
            //undo field metadata for new data
            if(field.count != (short)count) {
                field.count    = (short)count;
                field.datalen  = elemsize * count;
                field.count    = (short)count;
                field.size     = field.mdlength + field.datalen;
            }
            //updateAggregateLengthByDelta(-field.size, -field.mdlength, -field.datalen);
            /* totalLength -= field.size; metadataLength -= field.mdlength; dataLength -= field.datalen; */
            errorCode = Layer6.L6_WARN_FIELD_REPLACED;
            ret = 1;
        }

        if(copy) {
            try {
                field.offset = 0;
                if(type == Layer6.L6_DATATYPE_STRING) {
                    field.data = data.toString();
                }
                else if(type == Layer6.L6_DATATYPE_L6MSG) {
                    field.data = ((Layer6Msg)data).dup();
                    subMsgs.add(field.data);
                }
                else {
                    switch(type) {
                        case Layer6.L6_DATATYPE_BYTES:
                            field.data = new byte[count];
                            break;

                        case Layer6.L6_DATATYPE_SHORT_ARRAY:
                            field.data = new short[count];
                            break;

                        case Layer6.L6_DATATYPE_INT32_ARRAY:
                            field.data = new int[count];
                            break;

                        //case Layer6.L6_DATATYPE_INT64_ARRAY:
                        case Layer6.L6_DATATYPE_LONG_ARRAY:
                            field.data = new long[count];
                            break;

                        case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                            field.data = new float[count];
                            break;

                        case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                            field.data = new double[count];
                            break;

                        case Layer6.L6_DATATYPE_L6MSG:
                            field.data = new double[count];
                            break;

                        default:
                            break;
                    }
                    System.arraycopy(data, offset, field.data, 0, count);
                }
            }
            catch(ClassCastException ccEx) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELD_TYPE_INCOMPATIBLE);
            }
            catch(ArrayStoreException asEx) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELD_TYPE_INCOMPATIBLE);
            }
            catch(IndexOutOfBoundsException ioobEx) {
                throw new Layer6Exception(Layer6.L6_WARN_INDEX_OUT_OF_BOUNDS);
            }
        }
        else {
            field.data = data;
            field.offset = offset;
        }
        //updateAggregateLengthByDelta(field.size, field.mdlength, field.datalen);
        return ret;
    }
    
    private void updateAggregateLengthByDelta(int fieldSize, int fieldMdlength, int fieldDatalen ) {
        totalLength     += fieldSize;
        metadataLength  += fieldMdlength;
        dataLength      += fieldDatalen;
    }
    
    private Layer6MsgField getFieldInMsg(String name, int id, int index, int type, boolean throwIfNotFound) 
        throws Layer6Exception {
        Layer6MsgField field = null;
        if((index >= 0) && (index < qfields.size())) {
            field = (Layer6MsgField)qfields.get(index);
        }
        else if(id >= 0) {
            Integer Id = new Integer(id);
            field = (Layer6MsgField)htblFId.get(Id);
        }
        else if(name != null) {
            field = (Layer6MsgField)htblFName.get(name);
        }

        if(field != null) {
            if(field.type != type) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELD_TYPE_INCOMPATIBLE);
            }
        }
        else if(throwIfNotFound) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field;       
    }

    private double getFieldDataInMsg(String name, int id, int index, int type)
        throws Layer6Exception {
        return getFieldInMsg(name, id, index, type, true).nData;
    }

    private Object getFieldDataInMsg(String name, int id, int index, int type, 
            Object data, int offset, int count, boolean copy)
        throws Layer6Exception {
        Object ret = null;
        Layer6MsgField field = getFieldInMsg(name, id, index, type, true);

        if(copy || (count < 0)) {
            try {
                if(type == Layer6.L6_DATATYPE_STRING) {
                    ret = field.data.toString();
                }
                else if(type == Layer6.L6_DATATYPE_L6MSG) {
                    Layer6Msg subMsg = (Layer6Msg)field.data;
                    ret = subMsg.dup();
                }
                else if(data != null) {
                    //check if it is an array
                    int n = 0;
                    ret = data;
                    switch(type) {
                        case Layer6.L6_DATATYPE_BYTES:
                            n = ((byte[])data).length;
                            break;
    
                        case Layer6.L6_DATATYPE_SHORT_ARRAY:
                            n = ((short[])data).length;
                            break;
    
                        case Layer6.L6_DATATYPE_INT32_ARRAY:
                            n = ((int[])data).length;
                            break;
    
                        //case Layer6.L6_DATATYPE_INT64_ARRAY:
                        case Layer6.L6_DATATYPE_LONG_ARRAY:
                            n = ((long[])data).length;
                            break;
    
                        case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                            n = ((float[])data).length;
                            break;
    
                        case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                            n = ((float[])data).length;
                            break;
    
                        default:
                            n = 0;
                            break;
                    }
                    if(n < count) {
                        count = n;
                    }
                    System.arraycopy(field.data, offset, ret, 0, count);
                }
                else {
                    switch(type) {
                        case Layer6.L6_DATATYPE_BYTES:
                            ret = new byte[field.count];
                            break;

                        case Layer6.L6_DATATYPE_SHORT_ARRAY:
                            ret = new short[field.count];
                            break;

                        case Layer6.L6_DATATYPE_INT32_ARRAY:
                            ret = new int[field.count];
                            break;

                        //case Layer6.L6_DATATYPE_INT64_ARRAY:
                        case Layer6.L6_DATATYPE_LONG_ARRAY:
                            ret = new long[field.count];
                            break;

                        case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                            ret = new float[field.count];
                            break;

                        case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                            ret = new double[field.count];
                            break;

                        default:
                            break;
                    }
                    if(count < 0) {
                        count = field.count;
                    }
                    System.arraycopy(field.data, offset, ret, 0, count);
                }
            }
            catch(ClassCastException ccEx) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELD_TYPE_INCOMPATIBLE);
            }
            catch(ArrayStoreException asEx) {
                throw new Layer6Exception(Layer6.L6_ERR_FIELD_TYPE_INCOMPATIBLE);
            }
            catch(IndexOutOfBoundsException ioobEx) {
                throw new Layer6Exception(Layer6.L6_WARN_INDEX_OUT_OF_BOUNDS);
            }
        }
        else {
            ret = field.data;
        }
        return ret;
    }

    private void init() {
        qfields   = new ArrayList(poolsize);
        subMsgs   = new ArrayList(0);
        qpool     = new ArrayList(poolsize);
        htblFName = new HashMap(poolsize);
        htblFId   = new HashMap(poolsize);
        
        if(!useNIOBuffers) {
            popStream = new PipedOutputStream();
            pipStream = new PipedInputStream();
            try {
                popStream.connect(pipStream);
            }
            catch(IOException ioEx) {
            }
        }

        reset();
    }
    
    //KISS by using small switch/case rather than a cumbersome OO inheritance tree
    private void setFieldLengthByType(Layer6MsgField field) {
        switch(field.type) {
            case Layer6.L6_DATATYPE_UINT16:
            case Layer6.L6_DATATYPE_UINT16_ARRAY:
            case Layer6.L6_DATATYPE_SHORT:
            case Layer6.L6_DATATYPE_SHORT_ARRAY:
                //field.nData = (double)diStream.readShort();
                field.elemsize = 2;
                break;

            case Layer6.L6_DATATYPE_UINT32:
            case Layer6.L6_DATATYPE_UINT32_ARRAY:
            case Layer6.L6_DATATYPE_INT32:
            case Layer6.L6_DATATYPE_INT32_ARRAY:
                field.elemsize = 4;
                break;

            //case Layer6.L6_DATATYPE_INT64:
            case Layer6.L6_DATATYPE_UINT64:
            case Layer6.L6_DATATYPE_UINT64_ARRAY:            case Layer6.L6_DATATYPE_LONG:
            //case Layer6.L6_DATATYPE_INT64_ARRAY:
            case Layer6.L6_DATATYPE_LONG_ARRAY:
                field.elemsize = 8;
                break;

            case Layer6.L6_DATATYPE_FLOAT:
            case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                field.elemsize = 4;
                break;

            case Layer6.L6_DATATYPE_DOUBLE:
            case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                field.elemsize = 8;
                break;

            case Layer6.L6_DATATYPE_UINT8_ARRAY:
            case Layer6.L6_DATATYPE_STRING:
            case Layer6.L6_DATATYPE_BYTES:
            case Layer6.L6_DATATYPE_L6MSG:
            default:
                //field.nData = (double)diStream.readShort();
                field.elemsize = 1;
                break;
        }
    }
    
    private int serializeHeaderUsingNIOBuffer(ByteBuffer doBuffer, int mdLen) throws Layer6Exception {
        int start = doBuffer.position();
        int length = doBuffer.remaining();
        if(length < mdLen) {
            debugInfo = mdLen +" > "  + length;
            throw new Layer6Exception(Layer6.L6_ERR_INSUFF_BUFFER_SIZE, debugInfo);
        }
        int nfields = qfields.size();

        if((code & L6_FLAG_MSG_IS_TEMPLATE) > 0) {
            throw new Layer6Exception(Layer6.L6_ERR_NOT_IMPLEMENTED);
        }
        doBuffer.put((byte)(code & 0xFF));
        doBuffer.put((byte)(nfields & 0xFF));
        doBuffer.putShort((short)0);
        doBuffer.putShort((short)(totalLength & 0xFFFF));
        doBuffer.putShort((short)(metadataLength & 0xFFFF));
        if((code & L6_FLAG_MSG_IS_TEMPLATE) > 0) {
            doBuffer.putShort(templateID);    //should never execute
        }

        //write metadata
        for(int i = 0; i < nfields; i++) {
            //write metadata
            Layer6MsgField field = (Layer6MsgField)qfields.get(i);
            int type = field.type | ((field.name != null) ? Layer6Msg.L6_FLAG_FIELD_HAS_NAME : 0)
                                  | ((field.id   >= 0)    ? Layer6Msg.L6_FLAG_FIELD_HAS_ID   : 0);
            doBuffer.put((byte)type);

            if((field.type &  Layer6Msg.L6_FLAG_FIELD_IS_ARRAY) > 0) {
                //arrays - write count
                doBuffer.putShort((short)(field.count & 0xFFFF));
            }
            if(field.id   >= 0) {
                doBuffer.putShort(field.id);
            }            
            if(field.name!=null) {
                doBuffer.put((byte)(field.namelen));
                doBuffer.put(field.name.getBytes());
                doBuffer.put((byte)0);        //null terminator
            }
        }
        return (doBuffer.position() - start);
    }
    
    private int serializeBodyUsingNIOBuffer(ByteBuffer doBuffer, int totalLen) throws Layer6Exception {
        int length = doBuffer.remaining();
        int start = doBuffer.position();
        if(length < totalLen) {
            debugInfo = totalLen +" > "  + length;
            throw new Layer6Exception(Layer6.L6_ERR_INSUFF_BUFFER_SIZE, debugInfo);
        }

        int nfields = qfields.size();

        //check written vs. metadatalength

        for(int i = 0; i < nfields; i++) {
            Layer6MsgField field = (Layer6MsgField)qfields.get(i);
            //serialize fields
            int now = doBuffer.position();
            if((field.type &  Layer6Msg.L6_FLAG_FIELD_IS_ARRAY) == 0) {
                switch(field.type) {
                    case Layer6.L6_DATATYPE_BYTE:
                        doBuffer.put((byte)field.nData);
                        break;

                    case Layer6.L6_DATATYPE_SHORT:
                        doBuffer.putShort((short)field.nData);
                        break;

                    case Layer6.L6_DATATYPE_INT32:
                        doBuffer.putInt((int)field.nData);
                        break;

                    //case Layer6.L6_DATATYPE_INT64:
                    case Layer6.L6_DATATYPE_LONG:
                        doBuffer.putLong((long)field.nData);
                        break;

                    case Layer6.L6_DATATYPE_FLOAT:
                        doBuffer.putFloat((float)field.nData);
                        break;

                    case Layer6.L6_DATATYPE_DOUBLE:
                        doBuffer.putDouble(field.nData);
                        break;

                    default:
                           throw new Layer6Exception(Layer6.L6_ERR_UNHANDLED_FIELD_TYPE);
                }
            }
            else
            {
                switch(field.type)
                {
                    case Layer6.L6_DATATYPE_BYTES:
                        byte [] barr = (byte []) field.data;
                        doBuffer.put(barr, field.offset, field.count);
                        break;

                    case Layer6.L6_DATATYPE_STRING:
                        doBuffer.put(((String)field.data).getBytes());
                        doBuffer.put((byte)0);    //null terminator
                        break;

                    case Layer6.L6_DATATYPE_SHORT_ARRAY:
                        short [] sarr = (short []) field.data;
                        for(int j = 0; j < sarr.length; j++) {
                            doBuffer.putShort(sarr[j]);
                        }
                        break;

                    case Layer6.L6_DATATYPE_INT32_ARRAY:
                        int [] iarr = (int []) field.data;
                        for(int j = 0; j < iarr.length; j++) {
                            doBuffer.putInt(iarr[j]);
                        }
                        break;

                    //case Layer6.L6_DATATYPE_INT64_ARRAY:
                    case Layer6.L6_DATATYPE_LONG_ARRAY:
                        long [] larr = (long []) field.data;
                        for(int j = 0; j < larr.length; j++) {
                            doBuffer.putLong(larr[j]);
                        }
                        break;

                    case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                        float [] farr = (float []) field.data;
                        for(int j = 0; j < farr.length; j++) {
                            doBuffer.putFloat(farr[j]);
                        }
                        break;

                    case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                        double [] darr = (double []) field.data;
                        for(int j = 0; j < darr.length; j++) {
                            doBuffer.putDouble(darr[j]);
                        }
                        break;
                        
                    case Layer6.L6_DATATYPE_L6MSG:
                        Layer6Msg subMsg = (Layer6Msg)field.data;
                        subMsg.serializeUsingNIOBuffer(doBuffer);
                        /*
                        //get current position
                        int subMsgOffset = doBuffer.position();
                        int subMsgLen = subMsg.size();
                        subMsg.serialize(buffer, subMsgOffset, subMsgLen);
                        //now reset current position taking into account recently serialized msg
                        doBuffer.position(subMsgOffset + subMsgLen);
                        */
                        break;

                    //extensions and unsigned types not handled
                    case Layer6.L6_DATATYPE_EXTENSION:
                    default:
                           throw new Layer6Exception(Layer6.L6_ERR_UNHANDLED_FIELD_TYPE);
                }
            }
        }
        return (doBuffer.position() - start);
    }
    
    private int serializeUsingNIOBuffer(ByteBuffer doBuffer) throws Layer6Exception {
        int ret = 0;
        errorCode = 0;
        totalLength = getSizeInBytes();
        ret = serializeHeaderUsingNIOBuffer(doBuffer, metadataLength);
    
        ret += serializeBodyUsingNIOBuffer(doBuffer, totalLength - ret);
        
        return ret;
    }

    private int deserializeHeaderUsingNIOBuffer(ByteBuffer diBuffer, int length) throws Layer6Exception {
        int nfields = 0;
        int start   = diBuffer.position();
        if(length < L6_BASE_MSG_HDR_LENGTH) {
            throw new Layer6Exception(Layer6.L6_ERR_INCORRECT_MSG_SIZE);
        }
        //diBuffer.limit(diBuffer.position() + Layer6.L6_BASE_MSG_HDR_LENGTH);

        code = (short)(diBuffer.get() & (short)0xFF);
        if((code & L6_FLAG_MSG_IS_TEMPLATE) > 0) {
            debugInfo = "Template flag set in " + code;
            throw new Layer6Exception(Layer6.L6_ERR_NOT_IMPLEMENTED, debugInfo);
        }
        nfields = (diBuffer.get() & (int)0xFF);
        short unused   = diBuffer.getShort(); 
        totalLength    = (diBuffer.getShort() & (int)0xFFFF);
        metadataLength = (diBuffer.getShort() & (int)0xFFFF);

        if(metadataLength > length) {
            debugInfo = code+","+nfields+","+unused+","+totalLength+","+metadataLength +" > "  + length;
            throw new Layer6Exception(Layer6.L6_ERR_INSUFF_BUFFER_SIZE, debugInfo);
        }
        
        //diBuffer.limit(diBuffer.position() + totalLength);

        //deserialize metadata
        int left = length;
        for(int i = 0; i < nfields; i++) {
            Layer6MsgField field = new Layer6MsgField();
            field.name     = null;
            field.count    = 1;
            field.mdlength = 1;

            //read metadata
            field.type = diBuffer.get();

            if((field.type &  Layer6Msg.L6_FLAG_FIELD_IS_ARRAY) > 0) {
                //read count
                field.count = (diBuffer.getShort() & (int)0xFFFF);
                field.mdlength += 2;
            }

            if((field.type & Layer6Msg.L6_FLAG_FIELD_HAS_ID) > 0) {
                field.id = diBuffer.getShort();
                field.mdlength += 2;
                htblFId.put(new Integer(field.id), field);
            }

            if((field.type & Layer6Msg.L6_FLAG_FIELD_HAS_NAME) > 0) {
                //field is named
                field.namelen = diBuffer.get();
                if((field.namelen) > left) {
                    throw new Layer6Exception(Layer6.L6_ERR_INSUFF_BUFFER_SIZE);
                }
                byte [] name = new byte[field.namelen];
                diBuffer.get(name);
                field.name = new String(name, 0, (field.namelen - 1));
                field.mdlength += (field.namelen + 1);
                htblFName.put(field.name, field);
            }
            field.type &= Layer6.L6_FLAG_FIELD_TYPE_MASK;    //Layer6Msg.L6_FLAG_FIELD_HAS_NAME;
            setFieldLengthByType(field);

            field.datalen= field.count * field.elemsize;
            field.size = field.mdlength + field.datalen;
            qfields.add(field);
            left -= field.mdlength;
        }
        return (diBuffer.position() - start);
    }
    
    //deserialize fields
    private int deserializeBodyUsingNIOBuffer(ByteBuffer diBuffer, int nfields, int length) throws Layer6Exception {
        int start = diBuffer.position();
        int left = diBuffer.remaining();
        //TODO: check if start matches metadataLength

        if(left < length) {
            debugInfo = "nfields="+nfields+", start="+start+", left="+left +" < length="  + length;
            throw new Layer6Exception(Layer6.L6_ERR_INCORRECT_MSG_SIZE, debugInfo);
        }
        for(int i = 0; i < nfields; i++) {
            Layer6MsgField field = (Layer6MsgField)qfields.get(i);
            if(field.datalen > left) {
                debugInfo = "nfield="+i+"/"+nfields+", datalen="+field.datalen+" > left="+left;
                throw new Layer6Exception(Layer6.L6_ERR_INCORRECT_MSG_SIZE);
            }
            //read data
            switch(field.type) {
                case Layer6.L6_DATATYPE_BYTE:
                    field.nData = (byte)diBuffer.get();
                    break;

                case Layer6.L6_DATATYPE_BYTES:
                    byte [] barr = new byte[field.count];
                    field.data = barr;
                    diBuffer.get(barr);
                    break;

                case Layer6.L6_DATATYPE_STRING:
                    byte [] starr = new byte[field.count];
                    diBuffer.get(starr);
                    field.data = new String(starr, 0, (field.count - 1));
                    break;

                case Layer6.L6_DATATYPE_SHORT:
                    field.nData = (double)diBuffer.getShort();
                    break;

                case Layer6.L6_DATATYPE_SHORT_ARRAY:
                    short [] sarr = new short[field.count];
                    field.data = sarr;
                    for(int j = 0; j < field.count; j++) {
                        sarr[j] = diBuffer.getShort();
                    }
                    break;

                case Layer6.L6_DATATYPE_INT32:
                    field.nData = (double)diBuffer.getInt();
                    break;

                case Layer6.L6_DATATYPE_INT32_ARRAY:
                    int [] iarr = new int[field.count];
                    field.data = iarr;
                    for(int j = 0; j < field.count; j++) {
                        iarr[j] = diBuffer.getInt();
                    }
                    break;

                //case Layer6.L6_DATATYPE_INT64:
                case Layer6.L6_DATATYPE_LONG:
                    field.nData = (double)diBuffer.getLong();
                    break;

                //case Layer6.L6_DATATYPE_INT64_ARRAY:
                case Layer6.L6_DATATYPE_LONG_ARRAY:
                    long [] larr = new long[field.count];
                    field.data = larr;
                    for(int j = 0; j < field.count; j++) {
                        larr[j] = diBuffer.getLong();
                    }
                    break;

                case Layer6.L6_DATATYPE_FLOAT:
                    field.nData = (double)diBuffer.getFloat();
                    break;

                case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                    float [] farr = new float[field.count];
                    field.data = farr;
                    for(int j = 0; j < field.count; j++) {
                        farr[j] = diBuffer.getFloat();
                    }
                    break;

                case Layer6.L6_DATATYPE_DOUBLE:
                    field.nData = diBuffer.getDouble();
                    break;

                case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                    double [] darr = new double[field.count];
                    field.data = darr;
                    for(int j = 0; j < field.count; j++) {
                        darr[j] = diBuffer.getDouble();
                    }
                    break;
                    
                case Layer6.L6_DATATYPE_L6MSG:
                    Layer6Msg subMsg = new Layer6Msg();
                    subMsg.deserializeUsingNIOBuffer(diBuffer);
                    /*
                    //get current position
                    int subMsgOffset = diBuffer.position();
                    int subMsgLen = field.size;
                    subMsg.deserialize(buffer, subMsgOffset, subMsgLen);
                    //now reset current position taking into account recently serialized msg
                    diBuffer.position(subMsgOffset + subMsgLen);
                    */
                    field.data = subMsg;
                    subMsgs.add(subMsg);
                    break;

                default:
                    throw new Layer6Exception(Layer6.L6_ERR_UNHANDLED_FIELD_TYPE);
            }
            left -= field.datalen;
        }
        return (diBuffer.position() - start);
    }
    
    private int deserializeUsingNIOBuffer(ByteBuffer diBuffer) throws Layer6Exception {
        errorCode = 0;
        int length = diBuffer.remaining();
        int ret = deserializeHeaderUsingNIOBuffer(diBuffer, length);

        ret += deserializeBodyUsingNIOBuffer(diBuffer, qfields.size(), (totalLength - metadataLength));

        return ret;
    }
    
    private int serializeUsingDataStreamPipes(byte [] buffer, int offset, int length) throws Layer6Exception {
        int ret = 0;
        int i = 0;
        int j = 0;
        int nfields = 0;

        Layer6MsgField field;
        errorCode = 0;
        totalLength = getSizeInBytes();

        if(length < totalLength) {
            throw new Layer6Exception(Layer6.L6_ERR_INSUFF_BUFFER_SIZE);
        }
        try {
            DataOutputStream doStream = new DataOutputStream(popStream);
            if(templateID >= 0) {
                //code |= L6_FLAG_MSG_IS_TEMPLATE;
                debugInfo = "Template flag set in"+code;
                throw new Layer6Exception(Layer6.L6_ERR_NOT_IMPLEMENTED);
            }
            doStream.writeByte((byte)(code & 0xFF));
            doStream.writeByte(qfields.size());
            doStream.writeShort(0);
            doStream.writeShort((short)(totalLength & 0xFFFF));
            doStream.writeShort((short)(metadataLength & 0xFFFF));
            if((code & L6_FLAG_MSG_IS_TEMPLATE) > 0) {
                doStream.writeShort(templateID);
            }

            nfields = qfields.size();

            //write metadata
            for(i = 0; i < nfields; i++) {
                field = (Layer6MsgField)qfields.get(i);
                //write metadata
                int type = field.type | ((field.name != null) ? Layer6Msg.L6_FLAG_FIELD_HAS_NAME : 0)
                                      | ((field.id   >= 0)    ? Layer6Msg.L6_FLAG_FIELD_HAS_ID   : 0);
                doStream.writeByte(type);
                if((field.type &  Layer6Msg.L6_FLAG_FIELD_IS_ARRAY) > 0) {
                    //arrays - write count
                    doStream.writeShort(field.count);
                }
                if(field.id >= 0) {
                    doStream.writeShort(field.id);
                }                if(field.name!=null) {
                    doStream.writeByte(field.namelen);
                    doStream.writeBytes(field.name);
                    doStream.writeByte(0);        //null terminator
                }
            }

            //check written vs. metadatalength
            for(i = 0; i < nfields; i++) {
                field = (Layer6MsgField)qfields.get(i);
                //serialize fields
                if((field.type &  Layer6Msg.L6_FLAG_FIELD_IS_ARRAY) == 0) {
                    switch(field.type) {
                        case Layer6.L6_DATATYPE_BYTE:
                            doStream.write((byte)field.nData);
                            break;
                        case Layer6.L6_DATATYPE_SHORT:
                            doStream.writeShort((int)field.nData);
                            break;

                        case Layer6.L6_DATATYPE_INT32:
                            doStream.writeInt((int)field.nData);
                            break;

                        //case Layer6.L6_DATATYPE_INT64:
                        case Layer6.L6_DATATYPE_LONG:
                            doStream.writeLong((long)field.nData);
                            break;

                        case Layer6.L6_DATATYPE_FLOAT:
                            doStream.writeFloat((float)field.nData);
                            break;

                        case Layer6.L6_DATATYPE_DOUBLE:
                            doStream.writeDouble(field.nData);
                            break;

                        default:
                            //do nothing
                            break;
                    }
                }
                else
                {
                    switch(field.type)
                    {
                        case Layer6.L6_DATATYPE_BYTES:
                            byte [] barr = (byte []) field.data;
                            doStream.write(barr, field.offset, field.count);
                            break;

                        case Layer6.L6_DATATYPE_STRING:
                            doStream.writeBytes((String)field.data);
                            doStream.writeByte(0);    //null terminator
                            break;

                        case Layer6.L6_DATATYPE_SHORT_ARRAY:
                            short [] sarr = (short []) field.data;
                            for(j = 0; j < sarr.length; j++) {
                                doStream.writeShort(sarr[j]);
                            }
                            break;

                        case Layer6.L6_DATATYPE_INT32_ARRAY:
                            int [] iarr = (int []) field.data;
                            for(j = 0; j < iarr.length; j++) {
                                doStream.writeInt(iarr[j]);
                            }
                            break;
                            
                        case Layer6.L6_DATATYPE_L6MSG:
                            Layer6Msg subMsg = (Layer6Msg)field.data; 
                            //get current position
                            int subMsgLen = subMsg.size();
                            byte [] l6marr = new byte[subMsgLen];                            
                            subMsg.serialize(l6marr, 0, subMsgLen);
                            //now reset current position taking into account recently serialized msg
                            doStream.write(l6marr,  0, subMsgLen);
                            break;

                        //case Layer6.L6_DATATYPE_INT64_ARRAY:
                        case Layer6.L6_DATATYPE_LONG_ARRAY:
                            long [] larr = (long []) field.data;
                            for(j = 0; j < larr.length; j++) {
                                doStream.writeLong(larr[j]);
                            }
                            break;

                        case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                            float [] farr = (float []) field.data;
                            for(j = 0; j < farr.length; j++) {
                                doStream.writeFloat(farr[j]);
                            }
                            break;

                        case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                            double [] darr = (double []) field.data;
                            for(j = 0; j < darr.length; j++) {
                                doStream.writeDouble(darr[j]);
                            }
                            break;

                        default:
                            throw new Layer6Exception(Layer6.L6_ERR_UNHANDLED_FIELD_TYPE);
                    }
                }
            }

            int len = doStream.size();
            //now flush
            doStream.flush();
            ret = 0;
            int left = len;
            while(left > 0) {
                len = pipStream.read(buffer, 0, left);
                ret+=len;
                left-=len;
            }
            //popStream.close();
        }
        catch(IOException ioEx) {
            throw new Layer6Exception(-1, ioEx);
        }
        return ret;
    }
    
    private int deserializeUsingDataStreamPipes(byte [] buffer, int offset, int length) throws Layer6Exception {
        int i = 0;
        int j = 0;
        int nfields = 0;
        errorCode = 0;

        int ret = 0;

        byte     type;
        int      len;
        int      id;
        int      count;

        Layer6MsgField field;

        if(length < L6_BASE_MSG_HDR_LENGTH) {
            throw new Layer6Exception(Layer6.L6_ERR_INCORRECT_MSG_SIZE);
        }

        try {
            DataInputStream diStream = new DataInputStream(pipStream);
            popStream.write(buffer, offset, L6_BASE_MSG_HDR_LENGTH);
            //now flush
            popStream.flush();

            code = (byte)diStream.readUnsignedByte();
            if((code & L6_FLAG_MSG_IS_TEMPLATE) > 0) {
                debugInfo = "Template flag set in " + code;
                throw new Layer6Exception(Layer6.L6_ERR_NOT_IMPLEMENTED);
            }
            nfields = diStream.readUnsignedByte();
            short unused   = diStream.readShort();
            totalLength    = diStream.readUnsignedShort();
            metadataLength = diStream.readUnsignedShort();
            if((nfields > 0) && ((code & L6_FLAG_MSG_IS_TEMPLATE) > 0)) {
                templateID = diStream.readShort();
            }

            popStream.write(buffer, (offset+L6_BASE_MSG_HDR_LENGTH), (totalLength-L6_BASE_MSG_HDR_LENGTH));
            //now flush
            popStream.flush();

            if(totalLength > length) {
                throw new Layer6Exception(Layer6.L6_ERR_INSUFF_BUFFER_SIZE);
            }

            //deserialize metadata
            for(i = 0; i < nfields; i++) {
                field         = new Layer6MsgField();
                field.name  = null;
                field.count = 1;
                field.mdlength = 1;

                //read metadata
                field.type = diStream.readUnsignedByte();

                if((field.type &  Layer6Msg.L6_FLAG_FIELD_IS_ARRAY) > 0) {
                    //read count
                    field.count = diStream.readShort();
                    field.mdlength += 2;
                }

                  if((field.type & Layer6Msg.L6_FLAG_FIELD_HAS_ID) > 0) {
                    field.id   = diStream.readShort();
                    field.mdlength += 2;
                     htblFId.put(new Integer(field.id), field);
                  }
                if((field.type & Layer6Msg.L6_FLAG_FIELD_HAS_NAME) > 0) {
                    //field is named
                    field.namelen = diStream.readUnsignedByte();
                    byte [] name = new byte[field.namelen];
                    diStream.read(name);
                    field.name = new String(name, 0, (field.namelen - 1));
                    field.mdlength += (field.namelen + 1);
                    htblFName.put(field.name, field);
                }
                field.type &= Layer6.L6_FLAG_FIELD_TYPE_MASK;
                setFieldLengthByType(field);

                field.datalen= field.count * field.elemsize;
                field.size = field.mdlength + field.datalen;
                qfields.add(field);
            }

            //deserialize fields
            for(i = 0; i < nfields; i++) {
                field = (Layer6MsgField)qfields.get(i);
                //read data
                switch(field.type) {
                    case Layer6.L6_DATATYPE_BYTE:
                        field.nData = (byte)diStream.read();
                        break;
                    case Layer6.L6_DATATYPE_BYTES:
                        byte [] barr = new byte[field.count];
                        field.data = barr;
                        diStream.read(barr);
                        break;

                    case Layer6.L6_DATATYPE_STRING:
                        byte [] starr = new byte[field.count];
                        diStream.read(starr);
                        field.data = new String(starr, 0, (field.count - 1));
                        break;

                    case Layer6.L6_DATATYPE_SHORT:
                        field.nData = (double)diStream.readShort();
                        break;

                    case Layer6.L6_DATATYPE_SHORT_ARRAY:
                        short [] sarr = new short[field.count];
                        field.data = sarr;
                        for(j = 0; j < field.count; j++) {
                            sarr[j] = diStream.readShort();
                        }
                        break;

                    case Layer6.L6_DATATYPE_INT32:
                        field.nData = (double)diStream.readInt();
                        break;

                    case Layer6.L6_DATATYPE_INT32_ARRAY:
                        int [] iarr = new int[field.count];
                        field.data = iarr;
                        for(j = 0; j < field.count; j++) {
                            iarr[j] = diStream.readInt();
                        }
                        break;

                    //case Layer6.L6_DATATYPE_INT64:
                    case Layer6.L6_DATATYPE_LONG:
                        field.nData = (double)diStream.readLong();
                        break;

                    //case Layer6.L6_DATATYPE_INT64_ARRAY:
                    case Layer6.L6_DATATYPE_LONG_ARRAY:
                        long [] larr = new long[field.count];
                        field.data = larr;
                        for(j = 0; j < field.count; j++) {
                            larr[j] = diStream.readLong();
                        }
                        break;

                    case Layer6.L6_DATATYPE_FLOAT:
                        field.nData = (double)diStream.readFloat();
                        break;

                    case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                        float [] farr = new float[field.count];
                        field.data = farr;
                        for(j = 0; j < field.count; j++) {
                            farr[j] = diStream.readFloat();
                        }
                        break;

                    case Layer6.L6_DATATYPE_DOUBLE:
                        field.nData = diStream.readDouble();
                        break;

                    case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                        double [] darr = new double[field.count];
                        field.data = darr;
                        for(j = 0; j < field.count; j++) {
                            darr[j] = diStream.readDouble();
                        }
                        break;
                        
                    case Layer6.L6_DATATYPE_L6MSG:
                        Layer6Msg subMsg = new Layer6Msg();
                        int subMsgLen = field.count *  field.elemsize;
                        byte [] lm6arr = new byte[subMsgLen];
                        diStream.read(lm6arr);
                        subMsg.deserialize(lm6arr, 0, subMsgLen);
                        field.data = subMsg;
                        subMsgs.add(subMsg);
                        break;

                    default:
                       throw new Layer6Exception(Layer6.L6_ERR_UNHANDLED_FIELD_TYPE);
                }
                if(field.name != null) {
                    htblFName.put(field.name, field);
                }
                htblFId.put(new Integer(field.id), field);
            }
            //popStream.close();
        }
        catch(IOException ioEx) {
            ioEx.printStackTrace();
            throw new Layer6Exception(-1, ioEx);
        }
        return totalLength;
    }
    
/******************************* public fields *************************************/
    private void checkForRecursion(Layer6Msg msg) throws Layer6Exception {
        int len = subMsgs.size();
        for(int i = 0; i < len; i++) {
            Layer6Msg subMsg = (Layer6Msg)subMsgs.get(i);
            if(subMsg == this) {
                throw new Layer6Exception(Layer6.L6_ERR_RECURSIVE_SUB_MSG);
            }
            subMsg.checkForRecursion(this);
        }
    }

    public int serialize(byte [] buffer, int offset, int length) throws Layer6Exception {
        int ret = 0;
        checkForRecursion(this);
        if(useNIOBuffers) {
            ByteBuffer doBuffer = ByteBuffer.wrap(buffer, offset, length);
            ret = serializeUsingNIOBuffer(doBuffer);
        }
        else {
            ret = serializeUsingDataStreamPipes(buffer, offset, length);
        }
        return ret;
    }

    public int deserialize(byte [] buffer, int offset, int length) throws Layer6Exception {
        reset();
        int ret = 0;
        if(useNIOBuffers) {
            ByteBuffer diBuffer = ByteBuffer.wrap(buffer, offset, length);
            ret = deserializeUsingNIOBuffer(diBuffer);
        }
        else {
            ret = deserializeUsingDataStreamPipes(buffer, offset, length);
        }
        return ret;
    }
    public void reset() {    
        qfields.clear();
        subMsgs.clear();
        qpool.clear();
        htblFName.clear();
        htblFId.clear();

        subject         = null;
        hasSubject      = false;
        hasMetadata     = false;
        deepCopy        = true;
        code            = 0;
        totalLength     = 0;
        metadataLength  = L6_BASE_MSG_HDR_LENGTH;
        dataLength      = 0;
        errorCode       = 0;
        idgen           = 1024;
        templateID      = -1;
        
        debugInfo       = "";
    }

    public void setSubject(String sub) {
        subject = sub;
    }

    public String getSubject() {
        return subject;
    }

    public void setCode(byte c) {
        code = (byte)c;
    }

    public int getCode() {
        return code;
    }
    
    public String getDebugInfo() {
        return errorCode+" - "+getErrorMsg()+":"+ debugInfo;
    }

    public String getErrorMsg() {
        return Layer6.getErrorMsg(errorCode);
    }

    public void setDeepCopyMode(boolean b) {
        deepCopy = b;
    }

    public boolean getDeepCopyMode() {
        return deepCopy;
    }
    
    void validateIdOrThrowEx(int id) throws Layer6Exception {
       if((id < 0) || (id > Short.MAX_VALUE))
           throw new Layer6Exception(Layer6.L6_ERR_FIELD_ID_INVALID);
    }

    //Scalars
    public void addShort(short data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_SHORT, (double)data, 2);
    }
    
    public void setShort(int id, short data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_SHORT, (double)data, 2);
    }

    public void setShort(String key, short data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_SHORT, (double)data, 2);
    }

    public void setShortAt(int index, short data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_SHORT, (double)data, 2);
    }

    public void addInt(int data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_INT32, (double)data, 4);
    }

    public void setInt(int id, int data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32, (double)data, 4);
    }

    public void setInt(String key, int data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32, (double)data, 4);
    }

    public void setIntAt(int index, int data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_INT32, (double)data, 4);
    }

    public void addLong(long data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_LONG, (double)data, 8);
    }

    public void setLong(int id, long data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG, (double)data, 8);
    }

    public void setLong(String key, long data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG, (double)data, 8);
    }

    public void setLongAt(int index, long data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_LONG, (double)data, 8);
    }

    public void addFloat(float data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_FLOAT, (double)data, 4);
    }

    public void setFloat(int id, float data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT, (double)data, 4);
    }

    public void setFloat(String key, float data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT, (double)data, 4);
    }

    public void setFloatAt(int index, float data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_FLOAT, (double)data, 4);
    }

    public void addDouble(double data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_DOUBLE, data, 8);
    }

    public void setDouble(int id, double data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE, data, 8);
    }

    public void setDouble(String key, double data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE, data, 8);
    }

    public void setDoubleAt(int index, double data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_DOUBLE, data, 8);
    }

    //Arrays
    public void addString(String data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_STRING, data, 1, 0, (data.length() +1), deepCopy);
    }

    public void setString(int id, String data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_STRING, data, 1, 0, (data.length() +1), deepCopy);
    }

    public void setString(String key, String data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_STRING, data, 1, 0, (data.length() +1), deepCopy);
    }

    public void setStringAt(int index, String data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_STRING, data, 1, 0, (data.length() +1), deepCopy);
    }

    public void addByteArray(byte [] data) throws Layer6Exception {
        addByteArray(data, 0, data.length);
    }

    public void setByteArray(int id, byte [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setByteArray(id, data, 0, data.length);
    }

    public void setByteArray(String key, byte [] data) throws Layer6Exception {
        setByteArray(key, data, 0, data.length);
    }

    public void setByteArrayAt(int index, byte [] data) throws Layer6Exception {
        setByteArrayAt(index, data, 0, data.length);
    }

    public void addByteArray(byte [] data, int offset, int length) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_BYTES, data, 1, offset, length, deepCopy);
    }

    public void setByteArray(int id, byte [] data, int offset, int length) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_BYTES, data, 1, offset, length, deepCopy);
    }

    public void setByteArray(String key, byte [] data, int offset, int length) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_BYTES, data, 1, offset, length, deepCopy);
    }

    public void setByteArrayAt(int index, byte [] data, int offset, int length) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_BYTES, data, 1, offset, length, deepCopy);
    }

    public void addShortArray(short [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, 0, data.length, deepCopy);
    }

    public void setShortArray(int id, short [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, 0, data.length, deepCopy);
    }

    public void setShortArray(String key, short [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, 0, data.length, deepCopy);
    }

    public void setShortArrayAt(int index, short [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, 0, data.length, deepCopy);
    }

    public void addShortArray(short [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, offset, len, deepCopy);
    }
    
    public void setShortArray(int id, short [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, offset, len, deepCopy);
    }

    public void setShortArray(String key, short [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, offset, len, deepCopy);
    }

    public void setShortArrayAt(int index, short [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, offset, len, deepCopy);
    }

    public void addShortArrayRef(short [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, offset, len, false);
    }
    
    public void setShortArrayRef(int id, short [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, offset, len, false);
    }

    public void setShortArrayRef(String key, short [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, offset, len, false);
    }

    public void setShortArrayRefAt(int index, short [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, offset, len, false);
    }

    public void addIntArray(int [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, 0, data.length, deepCopy);
    }

    public void setIntArray(int id, int [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, 0, data.length, deepCopy);
    }

    public void setIntArray(String key, int [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, 0, data.length, deepCopy);
    }

    public void setIntArrayAt(int index, int [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, 0, data.length, deepCopy);
    }

    public void addIntArray(int [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, offset, len, deepCopy);
    }

    public void setIntArray(int id, int [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, offset, len, deepCopy);
    }

    public void setIntArray(String key, int [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, offset, len, deepCopy);
    }

    public void setIntArrayAt(int index, int [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, offset, len, deepCopy);
    }

    public void addIntArrayRef(int [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, offset, len, false);
    }
    
    public void setIntArrayRef(int id, int [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, offset, len, false);
    }

    public void setIntArrayRef(String key, int [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, offset, len, false);
    }

    public void setIntArrayRefAt(int index, int [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, offset, len, false);
    }

    public void addLongArray(long [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void setLongArray(int id, long [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void setLongArray(String key, long [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void setLongArrayAt(int index, long [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void addLongArray(long [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, offset, len, deepCopy);
    }
    
    public void setLongArray(int id, long [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, offset, len, deepCopy);
    }

    public void setLongArray(String key, long [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, offset, len, deepCopy);
    }

    public void setLongArrayAt(int index, long [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, offset, len, deepCopy);
    }

    public void addLongArrayRef(long [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, offset, len, false);
    }
    
    public void setLongArrayRef(int id, long [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, offset, len, false);
    }

    public void setLongArrayRef(String key, long [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, offset, len, false);
    }

    public void setLongArrayRefAt(int index, long [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, offset, len, false);
    }

    public void addFloatArray(float [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void setFloatArray(int id, float [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void setFloatArray(String key, float [] data, int count) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, 0, data.length, deepCopy);
    }

    public void setFloatArrayAt(int index, float [] data, int count) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, 0, data.length, deepCopy);
    }

    public void addFloatArray(float [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -2, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, offset, len, deepCopy);
    }
    
    public void setFloatArray(int id, float [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, offset, len, deepCopy);
    }

    public void setFloatArray(String key, float [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, offset, len, deepCopy);
    }

    public void setFloatArrayAt(int index, float [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, offset, len, deepCopy);
    }

    public void addFloatArrayRef(float [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, offset, len, false);
    }
    
    public void setFloatArrayRef(int id, float [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, offset, len, false);
    }

    public void setFloatArrayRef(String key, float [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, offset, len, false);
    }

    public void setFloatArrayRefAt(int index, float [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, offset, len, false);
    }

    public void addDoubleArray(double [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void setDoubleArray(int id, double [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void setDoubleArray(String key, double [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void setDoubleArrayAt(int index, double [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, 0, data.length, deepCopy);
    }

    public void addDoubleArray(double [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, len, deepCopy);
    }
    
    public void setDoubleArray(int id, double [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, len, deepCopy);
    }

    public void setDoubleArray(String key, Double [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, len, deepCopy);
    }

    public void setDoubleArrayAt(int index, Double [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, len, deepCopy);
    }

    public void addDoubleArrayRef(Double [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, len, false);
    }
    
    public void setDoubleArrayRef(int id, Double [] data, int offset, int len) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, len, false);
    }

    public void setDoubleArrayRef(String key, Double [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, len, false);
    }

    public void setDoubleArrayRefAt(int index, Double [] data, int offset, int len) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, len, false);
    }
    
    public void addLayer6Msg(Layer6Msg data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_L6MSG, data, 1, 0, data.size(), deepCopy);
    }

    public void addLayer6MsgRef(Layer6Msg data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_L6MSG, data, 1, 0, data.size(), false);
    }
    
    public void setLayer6Msg(String key, Layer6Msg data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_L6MSG, data, 1, 0, data.size(), deepCopy);
    }

    public void setLayer6MsgRef(String key, Layer6Msg data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_L6MSG, data, 1, 0, data.size(), false);
    }
        
    public void setLayer6Msg(int id, Layer6Msg data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_L6MSG, data, 1, 0, data.size(), deepCopy);
    }

    public void setLayer6MsgRef(int id, Layer6Msg data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_L6MSG, data, 1, 0, data.size(), false);
    }

    public void setLayer6MsgAt(int index, Layer6Msg data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_L6MSG, data, 1, 0, data.size(), deepCopy);
    }

    public void setLayer6MsgRefAt(int index, Layer6Msg data) throws Layer6Exception {
        setFieldInMsg(null, -1, index, Layer6.L6_DATATYPE_L6MSG, data, 1, 0, data.size(), false);
    }

    public Layer6Msg getLayer6Msg(String key) throws Layer6Exception {
        return (Layer6Msg)getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_L6MSG, null, 0, 1, deepCopy);
    }
    
    public Layer6Msg getLayer6Msg(int id) throws Layer6Exception {
        return (Layer6Msg)getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_L6MSG, null, 0, 1, deepCopy);
    }

    public Layer6Msg getLayer6MsgAt(int index) throws Layer6Exception {
        return (Layer6Msg)getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_L6MSG, null, 0, 1, false);
    }

    public Layer6Msg getLayer6MsgRef(String key) throws Layer6Exception {
        return (Layer6Msg)getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_L6MSG, null, 0, 1, false);
    }
    
    public Layer6Msg getLayer6MsgRef(int id) throws Layer6Exception {
        return (Layer6Msg)getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_L6MSG, null, 0, 1, false);
    }

    public Layer6Msg getLayer6MsgRefAt(int index) throws Layer6Exception {
        return (Layer6Msg)getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_L6MSG, null, 0, 1, false);
    }


    //Set Ref arrays
    public void addStringRef(String data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_STRING, data, 1, 0, (data.length() +1), false);
    }

    public void setStringRef(int id, String data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_STRING, data, 1, 0, (data.length() +1), false);
    }

    public void setStringRef(String key, String data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_STRING, data, 1, 0, (data.length() +1), false);
    }

    public void addByteArrayRef(byte [] data) throws Layer6Exception {
        addByteArrayRef(data, 0, data.length);
    }

    public void setByteArrayRef(int id, byte [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setByteArrayRef(id, data, 0, data.length);
    }

    public void setByteArrayRef(String key, byte [] data) throws Layer6Exception {
        setByteArrayRef(key, data, 0, data.length);
    }

    public void addByteArrayRef(byte [] data, int offset, int length) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_BYTES, data, 1, offset, length, false);
    }

    public void setByteArrayRef(int id, byte [] data, int offset, int length) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_BYTES, data, 1, offset, length, false);
    }

    public void setByteArrayRef(String key, byte [] data, int offset, int length) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_BYTES, data, 1, offset, length, false);
    }

    public void addShortArrayRef(short [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, 0, data.length, false);
    }

    public void setShortArrayRef(int id, short [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, 0, data.length, false);
    }

    public void setShortArrayRef(String key, short [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 2, 0, data.length, false);
    }

    public void addIntArrayRef(int [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, 0, data.length, false);
    }

    public void setIntArrayRef(int id, int [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, 0, data.length, false);
    }

    public void setIntArrayRef(String key, int [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 4, 0, data.length, false);
    }

    public void addLongArrayRef(long [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, 0, data.length, false);
    }

    public void setLongArrayRef(int id, long [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, 0, data.length, false);
    }

    public void setLongArrayRef(String key, long [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 8, 0, data.length, false);
    }
    
    public void addFloatArrayRef(float [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, 0, data.length, false);
    }

    public void setFloatArrayRef(int id, float [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, 0, data.length, false);
    }

    public void setFloatArrayRef(String key, float [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 4, 0, data.length, false);
    }

    public void addDoubleArrayRef(double [] data) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, 0, data.length, false);
    }

    public void setDoubleArrayRef(int id, double [] data) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, 0, data.length, false);
    }

    public void setDoubleArrayRef(String key, double [] data) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, 0, data.length, false);
    }

    public void addDoubleArrayRef(double [] data, int offset, int length) throws Layer6Exception {
        setFieldInMsg(null, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, length, false);
    }

    public void setDoubleArrayRef(int id, double [] data, int offset, int length) throws Layer6Exception {
        validateIdOrThrowEx(id);
        setFieldInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, length, false);
    }

    public void setDoubleArrayRef(String key, double [] data, int offset, int length) throws Layer6Exception {
        setFieldInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 8, offset, length, false);
    }

    //Get Scalars
    public short getShortAt(int index) throws Layer6Exception {
        return (short)getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_SHORT);
    }

    public short getShort(int id) throws Layer6Exception {
        return (short)getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_SHORT);
    }

    public short getShort(String key) throws Layer6Exception {
        return (short)getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_SHORT);
    }

    public int getIntAt(int index) throws Layer6Exception {
        return (int)getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_INT32);
    }

    public int getInt(int id) throws Layer6Exception {
        return (int)getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32);
    }

    public int getInt(String key) throws Layer6Exception {
        return (int)getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32);
    }

    public long getLongAt(int index) throws Layer6Exception {
        return (long)getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_LONG);
    }

    public long getLong(int id) throws Layer6Exception {
        return (long)getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG);
    }

    public long getLong(String key) throws Layer6Exception {
        return (long)getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG);
    }

    public float getFloatAt(int index) throws Layer6Exception {
        return (float)getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_FLOAT);
    }

    public float getFloat(int id) throws Layer6Exception {
        return (float)getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT);
    }

    public float getFloat(String key) throws Layer6Exception {
        return (float)getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT);
    }

    public double getDoubleAt(int index) throws Layer6Exception {
        return getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_DOUBLE);
    }
    public double getDouble(int id) throws Layer6Exception {
        return getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE);
    }

    public double getDouble(String key) throws Layer6Exception {
        return getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE);
    }

    // Get Arrays
    public String getStringAt(int index) throws Layer6Exception {
        return (String)getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_STRING, null, 0, -1, deepCopy);
    }

    public String getString(int id) throws Layer6Exception {
        return (String)getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_STRING, null, 0, -1, deepCopy);
    }

    public String getString(String key) throws Layer6Exception {
        return (String)getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_STRING, null, 0, -1, deepCopy);
    }

    public byte [] getByteArrayAt(int index) throws Layer6Exception {
        return getByteArrayAt(index, 0, -1);
    }
    public byte [] getByteArray(int id) throws Layer6Exception {
        return getByteArray(id, 0, -1);
    }

    public byte [] getByteArray(String key) throws Layer6Exception {
        return getByteArray(key, 0, -1);
    }

    public byte [] getByteArrayAt(int index, int offset, int size) throws Layer6Exception {
        return (byte[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_BYTES, null, offset, size, deepCopy);
    }

    public byte [] getByteArray(int id, int offset, int size) throws Layer6Exception {
        return (byte[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_BYTES, null, offset, size, deepCopy);
    }

    public byte [] getByteArray(String key, int offset, int size) throws Layer6Exception {
        return (byte[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_BYTES, null, offset, size, deepCopy);
    }

    public short [] getShortArrayAt(int index) throws Layer6Exception {
        return (short[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_SHORT_ARRAY, null, 0, -1, deepCopy);
    }

    public short [] getShortArray(int id) throws Layer6Exception {
        return (short[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, null, 0, -1, deepCopy);
    }

    public short [] getShortArray(String key) throws Layer6Exception {
        return (short[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, null, 0, -1, deepCopy);
    }

    public int [] getIntArrayAt(int index) throws Layer6Exception {
        return (int[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_INT32_ARRAY, null, 0, -1, deepCopy);
    }

    public int [] getIntArray(int id) throws Layer6Exception {
        return (int[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32_ARRAY, null, 0, -1, deepCopy);
    }

    public int [] getIntArray(String key) throws Layer6Exception {
        return (int[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, null, 0, -1, deepCopy);
    }

    public long [] getLongArrayAt(int index) throws Layer6Exception {
        return (long[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_LONG_ARRAY, null, 0, -1, deepCopy);
    }

    public long [] getLongArray(int id) throws Layer6Exception {
        return (long[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG_ARRAY, null, 0, -1, deepCopy);
    }

    public long [] getLongArray(String key) throws Layer6Exception {
        return (long[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, null, 0, -1, deepCopy);
    }

    public float [] getFloatArrayAt(int index) throws Layer6Exception {
        return (float[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_FLOAT_ARRAY, null, 0, -1, deepCopy);
    }

    public float [] getFloatArray(int id) throws Layer6Exception {
        return (float[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, null, 0, -1, deepCopy);
    }

    public float [] getFloatArray(String key) throws Layer6Exception {
        return (float[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, null, 0, -1, deepCopy);
    }

    public double [] getDoubleArrayAt(int index) throws Layer6Exception {
        return (double[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_DOUBLE_ARRAY, null, 0, -1, deepCopy);
    }

    public double [] getDoubleArray(int id) throws Layer6Exception {
        return (double[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, null, 0, -1, deepCopy);
    }

    public double [] getDoubleArray(String key) throws Layer6Exception {
        return (double[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, null, 0, -1, deepCopy);
    }
    
    //copy-to-array-in-argument methods
    public void getByteArrayAt(int index, byte [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_BYTES, data, offset, size, deepCopy);
    }

    public void getByteArray(int id, byte [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_BYTES, data, offset, size, deepCopy);
    }

    public void getByteArray(String key, byte [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_BYTES, null, offset, size, deepCopy);
    }

    public void getShortArrayAt(int index, short [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 0, -1, deepCopy);
    }

    public void getShortArray(int id, short [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 0, -1, deepCopy);
    }

    public void getShortArray(String key, short [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_SHORT_ARRAY, data, 0, -1, deepCopy);
    }

    public void getIntArrayAt(int index, int [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_INT32_ARRAY, data, 0, -1, deepCopy);
    }

    public void getIntArray(int id, int [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 0, -1, deepCopy);
    }

    public void getIntArray(String key, int [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, data, 0, -1, deepCopy);
    }

    public void getLongArrayAt(int index, long [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_LONG_ARRAY, data, 0, -1, deepCopy);
    }

    public void getLongArray(int id, long [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 0, -1, deepCopy);
    }

    public void getLongArray(String key, long [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, data, 0, -1, deepCopy);
    }

    public void getFloatArrayAt(int index, float [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 0, -1, deepCopy);
    }

    public void getFloatArray(int id, float [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 0, -1, deepCopy);
    }

    public void getFloatArray(String key, float [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, data, 0, -1, deepCopy);
    }

    public void getDoubleArrayAt(int index, double [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 0, -1, deepCopy);
    }

    public void getDoubleArray(int id, double [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 0, -1, deepCopy);
    }

    public void getDoubleArray(String key, double [] data, int offset, int size) throws Layer6Exception {
        getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, data, 0, -1, deepCopy);
    }

    //shallow
    public String getStringRefAt(int index) throws Layer6Exception {
        return (String)getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_STRING, null, 0, -1, deepCopy);
    }

    public String getStringRef(int id) throws Layer6Exception {
        return (String)getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_STRING, null, 0, -1, deepCopy);
    }

    public String getStringRef(String key) throws Layer6Exception {
        return (String)getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_STRING, null, 0, -1, deepCopy);
    }

    public byte [] getByteArrayRefAt(int index) throws Layer6Exception {
        return getByteArrayAt(index, 0, -1);
    }
    public byte [] getByteArrayRef(int id) throws Layer6Exception {
        return getByteArray(id, 0, -1);
    }

    public byte [] getByteArrayRef(String key) throws Layer6Exception {
        return getByteArray(key, 0, -1);
    }

    public int [] getIntArrayRefAt(int index) throws Layer6Exception {
        return (int[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_INT32_ARRAY, null, 0, -1, false);
    }
    public int [] getIntArrayRef(int id) throws Layer6Exception {
        return (int[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_INT32_ARRAY, null, 0, -1, false);
    }

    public int [] getIntArrayRef(String key) throws Layer6Exception {
        return (int[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_INT32_ARRAY, null, 0, -1, false);
    }

    public long [] getLongArrayRefAt(int index) throws Layer6Exception {
        return (long[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_LONG_ARRAY, null, 0, -1, false);
    }

    public long [] getLongArrayRef(int id) throws Layer6Exception {
        return (long[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_LONG_ARRAY, null, 0, -1, false);
    }

    public long [] getLongArrayRef(String key) throws Layer6Exception {
        return (long[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_LONG_ARRAY, null, 0, -1, false);
    }

    public float [] getFloatArrayRefAt(int index) throws Layer6Exception {
        return (float[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_FLOAT_ARRAY, null, 0, -1, false);
    }

    public float [] getFloatArrayRef(int id) throws Layer6Exception {
        return (float[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, null, 0, -1, false);
    }

    public float [] getFloatArrayRef(String key) throws Layer6Exception {
        return (float[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_FLOAT_ARRAY, null, 0, -1, false);
    }

    public double [] getDoubleArrayRefAt(int index) throws Layer6Exception {
        return (double[])getFieldDataInMsg(null, -1, index, Layer6.L6_DATATYPE_DOUBLE_ARRAY, null, 0, -1, false);
    }

    public double [] getDoubleArrayRef(int id) throws Layer6Exception {
        return (double[])getFieldDataInMsg(null, id, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, null, 0, -1, false);
    }

    public double [] getDoubleArrayRef(String key) throws Layer6Exception {
        return (double[])getFieldDataInMsg(key, -1, -1, Layer6.L6_DATATYPE_DOUBLE_ARRAY, null, 0, -1, false);
    }

    //byte ops
    public int size() {
        return getSizeInBytes();
    }
    
    public int getSizeInBytes() {
        //recalculate
        totalLength       = L6_BASE_MSG_HDR_LENGTH;
        metadataLength    = L6_BASE_MSG_HDR_LENGTH;
        dataLength        = 0;

        for(int i = 0; i < qfields.size(); i++) {
            Layer6MsgField field = (Layer6MsgField)qfields.get(i);
            if(field.type == Layer6.L6_DATATYPE_L6MSG) {
                //check if it has been changed recently
                Layer6Msg subMsg = (Layer6Msg)field.data;
                int subMsgSize = subMsg.size();
                if(field.size != subMsgSize) {
                    //update new field count and size
                    field.datalen = subMsgSize;
                    field.count = (short)field.datalen;
                    field.size = field.mdlength + field.datalen;
                }
            }
            //updateAggregateLengthByDelta(field.size, field.mdlength, field.datalen);
            totalLength     += field.size;
            metadataLength  += field.mdlength;
            dataLength      += field.datalen;
        }
        return totalLength;
    }

    //field ops
    public int getNumFields() {
        return qfields.size();
    }
    
    private void removeFieldFromMsg(Layer6MsgField field) {
        if(field.name != null) {
            htblFName.remove(field.name);
        }
        if(field.id >= 0) {
            htblFId.remove(new Integer(field.id));
        }
        if(field.type == Layer6.L6_DATATYPE_L6MSG) {
            //remove from submsgs also
            subMsgs.remove(field.data);
        }
        qfields.remove(field);
        //undo field metadata for new data
        //updateAggregateLengthByDelta(-field.size, -field.mdlength, -field.datalen);
    }

    public void removeFieldAt(int index) throws Layer6Exception {
        if((index < 0) || (index >= qfields.size())) {
           throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        Layer6MsgField field = (Layer6MsgField)qfields.get(index);
        removeFieldFromMsg(field);
    }

    public void removeField(int id) throws Layer6Exception {
        Integer Id = new Integer(id);
        Layer6MsgField field = (Layer6MsgField)htblFId.remove(Id);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        field.id = -1;
        removeFieldFromMsg(field);
    }

    public void removeField(String fieldname) throws Layer6Exception {
        Layer6MsgField field = (Layer6MsgField)htblFName.remove(fieldname);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        field.name = null;
        removeFieldFromMsg(field);
    }

    public int getFieldSizeInBytes(int id) throws Layer6Exception {
        Integer Id = new Integer(id);
        Layer6MsgField field = (Layer6MsgField)htblFId.get(Id);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.size;
    }

    public int getFieldSizeInBytes(String fieldname) throws Layer6Exception {
        Layer6MsgField field = (Layer6MsgField)htblFName.get(fieldname);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.size;
    }

    public int getFieldCount(int id) throws Layer6Exception {
        Integer Id = new Integer(id);
        Layer6MsgField field = (Layer6MsgField)htblFId.get(Id);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.count;
    }

    public int getFieldCount(String fieldname) throws Layer6Exception {
        Layer6MsgField field = (Layer6MsgField)htblFName.get(fieldname);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.count;
    }

    public int getFieldUnitSize(int id) throws Layer6Exception {
        Integer Id = new Integer(id);
        Layer6MsgField field = (Layer6MsgField)htblFId.get(Id);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.elemsize;
    }

    public int getFieldUnitSize(String fieldname) throws Layer6Exception {
        Layer6MsgField field = (Layer6MsgField)htblFName.get(fieldname);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.elemsize;
    }

    private Layer6MsgField getField(int id) throws Layer6Exception {
        Integer Id = new Integer(id);
        Layer6MsgField field = (Layer6MsgField)htblFId.get(Id);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field;
    }

    private Layer6MsgField getField(String fieldname) throws Layer6Exception {
        Layer6MsgField field = (Layer6MsgField)htblFName.get(fieldname);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field;
    }

    public String getFieldNameAt(int index) throws Layer6Exception {
        if(index >= qfields.size()) {
            throw new Layer6Exception(Layer6.L6_WARN_INDEX_OUT_OF_BOUNDS);
        }
        Layer6MsgField field = (Layer6MsgField)qfields.get(index);
        if(field.name == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_UNNAMED);
        }
        return field.name;
    }

    public int getFieldIdAt(int index) throws Layer6Exception {
        if(index >= qfields.size()) {
            throw new Layer6Exception(Layer6.L6_WARN_INDEX_OUT_OF_BOUNDS);
        }
        Layer6MsgField field = (Layer6MsgField)qfields.get(index);
        return field.id;
    }

    public boolean isFieldNamed(int id) throws Layer6Exception {
        Integer Id = new Integer(id);
        Layer6MsgField field = (Layer6MsgField)htblFId.get(Id);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return (field.name !=null);
    }

    public boolean isFieldNamedAt(int index) throws Layer6Exception {
        if(index >= qfields.size()) {
            throw new Layer6Exception(Layer6.L6_WARN_INDEX_OUT_OF_BOUNDS);
        }
        Layer6MsgField field = (Layer6MsgField)qfields.get(index);
        return (field.name !=null);
    }

    public int getFieldTypeAt(int index) throws Layer6Exception {
        Layer6MsgField field = null;
        if((index >= 0) && (index < qfields.size())) {
            field = (Layer6MsgField)qfields.get(index);
        }
        else {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.type;
    }

    public int getFieldType(int id) throws Layer6Exception {
        Integer Id = new Integer(id);
        Layer6MsgField field = (Layer6MsgField)htblFId.get(Id);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.type;
    }

    public int getFieldType(String fieldname) throws Layer6Exception {
        Layer6MsgField field = (Layer6MsgField)htblFName.get(fieldname);
        if(field == null) {
            throw new Layer6Exception(Layer6.L6_ERR_FIELD_NOT_FOUND);
        }
        return field.type;
    }
    
    public String toString() {
        //return to String of fields
        return dump();
    }
    
    public String dump() {
        return dump("");
    }
    
    public Layer6Msg dup() throws Layer6Exception {
        Layer6Msg dupMsg = new Layer6Msg();
        dupInto(dupMsg);
        return dupMsg;
    }

    public void dupInto(Layer6Msg dupMsg) throws Layer6Exception {
        int len = qfields.size();
        for(int i = 0; i < len; i++) {
            Layer6MsgField field = (Layer6MsgField)qfields.get(i);
            if((field.type &  Layer6Msg.L6_FLAG_FIELD_IS_ARRAY) > 0) {
                dupMsg.setFieldInMsg(field.name, field.id, -1, field.type, field.data, field.elemsize, 0, field.count, dupMsg.deepCopy);
            }
            else {
                dupMsg.setFieldInMsg(field.name, field.id, -1, field.type, field.nData, field.elemsize);            
            }           }
    }
    
    public String dump(String indent) {
        StringBuffer sbuf = new StringBuffer(size());
        sbuf.append("l6msg:");
        int len = qfields.size();
        for(int i = 0; i < len; i++) {
            Layer6MsgField field = (Layer6MsgField)qfields.get(i);
            sbuf.append(field.toString(indent));
        }
        return sbuf.toString();
    }
    
    public void setTemplated(short tid) throws Layer6Exception {
        debugInfo = "Template flag set in " + code;
        //code |= L6_FLAG_MSG_IS_TEMPLATE;
        //templateID = tid;
        throw new Layer6Exception(Layer6.L6_ERR_NOT_IMPLEMENTED);
    }
    
    public boolean isTemplated() throws Layer6Exception {
        return ((code & L6_FLAG_MSG_IS_TEMPLATE) > 0);
    }
}
