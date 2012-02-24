package org.layer6;
class Layer6MsgField {
//metadata
    public short   id;
    public String  name;
    public int     type;
    public int     elemsize;
    public int     count;
    public short   mdlength;
    public int     size;
    public int     datalen;

    //data
    public Object  data;
    public double  nData;

    public int     namelen;
    public int     index;
    public int     offset;
    public boolean extension;

    public Layer6MsgField() {
        name      = null;
        namelen   = 0;
        elemsize  = 0;
        mdlength  = 0;
        id        = -1;
        extension = false;
    }

    public String toString() {
        return toString("");
    }
    
    public String toString(String indent) {
        int sz = (size < 0 ? 1024 : size);
        StringBuffer sbuf = new StringBuffer(sz);
        sbuf.append(indent);
        //sbuf.append((id >= 0   ? ("fid=[" + id + "]")   :  ""));
        sbuf.append("fid=[");
        sbuf.append(id);
        sbuf.append("]"); 
        sbuf.append((name!=null? (" name=\""+name+"\"") : ""));
        sbuf.append(" type=["); 
        sbuf.append(Layer6.getTypeString(type)); 
        sbuf.append("] size="); 
        sbuf.append("[");
        sbuf.append(elemsize); 
        sbuf.append("*"); 
        sbuf.append(count);
        sbuf.append("=");
        sbuf.append(datalen);
        sbuf.append(" + "); 
        sbuf.append(mdlength);
        sbuf.append("]=");
        sbuf.append(size);
        sbuf.append(" data=");
        sbuf.append("[");
        if((type > Layer6.L6_PRIMITIVE_DATATYPE_MAX) && (data ==null)) {
            sbuf.append("null");
        }
        else if((type & Layer6.L6_FLAG_FIELD_IS_EXTENSION) > 0) {
            sbuf.append(data.toString());
        }
        else
        switch(type & Layer6.L6_FLAG_FIELD_TYPE_MASK) {
        case Layer6.L6_DATATYPE_SHORT:
            sbuf.append((short)nData);
            break;
        case Layer6.L6_DATATYPE_INT32:
            sbuf.append((int)nData);
            break;
        //case Layer6.L6_DATATYPE_INT64:
        case Layer6.L6_DATATYPE_LONG:
            sbuf.append((long)nData);
            break;
        case Layer6.L6_DATATYPE_FLOAT:
            sbuf.append((float)nData);
            break;
        case Layer6.L6_DATATYPE_DOUBLE:
            sbuf.append((double)nData);
            break;
        case Layer6.L6_DATATYPE_BYTES:
            sbuf.append("bytes");
            break;
        case Layer6.L6_DATATYPE_SHORT_ARRAY:
            sbuf.append(java.util.Arrays.toString((short [])data));
            break;
        case Layer6.L6_DATATYPE_INT32_ARRAY:
            sbuf.append(java.util.Arrays.toString((int [])data));
            break;
        //case Layer6.L6_DATATYPE_INT64_ARRAY:
        case Layer6.L6_DATATYPE_LONG_ARRAY:
            sbuf.append(java.util.Arrays.toString((long [])data));
            break;
        case Layer6.L6_DATATYPE_FLOAT_ARRAY:
            sbuf.append(java.util.Arrays.toString((float [])data));
            break;
        case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
            sbuf.append(java.util.Arrays.toString((double [])data));
            break;
        case Layer6.L6_DATATYPE_STRING:
            sbuf.append(data.toString());
            break;
        case Layer6.L6_DATATYPE_L6MSG:
            sbuf.append("l6msg");  //((Layer6Msg)data).dump(indent+"\t"));
            break;
        }
        sbuf.append("]");
        return sbuf.toString();
    }
}