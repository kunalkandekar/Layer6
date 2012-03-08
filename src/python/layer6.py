import struct
import array  

def debug(sdbg):
    #print sdbg
    return

#define debug(X) //printf(X)fflush(stdout)
def dbgprint(buf, l):
    print "\n dbug:"
    if(len < 0):
        return
    for i in range(l):
       print "%d,"% buf[i]
    return

def debug_field(field):
    namestr = ""
    if field.name != None and len(field.name) > 0: namestr = field.name
    print "\t id=%d name='%s': type=[%d] size=[%d*%d=%d + %d]=%d data=%s" % (field.fid, namestr,
        field.ftype, field.length, field.count, field.datalen, field.mdlength, field.size, str(field.data))

class Layer6MsgField:
    def __init__(self):
    	# metadata 
    	self.fid      = -1
    	self.name     = None
    	self.ftype    = -1
    	self.data     = None
    	self.length   = 0
    	self.count    = 0
    	self.mdlength = 0
    	self.index    = 0
    	self.size     = 0
    	self.datalen  = 0
    	self.serialized = 0
    	self.internally_allocated = 0
    	# data 

    def reset(self):
    	self.fid      = -1
    	self.ftype    = -1
    	self.data     = None
    	self.mdlength = 0
    	self.index    = 0
    	self.size     = 0
        self.name     = None
        self.count    = 0
        self.length   = 0
        self.datalen  = 0
        self.serialized = 0
        self.internally_allocated = 0

    #end class Layer6MsgField

class Layer6:
    # static members
    L6_STATUS_OK                     =  0
    L6_STATUS_ERROR                  = -1
    L6_STATUS_WARNING                =  1
    
    L6_DATATYPE_UNSIGNED_BYTE        = 1
    L6_DATATYPE_UINT8                = 1
    L6_DATATYPE_BYTE                 = 2
    L6_DATATYPE_INT8                 = 2
    L6_DATATYPE_UINT16               = 3
    L6_DATATYPE_UNSIGNED_SHORT       = 3
    L6_DATATYPE_SHORT                = 4
    L6_DATATYPE_INT16                = 4
    L6_DATATYPE_UINT32               = 5
    L6_DATATYPE_UNSIGNED_INT         = 5
    L6_DATATYPE_INT                  = 6
    L6_DATATYPE_INT32                = 6
    L6_DATATYPE_UINT64               = 7
    L6_DATATYPE_UNSIGNED_LONG        = 7
    L6_DATATYPE_INT64                = 8
    L6_DATATYPE_LONG                 = 8
    L6_DATATYPE_FLOAT                = 9
    L6_DATATYPE_DOUBLE               = 10

    L6_DATATYPE_STRING              = 16
    L6_DATATYPE_UINT8_ARRAY         = 17
    L6_DATATYPE_UNSIGNED_BYTES      = 17
    L6_DATATYPE_BYTES               = 18
    L6_DATATYPE_INT8_ARRAY          = 18
    L6_DATATYPE_UINT16_ARRAY        = 19
    L6_DATATYPE_UNSIGNED_SHORT_ARRAY = 19
    L6_DATATYPE_INT16_ARRAY         = 20
    L6_DATATYPE_SHORT_ARRAY         = 20
    L6_DATATYPE_UINT32_ARRAY        = 21
    L6_DATATYPE_UNSIGNED_INT_ARRAY  = 21
    L6_DATATYPE_INT32_ARRAY         = 22
    L6_DATATYPE_INT_ARRAY           = 22
    L6_DATATYPE_UINT64_ARRAY        = 23
    L6_DATATYPE_UNSIGNED_LONG_ARRAY = 23
    L6_DATATYPE_INT64_ARRAY         = 24
    L6_DATATYPE_LONG_ARRAY          = 24
    L6_DATATYPE_FLOAT_ARRAY         = 25
    L6_DATATYPE_DOUBLE_ARRAY        = 26

    L6_DATATYPE_L6MSG               = 27
    L6_DATATYPE_UTF8_STRING         = 28

    L6_DATATYPE_EXTENSION           = 32
    
    L6_ERR_MEM_ALLOC                = 32
    L6_ERR_INSUFF_BUFFER_SIZE       = 33
    L6_ERR_INCORRECT_MSG_SIZE       = 34
    L6_ERR_FIELD_EXISTS             = 35
    L6_ERR_FIELD_NOT_FOUND          = 36
    L6_ERR_FIELD_NAME_TOO_LONG      = 37
    L6_ERR_FIELD_UNNAMED            = 38
    L6_ERR_FIELD_NO_ID              = 39
    L6_ERR_FIELD_ID_INVALID         = 40
    L6_ERR_FIELD_TYPE_INCOMPATIBLE  = 41
    L6_ERR_FIELD_MAX_COUNT_EXCEEDED = 42
    L6_ERR_FIELD_MAX_SIZE_EXCEEDED  = 43
    L6_ERR_MAX_NUM_FIELDS_EXCEEDED  = 44
    L6_ERR_MAX_MSG_SIZE_EXCEEDED    = 45
    L6_ERR_MSG_LOCKED               = 46
    L6_ERR_RECURSIVE_SUB_MSG        = 47
    L6_ERR_SUB_MSG_DESERIALIZE      = 48
    L6_ERR_OFFSET_OUT_OF_BOUNDS     = 49
    L6_ERR_UNHANDLED_FIELD_TYPE     = 50
    L6_ERR_NOT_IMPLEMENTED          = 51

    L6_WARN_INDEX_OUT_OF_BOUNDS     = 53
    L6_WARN_FIELD_REPLACED          = 54
    
    L6_ERR_CODES_MIN                = 32
    L6_ERR_CODES_MAX                = 52
    L6_ERR_CODES_OFFSET             = 28
    
    L6_BASE_MSG_HDR_LENGTH          = 8
    L6_BASE_FLD_HDR_LENGTH          = 1
    L6_FIELD_LENGTH_SCALAR          = 1
    L6_FIELD_LENGTH_ARRAY           = 3
    L6_FIELD_LENGTH_STRING          = 3

    L6_FLAG_FIELD_IS_ARRAY          = 16
    L6_FLAG_FIELD_HAS_NAME          = 64
    L6_FLAG_FIELD_HAS_ID            = 128    
    L6_FLAG_FIELD_TYPE_MASK         = 0x1F
    L6_PRIMITIVE_DATATYPE_MAX       = 10

    L6_MSG_FIELD_NAME_MAX_LEN       = 128
    
    DISALLOW_TRANSPARENT_OVERWRITE = False
        
    sizeof_int08_t  = 1
    sizeof_int16_t  = 2
    sizeof_int32_t  = 4
    sizeof_int64_t  = 8
    
    L6ErrorMsgs = [
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
        "Warning: L6_WARN_FIELD_REPLACED - Field replaced."]

    @staticmethod
    def get_error_str_for_code(error_code):
        if(error_code == 0):
            return Layer6.L6ErrorMsgs[0]
        elif((error_code > Layer6.L6_ERR_CODES_MIN) and (error_code < Layer6.L6_ERR_CODES_MAX)):
            return Layer6.L6ErrorMsgs[error_code - Layer6.L6_ERR_CODES_OFFSET]
        return Layer6.L6ErrorMsgs[1]

class Layer6Error(Exception):
    def __init__(self, code):
        self.error_code = code
        self.error_str  = Layer6.get_error_str_for_code(self.error_code)

    def __str__(self):
    	return repr(self.error_str)


class Layer6Msg:
    __METHODS_GENERATED__ = 0
    
    _SET_DEBUG_INFO = 1

    L6_MAX_UNUSED_SERIALIZATIONS    = 16

    L6_FLAG_MSG_IS_TEMPLATE         = 32

    l6_primitives = [Layer6.L6_DATATYPE_BYTE, Layer6.L6_DATATYPE_SHORT, Layer6.L6_DATATYPE_INT16, Layer6.L6_DATATYPE_INT32, Layer6.L6_DATATYPE_INT64, Layer6.L6_DATATYPE_LONG, Layer6.L6_DATATYPE_FLOAT, Layer6.L6_DATATYPE_DOUBLE] #Layer6.L6_DATATYPE_UINT8, Layer6.L6_DATATYPE_UINT16, Layer6.L6_DATATYPE_UINT32, Layer6.L6_DATATYPE_INT64]
    
    l6_array_types = [Layer6.L6_DATATYPE_STRING, Layer6.L6_DATATYPE_BYTES, Layer6.L6_DATATYPE_SHORT_ARRAY, Layer6.L6_DATATYPE_INT32_ARRAY, Layer6.L6_DATATYPE_INT64_ARRAY, Layer6.L6_DATATYPE_LONG_ARRAY,Layer6.L6_DATATYPE_FLOAT_ARRAY, Layer6.L6_DATATYPE_DOUBLE_ARRAY]
     #Layer6.L6_DATATYPE_UINT8_ARRAY, Layer6.L6_DATATYPE_UINT16_ARRAY, Layer6.L6_DATATYPE_UINT32_ARRAY, Layer6.L6_DATATYPE_INT64_ARRAY]

    l6_compound = [Layer6.L6_DATATYPE_STRING, Layer6.L6_DATATYPE_BYTES, Layer6.L6_DATATYPE_SHORT_ARRAY, Layer6.L6_DATATYPE_INT32_ARRAY, Layer6.L6_DATATYPE_INT64_ARRAY, Layer6.L6_DATATYPE_LONG_ARRAY, Layer6.L6_DATATYPE_FLOAT_ARRAY, Layer6.L6_DATATYPE_DOUBLE_ARRAY, Layer6.L6_DATATYPE_L6MSG]
    #Layer6.L6_DATATYPE_UINT8_ARRAY, Layer6.L6_DATATYPE_UINT16_ARRAY, Layer6.L6_DATATYPE_UINT32_ARRAY, Layer6.L6_DATATYPE_INT64_ARRAY]

    l6type_to_str = {
        Layer6.L6_DATATYPE_UINT8        : 'unsigned char', 
        Layer6.L6_DATATYPE_INT8         : 'byte', 
        Layer6.L6_DATATYPE_UINT16       : 'unsigned short', 
        Layer6.L6_DATATYPE_SHORT        : 'short', 
        Layer6.L6_DATATYPE_UINT32       : 'unsigned int', 
        Layer6.L6_DATATYPE_INT32        : 'int', 
        Layer6.L6_DATATYPE_UINT64       : 'unsigned long', 
        Layer6.L6_DATATYPE_LONG         : 'long',
        Layer6.L6_DATATYPE_FLOAT        : 'float', 
        Layer6.L6_DATATYPE_DOUBLE       : 'double', 
        Layer6.L6_DATATYPE_STRING       : 'string',
        Layer6.L6_DATATYPE_UINT8_ARRAY  : 'unsigned char []',
        Layer6.L6_DATATYPE_BYTES        : 'char []', 
        Layer6.L6_DATATYPE_UINT16_ARRAY : 'unsigned short []',  
        Layer6.L6_DATATYPE_SHORT_ARRAY  : 'short []',
        Layer6.L6_DATATYPE_UINT32_ARRAY : 'unsigned int []',  
        Layer6.L6_DATATYPE_INT_ARRAY    : 'int []', 
        Layer6.L6_DATATYPE_UINT64_ARRAY : 'unsigned long []',  
        Layer6.L6_DATATYPE_LONG_ARRAY   : 'long []', 
        Layer6.L6_DATATYPE_FLOAT_ARRAY  : 'float []', 
        Layer6.L6_DATATYPE_DOUBLE_ARRAY : 'double []', 
        Layer6.L6_DATATYPE_L6MSG        : 'l6msg',
        Layer6.L6_DATATYPE_UTF8_STRING  : 'utf-8'
    }
    
    l6type_to_struct_fmt = { 
        Layer6.L6_DATATYPE_UINT8           : 'B',
        Layer6.L6_DATATYPE_BYTE            : 'b',
        Layer6.L6_DATATYPE_UINT16          : 'H',
        Layer6.L6_DATATYPE_SHORT           : 'h',
        Layer6.L6_DATATYPE_INT16           : 'h',
        Layer6.L6_DATATYPE_UINT32          : 'I',
        Layer6.L6_DATATYPE_INT32           : 'i',
        Layer6.L6_DATATYPE_UINT64          : 'Q',
        Layer6.L6_DATATYPE_INT64           : 'q',
        Layer6.L6_DATATYPE_LONG            : 'q',        Layer6.L6_DATATYPE_FLOAT           : 'f',
        Layer6.L6_DATATYPE_DOUBLE          : 'd',
        Layer6.L6_DATATYPE_STRING          : 'str',
        Layer6.L6_DATATYPE_UINT8_ARRAY     : 'B',
        Layer6.L6_DATATYPE_BYTES           : 'b',
        Layer6.L6_DATATYPE_UINT16_ARRAY    : 'H',
        Layer6.L6_DATATYPE_SHORT_ARRAY     : 'h',
        Layer6.L6_DATATYPE_UINT32_ARRAY    : 'I',
        Layer6.L6_DATATYPE_INT32_ARRAY     : 'i',
        Layer6.L6_DATATYPE_UINT64_ARRAY    : 'Q',
        Layer6.L6_DATATYPE_INT64_ARRAY     : 'q',
        Layer6.L6_DATATYPE_LONG_ARRAY      : 'q',
        Layer6.L6_DATATYPE_FLOAT_ARRAY     : 'f',
        Layer6.L6_DATATYPE_DOUBLE_ARRAY    : 'd',
        Layer6.L6_DATATYPE_L6MSG           : 's'
    }
    
    l6type_to_size = { 
        Layer6.L6_DATATYPE_UINT8           : 1, 
        Layer6.L6_DATATYPE_BYTE            : 1,
        Layer6.L6_DATATYPE_UINT16          : 2,
        Layer6.L6_DATATYPE_SHORT           : 2,
        Layer6.L6_DATATYPE_INT16           : 2,
        Layer6.L6_DATATYPE_UINT32          : 4,
        Layer6.L6_DATATYPE_INT32           : 4,
        Layer6.L6_DATATYPE_UINT64          : 8,
        Layer6.L6_DATATYPE_INT64           : 8,
        Layer6.L6_DATATYPE_LONG            : 8,        Layer6.L6_DATATYPE_FLOAT           : 4,
        Layer6.L6_DATATYPE_DOUBLE          : 8,
        Layer6.L6_DATATYPE_STRING          : 1,
        Layer6.L6_DATATYPE_UINT8_ARRAY     : 1,
        Layer6.L6_DATATYPE_BYTES           : 1,
        Layer6.L6_DATATYPE_UINT16_ARRAY    : 2,
        Layer6.L6_DATATYPE_SHORT_ARRAY     : 2,
        Layer6.L6_DATATYPE_UINT32_ARRAY    : 4,
        Layer6.L6_DATATYPE_INT32_ARRAY     : 4,
        Layer6.L6_DATATYPE_UINT64_ARRAY    : 8,
        Layer6.L6_DATATYPE_INT64_ARRAY     : 8,
        Layer6.L6_DATATYPE_LONG_ARRAY      : 8,
        Layer6.L6_DATATYPE_FLOAT_ARRAY     : 4,
        Layer6.L6_DATATYPE_DOUBLE_ARRAY    : 8,
        Layer6.L6_DATATYPE_L6MSG           : 1
    }

    #Internal limits
    L6_MAX_NUM_FIELDS   = 255
    L6_FIELD_MAX_COUNT  = 65535

    def __init__(self):
        #build methods
        if not Layer6Msg.__METHODS_GENERATED__:
            Layer6Msg.gen_methods()
        # flags 
        opt = 5
        self.subject         = None
        self.code            = 0
        self.total_length    = Layer6.L6_BASE_MSG_HDR_LENGTH
        self.metadata_length = Layer6.L6_BASE_MSG_HDR_LENGTH
        self.data_length     = 0
        self.poolsize        = opt
        self.error_code      = 0
        self.deep_copy_default = True
        self.set_auto_byte_order_mode(1)
        
        self.fidgen          = 1024
        
        self.is_locked      = 0
        self.is_partial     = 0
        self.offset_md       = 0
        self.offset_d        = 0
        self.left           = 0
        self.done           = 0
        self.tmpbuffer      = None
        self.tmpbuffermem   = 0
        
        self.debug          = ''
        self.htbl_fname     = {}
        self.htbl_fid       = {}
        self.qfields        = []
        self.qsub_msgs      = []    
        self.qpool          = []

    #conversions    
    def to_float32(d):
        return d

    def to_short16(i):
        s16 = (i + 2**15) % 2**16 - 2**15
        return s16

    def from_float32(d):
        return d

    def from_short16(i):
        #s16 = (i + 2**15) % 2**16 - 2**15
        return i

    def __set_field_data_in_msg(self, s_name, i_id, i_index, i_type, data, i_size, i_count, b_copy=False):
        field = None
        ret = 0
        ilen = 0
        self.error_code = 0
    
        if(self.is_locked):
            self.error_code = Layer6.L6_ERR_MSG_LOCKED
            if(Layer6Msg._SET_DEBUG_INFO): 
                self.debug = ("%d-%s") % (self.error_code, self.get_error_str())
            raise Layer6Error(self.error_code)

        extra = ""
        if(s_name):
            ilen = len(s_name)
            #cannot be longer than L6_MSG_FIELD_NAME_MAX_LEN
            if(ilen > Layer6.L6_MSG_FIELD_NAME_MAX_LEN):
                self.error_code = Layer6.L6_ERR_FIELD_NAME_TOO_LONG
                if(Layer6Msg._SET_DEBUG_INFO): 
                    self.debug = "%d-%s" % (self.error_code, self.get_error_str())
                raise Layer6Error(self.error_code)
    
            field = self.__get_field_named(s_name, False)
            extra = "(name=%s)" % s_name

        if(i_id >= 0):
            field = self.__get_field_w_id(i_id, False)
            extra = "(id=%d)" % i_id

        if(i_index >= 0):
            field = self.__get_field_at_index(i_index)            
            extra = "(index=%d)" % i_index

        if(Layer6.DISALLOW_TRANSPARENT_OVERWRITE and field):
            self.error_code = Layer6.L6_ERR_FIELD_EXISTS
            if(Layer6Msg._SET_DEBUG_INFO): 
                self.debug = "%d-%s %s" % (self.error_code, self.get_error_str(), extra)
            raise Layer6Error(self.error_code)

        if(i_count > Layer6Msg.L6_FIELD_MAX_COUNT):
            self.error_code = Layer6.L6_ERR_FIELD_MAX_COUNT_EXCEEDED
            raise Layer6Error(self.error_code)

        if(field):
            if(field.ftype != i_type):
                self.error_code = Layer6.L6_ERR_FIELD_TYPE_INCOMPATIBLE
                if(Layer6Msg._SET_DEBUG_INFO): 
                    self.debug = "%d-%s" % (self.error_code, self.get_error_str())
                raise Layer6Error(self.error_code)
                #return None #, -1, -1

            if(field.count != i_count):
                self.total_length    -= field.size
                self.metadata_length -= field.mdlength
                self.data_length     -= field.datalen

                field.count           =  i_count
                field.datalen         =  field.length * field.count                field.size            =  field.mdlength + field.datalen

                self.total_length    += field.size
                self.metadata_length += field.mdlength
                self.data_length     += field.datalen
        else:
            if(len(self.qfields) >= Layer6Msg.L6_MAX_NUM_FIELDS):
                self.error_code = Layer6.L6_ERR_MAX_NUM_FIELDS_EXCEEDED
                raise Layer6Error(self.error_code)
            field = Layer6MsgField()
            field.name     = None
            field.dataptr  = None
            field.datamem  = 0
            field.serialized = 0
            field.internally_allocated = 0
            field.mdlength = 1
            field.length   = 0
    
            if(s_name != None):
                field.name = s_name
                field.mdlength += (len(field.name) + 2)    #strlen + 1 null byte + 1 byte length)
                self.htbl_fname[field.name] = field
            else:
                field.name = None
            
            if(i_id >= 0):
                field.fid = i_id            
                self.htbl_fid[field.fid] = field
                field.mdlength += Layer6.sizeof_int16_t

            self.qfields.append(field)
        
            field.ftype = i_type

            if (field.ftype & Layer6.L6_FLAG_FIELD_IS_ARRAY):
                field.mdlength += Layer6.sizeof_int16_t
            
            if(i_count > 0):
                #if field.ftype == Layer6.L6_DATATYPE_STRING: 
                #    i_count = i_count + 1
                field.datalen = i_size * i_count
                if(field.ftype == Layer6.L6_DATATYPE_L6MSG):
                    #field.data = data
                    self.qsub_msgs.append(field)
            else:
                field.datalen = i_size
    
            field.length   = i_size
            field.count    = i_count
            field.size     = field.mdlength + field.datalen
        
        
            self.total_length    += field.size
            self.metadata_length += field.mdlength
            self.data_length     += field.datalen

        field.data = data
        if b_copy:
            if field.ftype == Layer6.L6_DATATYPE_L6MSG:
                field.data = data.dup() 
            elif (field.ftype & Layer6.L6_FLAG_FIELD_IS_ARRAY):
                field.data = data[:]
        #else:  # for some weird reason, this else clause never executes... Python bug?
        #    field.data = data

        return field
    
    def __get_field_data_in_msg(self, name, fid, index, ftype, length, offset, count, b_deepcopy=0):
        field = None
        data = None
        ret = 0
        self.error_code = 0
        if(name):
            field = self.__get_field_named(name)
        elif(fid >= 0):
            field = self.__get_field_w_id(fid)
        else:   #index
            field = self.__get_field_at_index(index)

        if(field == None):
            self.error_code = Layer6.L6_ERR_FIELD_NOT_FOUND
            if(Layer6Msg._SET_DEBUG_INFO): 
                self.debug = "%d-%s" % (self.error_code, self.get_error_str())
            raise Layer6Error(self.error_code)   #field not found
    
        if(ftype != field.ftype):
            self.error_code = Layer6.L6_ERR_FIELD_TYPE_INCOMPATIBLE
            if(Layer6Msg._SET_DEBUG_INFO): 
                self.debug = "%d-%s" % (self.error_code, self.get_error_str())
            raise Layer6Error(self.error_code)
    
        if(field.ftype == Layer6.L6_DATATYPE_L6MSG):
            if(field.serialized):
                #deserialize
                sub_msg = Layer6Msg()
                ret = sub_msg.deserialize(field.data, field.datalen)
                if(ret < 0):
                    self.error_code = Layer6.L6_ERR_SUB_MSG_DESERIALIZE
                    if(Layer6Msg._SET_DEBUG_INFO): 
                        self.debug = "%d-%s" % (self.error_code, self.get_error_str())
                    raise Layer6Error(self.error_code)
                field.serialized = 0
                field.data = sub_msg
            else:
                if b_deepcopy:
                    data = field.data.dup()
                else:
                    data = field.data
        elif (field.ftype & Layer6.L6_FLAG_FIELD_IS_ARRAY): #arrays
            if b_deepcopy:  # return a copy of the array
                if count < 0: count = field.count
                if(self.is_locked):
                    self.error_code = Layer6.L6_ERR_MSG_LOCKED
                    data = None
                    if(Layer6Msg._SET_DEBUG_INFO): 
                        self.debug = "%d-%s" % (self.error_code, self.get_error_str())
                    ret = -1
                    raise Layer6Error(self.error_code)
                if((offset + count) > field.count):
                    count = field.count - offset
                    self.error_code = Layer6.L6_WARN_INDEX_OUT_OF_BOUNDS
                    ret = 1
                if(field.ftype == Layer6.L6_DATATYPE_STRING) or (field.ftype == Layer6.L6_DATATYPE_BYTES):
                    #do something?
                    ret = 0
                data = field.data[offset : offset+count]
            else:   
                data = field.data   #return a reference to the array
                ret = 0
        else:   
            #primitives
            data = field.data
            ret = 0
        return data #, field.count, ret
    
    
    def __remove_field_in_msg(self, field):
        self.error_code = 0
        if(field):
            self.qfields.remove(field)
            self.total_length    -= field.size
            self.metadata_length -= field.mdlength
            self.data_length     -= field.datalen
            if(field.ftype == Layer6.L6_DATATYPE_L6MSG):
                #remove from qsub_msgs
                self.qsub_msgs.remove(field)
                #is this internally allocated?
                if(field.internally_allocated):
                    #then it must also be freed internally
                    sub_msg = field.data.msg
                    field.data.free()
                    field.internally_allocated = 0            
            retfield = None
            if(field.name):
                retfield = self.htbl_fname[field.name]
                del self.htbl_fname[field.name]
            if(field.fid >= 0):
                del self.htbl_fid[field.fid]
        else:
            self.error_code = Layer6.L6_ERR_FIELD_NOT_FOUND
            if(Layer6Msg._SET_DEBUG_INFO): 
                self.debug = "%d-%s" % (self.error_code, self.get_error_str())
            raise Layer6Error(self.error_code)
        return 0

    def get_debug_info(self):
        return self.getDebugInfo()
        
    def getDebugInfo(self):
        return self.debug

    def get_error_str(self):
        return Layer6.get_error_str_for_code(self.error_code)

    def get_error_code(self):
        return self.error_code

    def free(self):
        self.reset()    #empties all field lists and table entries        
        del self.htbl_fname
        del self.htbl_fid
        del self.qfields
        del self.qsub_msgs
        self.error_code     = 0

    #is this a good idea? would it be useful to return num fields instead?
    def __len__(self):
        return self.get_size_in_bytes()

    def size(self):
    	return self.get_size_in_bytes()

    def get_size_in_bytes(self):
        self.error_code      = 0
        self.total_length    = Layer6.L6_BASE_MSG_HDR_LENGTH
        self.metadata_length = Layer6.L6_BASE_MSG_HDR_LENGTH
        self.data_length     = 0
        for field in self.qfields: 
            if(field.ftype == Layer6.L6_DATATYPE_L6MSG):
                #check if it has been changed recently
                sub_msg = field.data
                sub_msg_size = sub_msg.get_size_in_bytes()
                if(field.datalen != sub_msg_size):
                    #update new field count and size
                    field.datalen = sub_msg_size
                    field.count   = field.datalen
                    field.size    = field.mdlength + field.datalen
            #updateAggregateLengthByDelta(field.size, field.mdlength, field.datalen)
            self.total_length    += field.size
            self.metadata_length += field.mdlength
            self.data_length     += field.datalen
        return self.total_length
        
    def pack(self, buf, offset, data, sizeof_data_type, dtype, count=1):
        if dtype == 'str':
            sdata = data.encode('ascii')    #utf-8
            dtype = 's'
            struct.pack_into('!%ds\0' % (count - 1), buf, offset, sdata)
            offset = offset + count
        elif dtype == 's':
            buf[offset:offset+count] = data
            offset = offset + count
        elif count <= 1:
            #buf[offset_md:sizeof_data_type] = pack('!'+dtype, data)
            struct.pack_into('!%s' % (dtype), buf, offset, data)
            offset = offset + sizeof_data_type
        else:
            struct.pack_into('!%d%s' % (count, dtype), buf, offset, *data)
            offset = offset + (count * sizeof_data_type)
            #for ix in range(count):
            #    #buf[offset_md:sizeof_data_type] = pack('!'+dtype, data[ix])
            #    struct.pack_into('!'+dtype, buf, offset, data[ix])
            #    offset = offset + sizeof_data_type
        return offset

    def serialize(self):
        l = self.size()
        buf = array.array('c', '\0' * l)
        length, left = self.serialize_to(buf, l)
        return buf
    def serialize_to(self, buf, length):
        j = 0
        self.left = 0
        if(self.check_for_sub_msg_cycles() < 0):
            if(Layer6Msg._SET_DEBUG_INFO): 
                self.debug = "%d-%s" % (self.error_code, self.get_error_str())
            raise Layer6Error(self.error_code)
            #return buf, -1, self.left
    
        field = None
    
        if(length < Layer6.L6_BASE_MSG_HDR_LENGTH):
            self.error_code = Layer6.L6_ERR_INSUFF_BUFFER_SIZE
            if(Layer6Msg._SET_DEBUG_INFO): 
                self.debug = "%d-%s" % (self.error_code, self.get_error_str())
            raise Layer6Error(self.error_code)
            #return buf, -1, self.left
    
        if(self.is_locked  == 0):
            self.is_locked = 1
            self.error_code = 0
            sz = self.size()
            self.offset_md = 0
            self.offset_d = 0
            self.itr = 0
            self.left = self.total_length
            
            if self.code & Layer6Msg.L6_FLAG_MSG_IS_TEMPLATE:
                self.error_code = Layer6.L6_ERR_NOT_IMPLEMENTED
                self.debug = "Template flag set in %d " % (self.code)
                raise Layer6Error(self.error_code, self.debug)
            
            self.code = 0   #TODO: handle templates and user-assigned code

            self.offset_md = self.pack(buf, self.offset_md, self.code, Layer6.sizeof_int08_t, 'B')
            self.offset_md = self.pack(buf, self.offset_md, len(self.qfields), Layer6.sizeof_int08_t, 'B')
            
            #unused bytes
            self.offset_md = self.pack(buf, self.offset_md, 0, Layer6.sizeof_int16_t, 'H')            
            s = self.total_length
            self.offset_md = self.pack(buf, self.offset_md, s, Layer6.sizeof_int16_t, 'H')
    
            s = self.metadata_length
            self.offset_md = self.pack(buf, self.offset_md, s, Layer6.sizeof_int16_t, 'H')
    
            self.offset_d = self.metadata_length
            if(self.total_length > length):
                self.is_partial = 1
    
        if(self.is_partial == 0):
            self.nfields = len(self.qfields)

            for self.itr in range(self.nfields):
                field = self.qfields[self.itr]
                #debug_field(field)
    
                #write metadata
                #type
                data_b = field.ftype 
                if (field.fid >= 0): 
                    data_b = data_b | Layer6.L6_FLAG_FIELD_HAS_ID
                if (field.name): 
                    data_b = data_b | Layer6.L6_FLAG_FIELD_HAS_NAME
                self.offset_md = self.pack(buf, self.offset_md, data_b, Layer6.sizeof_int08_t, 'B')

                #field count
                if (field.ftype & Layer6.L6_FLAG_FIELD_IS_ARRAY):
                    data_s = field.count
                    self.offset_md = self.pack(buf, self.offset_md, data_s, Layer6.sizeof_int16_t, 'H')

                #field id
                if (field.fid >= 0):
                    data_s = field.fid
                    self.offset_md = self.pack(buf, self.offset_md, data_s, Layer6.sizeof_int16_t, 'h')
                
                #field name
                if(field.name):
                    #data_str = field.name.encode('ascii') #('utf-8')
                    data_b = len(field.name) + 1
                    self.offset_md = self.pack(buf, self.offset_md, data_b, Layer6.sizeof_int08_t, 'B')
                    self.offset_md = self.pack(buf, self.offset_md, field.name, 1, 'str', data_b)
    
                #serialize fields
                if(field.ftype < Layer6.L6_PRIMITIVE_DATATYPE_MAX):
                    fmt = Layer6Msg.l6type_to_struct_fmt[field.ftype]
                    #write data
                    self.offset_d = self.pack(buf, self.offset_d, field.data, field.length, fmt, 1)
                elif (field.ftype == Layer6.L6_DATATYPE_L6MSG) :
                    #if field is a serialized sub-msg ref, re-set type to normal sub-msg because when serialized, there is no difference
                    if(field.serialized):
                        #it's a ref to un-deserialized msg, so no need to serialize
                        #write as is
                        self.offset_d = self.pack(buf, self.offset_d, field.data, field.length, 's', field.datalen)
                    else:
                        #else serialize and write to buffer
                        sub_msg = field.data
                        l = sub_msg.get_size_in_bytes()
                        subbuf = array.array('c', '\0' * l)
                        subsz, subleft = sub_msg.serialize_to(subbuf, l)
                        if(subsz < 0):
                            self.error_code = L6_ERR_SUB_MSG_DESERIALIZE
                            raise Layer6Error(self.error_code)
                            #self.reset()
                            #return buf, -1, self.left
                        if (subsz != l):
                            #TODO: set a proper error here
                            self.error_code = L6_ERR_SUB_MSG_DESERIALIZE
                            raise Layer6Error(self.error_code)
                        self.offset_d = self.pack(buf, self.offset_d, subbuf, 1, 's', subsz)
                else:
                    fmt = Layer6Msg.l6type_to_struct_fmt[field.ftype]
                    #write data
                    self.offset_d = self.pack(buf, self.offset_d, field.data, field.length, fmt, field.count)
                #debug_field(field)
            self.done = self.offset_d

        self.left = self.total_length - self.done
        left = self.left
        if(self.left == 0):
            self.is_partial = 0
            self.is_locked = 0
        #printf "\n partial serialize len=%d - done=%d = left=%d\n" % (self.total_length, self.done, self.left)
        return self.done, self.left

    def unpack(self, buf, offset, sizeof_data_type, dtype, count=1):
        #sanity check
        if (len(buf) - offset) < (sizeof_data_type*count): #struct.calcsize(dtype):
            #ln = buf.buffer_info()[1]
            return None, offset
            #raise Error?  
        data = None 
        if dtype == 's':
            #data = struct.unpack_from('!%ds' % count, buf, offset)
            data = buf[offset:offset + count]
            offset = offset + count
        elif dtype == 'str':
            data_tuple = struct.unpack_from('!%ds' % (count - 1), buf, offset)
            data = data_tuple[0] #get element from tuple
            offset = offset + count
        elif count <= 1:
            #buf[offset_md:sizeof_data_type] = pack('!'+dtype, data)
            data_tuple = struct.unpack_from('!%s' % dtype, buf, offset)
            data = data_tuple[0] #get element from tuple
            offset = offset + sizeof_data_type
        else:
            data = struct.unpack_from('!%d%s' % (count, dtype), buf, offset)
            offset = offset + (sizeof_data_type * count)
        return data, offset    
        

    def deserialize(self, buf, length):
        return self.deserialize_opt(buf, length, self.deep_copy_default)
    
    def deserialize_shallow(self, buf, length):
        return self.deserialize_opt(buf, length, 0)

    def deserialize_opt(self, buf, length, copy=0):
        j = 0
        left = 0
        ret = -1
    
        #ln = 0
        field = None
    
        if(length < Layer6.L6_BASE_MSG_HDR_LENGTH):
            self.error_code = Layer6.L6_ERR_INSUFF_BUFFER_SIZE
            if(Layer6Msg._SET_DEBUG_INFO): 
                self.debug = "%d-%s" % (self.error_code, self.get_error_str())
            raise Layer6Error(self.error_code)
            #return ret    #field not found
        
        #read base msg header
        if(self.is_locked == 0):
            self.reset(1)
            self.left = self.total_length
            self.offset_md = 0
            self.code, self.offset_md            = self.unpack(buf, self.offset_md, Layer6.sizeof_int08_t, 'B')

            if self.code & Layer6Msg.L6_FLAG_MSG_IS_TEMPLATE:
                self.error_code = Layer6.L6_ERR_NOT_IMPLEMENTED
                self.debug = "Template flag set in %d " % (self.code)
                raise Layer6Error(self.error_code, self.debug)
            
            self.nfields, self.offset_md         = self.unpack(buf, self.offset_md, Layer6.sizeof_int08_t, 'B')
            unused, self.offset_md               = self.unpack(buf, self.offset_md, Layer6.sizeof_int16_t, 'H') 
            self.total_length, self.offset_md    = self.unpack(buf, self.offset_md, Layer6.sizeof_int16_t, 'H')
            self.metadata_length, self.offset_md = self.unpack(buf, self.offset_md, Layer6.sizeof_int16_t, 'H')
            self.offset_d = self.metadata_length
            self.is_partial = 0

            if(self.total_length > length):
                self.is_partial = 1
                #self.is_locked  = 1
    
        if(self.is_partial):
            ret = 0
            if(self.done == 0):
                #dunno what to do
                ret = 0
    
        exp_total_length     = self.total_length
        self.offset_d        = self.metadata_length
        self.total_length    = Layer6.L6_BASE_MSG_HDR_LENGTH
        self.metadata_length = Layer6.L6_BASE_MSG_HDR_LENGTH
        self.data_length     = 0
        
        remaining = length - self.offset_d
    
        #read field metadata
        for self.itr in range(self.nfields):
            name  = None
            fid    = -1
            count = 1
            data = None
    
            #read metadata
            ftype, self.offset_md = self.unpack(buf, self.offset_md, Layer6.sizeof_int08_t, 'B')
            remaining = remaining - 1

            #field count
            if (ftype & Layer6.L6_FLAG_FIELD_IS_ARRAY):
                count, self.offset_md = self.unpack(buf, self.offset_md, Layer6.sizeof_int16_t, 'H')
            
            if(ftype & Layer6.L6_FLAG_FIELD_HAS_ID):
                fid, self.offset_md   = self.unpack(buf, self.offset_md, Layer6.sizeof_int16_t, 'h')                remaining = remaining - 2            

            if(ftype & Layer6.L6_FLAG_FIELD_HAS_NAME):
                #field is named
                nmln, self.offset_md = self.unpack(buf, self.offset_md, Layer6.sizeof_int08_t, 'B')
                name, self.offset_md = self.unpack(buf, self.offset_md, Layer6.sizeof_int08_t, 'str', nmln)
            else:
                name = None
    
            #self.__set_field_data_in_msg(name, fid, -1, ftype, data, len, count, 1)

            #deserialize fields
            ftype = ftype & Layer6.L6_FLAG_FIELD_TYPE_MASK
            if(ftype < Layer6.L6_PRIMITIVE_DATATYPE_MAX):
                dtype = Layer6Msg.l6type_to_struct_fmt[ftype]
                dsize = Layer6Msg.l6type_to_size[ftype]
                data, self.offset_d = self.unpack(buf, self.offset_d, dsize, dtype, 1)
                #set field
                if(self.__set_field_data_in_msg(name, fid, -1, ftype, data, dsize, 1, copy) == None):
                    self.reset(0)
                    if(Layer6Msg._SET_DEBUG_INFO): self.debug = "%d-%s" % (self.error_code, self.get_error_str())
                    return -1
            elif (ftype == Layer6.L6_DATATYPE_L6MSG):
                sublen = count                
                sub_msg = Layer6Msg()
    
                #deserialize shallow
                sub_msg.set_deep_copy_default(0)
                subbuf = buf[self.offset_d : self.offset_d + sublen]

                ret = sub_msg.deserialize(subbuf, sublen)
                if(ret < 0):
                    #self.reset(0)
                    #return ret
                    self.error_code = Layer6.L6_ERR_SUB_MSG_DESERIALIZE
                    self.debug = "%d-%s" % (self.error_code, self.get_error_str())
                    raise Layer6Error(self.error_code)

                #set
                field = self.__set_field_data_in_msg(name, fid, -1, ftype, sub_msg, 1, count, copy)
                if(field == None):
                    self.error_code = Layer6.L6_ERR_SUB_MSG_DESERIALIZE
                    self.debug = "%d-%s" % (self.error_code, self.get_error_str())
                    raise Layer6Error(self.error_code)

                field.internally_allocated = 1
                self.offset_d += sublen
            else: #array
                if(self.offset_d > length):
                    self.error_code = Layer6.L6_ERR_INSUFF_BUFFER_SIZE
                    self.debug = "%d-%s" % (self.error_code, self.get_error_str())
                    raise Layer6Error(self.error_code)
                    #return -1
                dtype = Layer6Msg.l6type_to_struct_fmt[ftype]
                dsize = Layer6Msg.l6type_to_size[ftype]
                data, self.offset_d = self.unpack(buf, self.offset_d, dsize, dtype, count)
                field = self.__set_field_data_in_msg(name, fid, -1, ftype, data, dsize, count, copy)

        self.done = self.offset_d
        self.left = self.total_length - self.done
        #print "\n returning %d (expected = %d) - %d = %d" % (self.total_length, exp_total_length, self.done, self.left)
        return self.left


    def check_for_sub_msg_cycles(self): 
        ret = 0
        #remove from qsub_msgs
        ln = len(self.qsub_msgs)
        for i in range(ln):
            field = self.qsub_msgs[i]
            if(field.ftype == Layer6.L6_DATATYPE_L6MSG):
                sub_msg = field.data
                if(sub_msg == self):
                    #recursion!
                    self.error_code = Layer6.L6_ERR_RECURSIVE_SUB_MSG
                    raise Layer6Error(self.error_code)
                    #return -1
                ret = sub_msg.check_for_sub_msg_cycles()
        return ret

    def reset(self, reset_err=1):
        field = None
        #empty sub_msgs
        self.qsub_msgs  = []
        self.qfields    = []
        self.htbl_fname.clear()
        self.htbl_fid.clear()
                
        self.code               = 0
        #self.has_metadata      = False
        #self.has_subject       = False
        self.total_length       = Layer6.L6_BASE_MSG_HDR_LENGTH
        self.metadata_length    = Layer6.L6_BASE_MSG_HDR_LENGTH
        self.data_length        = 0
        if(reset_err):
            self.error_code     = 0

        self.deep_copy_default  = 1
        self.fidgen             = 1024
        #if(self.subject != None):
        #    self.subject       = None
        self.is_locked          = 0
        self.is_partial         = 0
        self.offset_md          = 0
        self.offset_d           = 0
        self.left               = 0
        self.done               = 0
        self.debug              = ''
        return 0


    def set_deep_copy_default(self, mode):
        self.deep_copy_default = mode
    
    def get_deep_copy_default(self):
        return self.deep_copy_default
    
    def set_auto_byte_order_mode(self, byteorder):
        self.auto_byte_order_mode = byteorder
    
    def get_auto_byte_order_mode(self):
        return self.auto_byte_order_mode

#/*********************************** GET/SET DATA FUNCTIONS *********************************/
    @staticmethod
    def def_add_method(ftype, typeid, typesize, deepmode = '', accessmode='', marshaller=None):
        if (typeid in Layer6Msg.l6_primitives) and (deepmode != ''): return  # doesn't make sense otherwise
        if typeid in Layer6Msg.l6_primitives:
            def addmethod(self, data):
                return self.__set_field_data_in_msg(None, -1, -1, typeid, data, typesize, 1, self.deep_copy_default)
        elif typeid == Layer6.L6_DATATYPE_L6MSG:
            #arrays
            if deepmode =='':   #deep copy data
                def addmethod(self, data):
                    return self.__set_field_data_in_msg(None, -1, -1, typeid, data, typesize, data.get_size_in_bytes(), self.deep_copy_default)
            else:   #shallow copy
                def addmethod(self, data):
                    return self.__set_field_data_in_msg(None, -1, -1, typeid, data, typesize, data.get_size_in_bytes(), 0)
        elif typeid == Layer6.L6_DATATYPE_STRING:
            if deepmode =='':   #deep copy data
                def addmethod(self, data):
                    return self.__set_field_data_in_msg(None, -1, -1, typeid, data, typesize, len(data) + 1, self.deep_copy_default)
            else:   #shallow copy
                def addmethod(self, data):
                    return self.__set_field_data_in_msg(None, -1, -1, typeid, data, typesize, len(data) + 1, 0)
        else:
            #arrays
            if deepmode =='':   #deep copy data
                def addmethod(self, data):
                    return self.__set_field_data_in_msg(None, -1, -1, typeid, data, typesize, len(data), self.deep_copy_default)
            else:   #shallow copy
                def addmethod(self, data):
                    return self.__set_field_data_in_msg(None, -1, -1, typeid, data, typesize, len(data), 0)

        deepmodestr   = ''
        if (deepmode   != ''): deepmodestr   = '_'+deepmode
        
        prefix = 'add_'        
        
        methodname = prefix + ftype + deepmodestr
        
        addmethod.__name__ = methodname
        addmethod.__doc__  = "method %s" % addmethod.__name__
        setattr(Layer6Msg, addmethod.__name__, addmethod)
        #print 'added method :'+ addmethod.__name__
        return

    @staticmethod
    def def_set_method(ftype, typeid, typesize, deepmode = '', accessmode = '', marshaller=None):
        if (typeid in Layer6Msg.l6_primitives) and (deepmode != ''): return  # doesn't make sense otherwise
        if typeid in Layer6Msg.l6_primitives:
            if accessmode == '' or accessmode == 'w_id': #by id
                def setmethod(self, fid, data):
                    return self.__set_field_data_in_msg(None, fid, -1, typeid, data, typesize, 1, self.deep_copy_default)
            elif accessmode == 'named':   #by name
                def setmethod(self, name, data):
                    return self.__set_field_data_in_msg(name, -1, -1, typeid, data, typesize, 1, self.deep_copy_default)
            else: #by index
                def setmethod(self, index, data):
                    return self.__set_field_data_in_msg(None, -1, index, typeid, data, typesize, 1, self.deep_copy_default)
        elif typeid == Layer6.L6_DATATYPE_L6MSG:
            #arrays
            if deepmode =='':   #deep copy data
                if accessmode == '' or accessmode == 'w_id':    #by id
                    def setmethod(self, fid, data):
                        return self.__set_field_data_in_msg(None, fid, -1, typeid, data, typesize, data.get_size_in_bytes(), self.deep_copy_default)
                elif accessmode == 'named': #by name
                    def setmethod(self, name, data):
                        return self.__set_field_data_in_msg(name, -1, -1, typeid, data, typesize, data.get_size_in_bytes(), self.deep_copy_default)
                else:  #by index 
                    def setmethod(self, index, data):
                        return self.__set_field_data_in_msg(None, -1, index, typeid, data, typesize, data.get_size_in_bytes(), self.deep_copy_default)
            else:   #shallow copy
                if accessmode == '' or accessmode == 'w_id': #by id
                    def setmethod(self, fid, data):
                        return self.__set_field_data_in_msg(None, fid, -1, typeid, data, typesize, data.get_size_in_bytes(), 0)
                elif accessmode == 'named': #by name
                    def setmethod(self, name, data):
                        return self.__set_field_data_in_msg(name, -1, -1, typeid, data, typesize, data.get_size_in_bytes(), 0)
                else:   #by index
                    def setmethod(self, index, data):
                        return self.__set_field_data_in_msg(None, -1, index, typeid, data, typesize, data.get_size_in_bytes(), 0)
        elif typeid == Layer6.L6_DATATYPE_STRING:
            if deepmode =='':   #deep copy data
                if accessmode == '' or accessmode == 'w_id':    #by id
                    def setmethod(self, fid, data):
                        return self.__set_field_data_in_msg(None, fid, -1, typeid, data, typesize, len(data) + 1, self.deep_copy_default)
                elif accessmode == 'named': #by name
                    def setmethod(self, name, data):
                        return self.__set_field_data_in_msg(name, -1, -1, typeid, data, typesize, len(data) + 1, self.deep_copy_default)
                else:  #by index 
                    def setmethod(self, index, data):
                        return self.__set_field_data_in_msg(None, -1, index, typeid, data, typesize, len(data) + 1, self.deep_copy_default)
            else:   #shallow copy
                if accessmode == '' or accessmode == 'w_id': #by id
                    def setmethod(self, fid, data):
                        return self.__set_field_data_in_msg(None, fid, -1, typeid, data, typesize, len(data) + 1, 0)
                elif accessmode == 'named': #by name
                    def setmethod(self, name, data):
                        return self.__set_field_data_in_msg(name, -1, -1, typeid, data, typesize, len(data) + 1, 0)
                else:   #by index
                    def setmethod(self, index, data):
                        return self.__set_field_data_in_msg(None, -1, index, typeid, data, typesize, len(data) + 1, 0)
        else:
            #arrays
            if deepmode =='':   #deep copy data
                if accessmode == '' or accessmode == 'w_id':    #by id
                    def setmethod(self, fid, data):
                        return self.__set_field_data_in_msg(None, fid, -1, typeid, data, typesize, len(data), self.deep_copy_default)
                elif accessmode == 'named': #by name
                    def setmethod(self, name, data):
                        return self.__set_field_data_in_msg(name, -1, -1, typeid, data, typesize, len(data), self.deep_copy_default)
                else:  #by index 
                    def setmethod(self, index, data):
                        return self.__set_field_data_in_msg(None, -1, index, typeid, data, typesize, len(data), self.deep_copy_default)
            else:   #shallow copy
                if accessmode == '' or accessmode == 'w_id': #by id
                    def setmethod(self, fid, data):
                        return self.__set_field_data_in_msg(None, fid, -1, typeid, data, typesize, len(data), 0)
                elif accessmode == 'named': #by name
                    def setmethod(self, name, data):
                        return self.__set_field_data_in_msg(name, -1, -1, typeid, data, typesize, len(data), 0)
                else:   #by index
                    def setmethod(self, index, data):
                        return self.__set_field_data_in_msg(None, -1, index, typeid, data, typesize, len(data), 0)
        deepmodestr   = ''
        accessmodestr = ''
        if (deepmode   != ''): deepmodestr   = '_'+deepmode
        if (accessmode != ''): accessmodestr = '_'+accessmode
        prefix = 'set_'
        
        methodname = prefix + ftype + deepmodestr + accessmodestr
        setmethod.__name__ = methodname
        setmethod.__doc__  = "method %s" % setmethod.__name__
        setattr(Layer6Msg, setmethod.__name__, setmethod)
        #print 'added method :'+ setmethod.__name__
        return

    @staticmethod
    def def_get_method(ftype, typeid, typesize, deepmode = '', accessmode = '', unmarshaller=None):
        if (typeid in Layer6Msg.l6_primitives) and (deepmode != ''): return  # doesn't make sense otherwise
        if typeid in Layer6Msg.l6_primitives:
            if accessmode == '' or accessmode == 'w_id': #by id
                def getmethod(self, fid):
                    return self.__get_field_data_in_msg(None, fid, -1, typeid, typesize, 0, 1)
            elif accessmode == 'named':   #by name
                def getmethod(self, name):
                    return self.__get_field_data_in_msg(name, -1, -1, typeid, typesize, 0, 1)
            else:   #by index
                def getmethod(self, index):
                    return self.__get_field_data_in_msg(None, -1, index, typeid, typesize, 0, 1)
        elif typeid == Layer6.L6_DATATYPE_L6MSG:
            #arrays
            if deepmode =='':   #deep copy data
                if accessmode == '':    #by id
                    def getmethod(self, fid):
                        return self.__get_field_data_in_msg(None, fid, -1, typeid, typesize, 0, 1, 1)
                elif accessmode == 'named':  #by name 
                    def getmethod(self, name):
                        return self.__get_field_data_in_msg(name, -1, -1, typeid, typesize, 0, 1, 1)
                else:   #by index
                    def getmethod(self, index):
                        return self.__get_field_data_in_msg(None, -1, index, typeid, typesize, 0, 1, 1)
            else:   #shallow copy
                if accessmode == '' or accessmode == 'w_id': #by id
                    def getmethod(self, fid):
                        return self.__get_field_data_in_msg(None, fid, -1, typeid, typesize, 0, -1)
                elif accessmode == 'named':   #by name
                    def getmethod(self, name):
                        return self.__get_field_data_in_msg(name, -1, -1, typeid, typesize, 0, -1)
                else:   #by index
                    def getmethod(self, index):
                        return self.__get_field_data_in_msg(None, -1, index, typeid, typesize, 0, -1)
        elif typeid == Layer6.L6_DATATYPE_STRING:
            #arrays
            if deepmode =='':   #deep copy data
                if accessmode == '' or accessmode == 'w_id':    #by id
                    def getmethod(self, fid):
                        return self.__get_field_data_in_msg(None, fid, -1, typeid, typesize, 0, -1, 1)
                elif accessmode == 'named':  #by name 
                    def getmethod(self, name):
                        return self.__get_field_data_in_msg(name, -1, -1, typeid, typesize, 0, -1, 1)
                else:   #by index
                    def getmethod(self, index):
                        return self.__get_field_data_in_msg(None, -1, index, typeid, typesize, 0, -1, 1)
            else:   #shallow copy
                if accessmode == '' or accessmode == 'w_id': #by id
                    def getmethod(self, fid):
                        return self.__get_field_data_in_msg(None, fid, -1, typeid, typesize, 0, -1)
                elif accessmode == 'named':   #by name
                    def getmethod(self, name):
                        return self.__get_field_data_in_msg(name, -1, -1, typeid, typesize, 0, -1)
                else:   #by index
                    def getmethod(self, index):
                        return self.__get_field_data_in_msg(None, -1, index, typeid, typesize, 0, -1)
        else:
            #arrays
            if deepmode =='':   #deep copy data
                if accessmode == '' or accessmode == 'w_id':    #by id
                    def getmethod(self, fid, offset=0, count=-1):
                        return self.__get_field_data_in_msg(None, fid, -1, typeid, typesize, offset, count, 1)
                elif accessmode == 'named':  #by name 
                    def getmethod(self, name, offset=0, count=-1):
                        return self.__get_field_data_in_msg(name, -1, -1, typeid, typesize, offset, count, 1)
                else:   #by index
                    def getmethod(self, index, offset=0, count=-1):
                        return self.__get_field_data_in_msg(None, -1, index, typeid, typesize, offset, count, 1)
            else:   #shallow copy
                if accessmode == '' or accessmode == 'w_id': #by id
                    def getmethod(self, fid, offset=0, count=-1):
                        return self.__get_field_data_in_msg(None, fid, -1, typeid, typesize, offset, count)
                elif accessmode == 'named':   #by name
                    def getmethod(self, name, offset=0, count=-1):
                        return self.__get_field_data_in_msg(name, -1, -1, typeid, typesize, offset, count)
                else:   #by index
                    def getmethod(self, index, offset=0, count=-1):
                        return self.__get_field_data_in_msg(None, -1, index, typeid, typesize, offset, count)

        deepmodestr   = ''
        accessmodestr = ''
        if (deepmode   != ''): deepmodestr   = '_'+deepmode
        if (accessmode != ''): accessmodestr = '_'+accessmode
        methodname = 'get_' + ftype + deepmodestr + accessmodestr
        #methodname = 'get_' + ftype + (deepmode != '' ? '_'+deepmode : '') + (accessmode != '' ? '_'+accessmode : '')
        getmethod.__name__ = methodname
        getmethod.__doc__  = "method %s" % getmethod.__name__
        setattr(Layer6Msg, getmethod.__name__, getmethod)
        #print 'added method :'+ getmethod.__name__
        return

    @staticmethod
    def gen_methods():
        #actions = ['set', 'get']
        #print 'Generating accessors and mutators'
        types = {
            'ubyte'         : Layer6.L6_DATATYPE_UINT8,
            'byte'          : Layer6.L6_DATATYPE_INT8,
            'ushort'        : Layer6.L6_DATATYPE_UINT16,
            'short'         : Layer6.L6_DATATYPE_SHORT, 
            'uint'          : Layer6.L6_DATATYPE_UINT32, 
            'int'           : Layer6.L6_DATATYPE_INT32, 
            'ulong'         : Layer6.L6_DATATYPE_UINT64,
            'long'          : Layer6.L6_DATATYPE_LONG, 
            'float'         : Layer6.L6_DATATYPE_FLOAT, 
            'double'        : Layer6.L6_DATATYPE_DOUBLE, 
            'string'        : Layer6.L6_DATATYPE_STRING, 
            'ubyte_array'   : Layer6.L6_DATATYPE_UINT8_ARRAY,
            'byte_array'    : Layer6.L6_DATATYPE_BYTES, 
            'ushort_array'  : Layer6.L6_DATATYPE_UINT16_ARRAY, 
            'short_array'   : Layer6.L6_DATATYPE_SHORT_ARRAY, 
            'uint_array'    : Layer6.L6_DATATYPE_UINT32_ARRAY,
            'int_array'     : Layer6.L6_DATATYPE_INT_ARRAY, 
            'ulong_array'   : Layer6.L6_DATATYPE_UINT64_ARRAY, 
            'long_array'    : Layer6.L6_DATATYPE_LONG_ARRAY, 
            'float_array'   : Layer6.L6_DATATYPE_FLOAT_ARRAY, 
            'double_array'  : Layer6.L6_DATATYPE_DOUBLE_ARRAY, 
            'layer6_msg'    : Layer6.L6_DATATYPE_L6MSG
        }
        marshallers = {
            Layer6.L6_DATATYPE_UINT8        : None, 
            Layer6.L6_DATATYPE_INT8         : None, 
            Layer6.L6_DATATYPE_UINT16       : None, 
            Layer6.L6_DATATYPE_SHORT        : Layer6Msg.to_short16, 
            Layer6.L6_DATATYPE_UINT32       : None, 
            Layer6.L6_DATATYPE_INT32        : None, 
            Layer6.L6_DATATYPE_UINT64       : None, 
            Layer6.L6_DATATYPE_LONG         : None, #Layer6Msg.to_long64, 
            Layer6.L6_DATATYPE_FLOAT        : Layer6Msg.to_float32, 
            Layer6.L6_DATATYPE_DOUBLE       : None, 
            Layer6.L6_DATATYPE_STRING       : None,
            Layer6.L6_DATATYPE_UINT8_ARRAY  : None,  
            Layer6.L6_DATATYPE_BYTES        : None, 
            Layer6.L6_DATATYPE_UINT16_ARRAY : None,  
            Layer6.L6_DATATYPE_SHORT_ARRAY  : None,
            Layer6.L6_DATATYPE_UINT32_ARRAY : None,  
            Layer6.L6_DATATYPE_INT_ARRAY    : None, 
            Layer6.L6_DATATYPE_UINT64_ARRAY : None,  
            Layer6.L6_DATATYPE_LONG_ARRAY   : None, 
            Layer6.L6_DATATYPE_FLOAT_ARRAY  : None, 
            Layer6.L6_DATATYPE_DOUBLE_ARRAY : None, 
            Layer6.L6_DATATYPE_L6MSG        : None
        }
        unmarshallers = {
            Layer6.L6_DATATYPE_UINT8        : None, 
            Layer6.L6_DATATYPE_INT8         : None, 
            Layer6.L6_DATATYPE_UINT16       : None, 
            Layer6.L6_DATATYPE_SHORT        : Layer6Msg.from_short16, 
            Layer6.L6_DATATYPE_UINT32       : None, 
            Layer6.L6_DATATYPE_INT32        : None, 
            Layer6.L6_DATATYPE_UINT64       : None,
            Layer6.L6_DATATYPE_LONG         : None, 
            Layer6.L6_DATATYPE_FLOAT        : Layer6Msg.from_float32, 
            Layer6.L6_DATATYPE_DOUBLE       : None, 
            Layer6.L6_DATATYPE_STRING       : None,
            Layer6.L6_DATATYPE_UINT8_ARRAY  : None,  
            Layer6.L6_DATATYPE_BYTES        : None, 
            Layer6.L6_DATATYPE_UINT16_ARRAY : None,  
            Layer6.L6_DATATYPE_SHORT_ARRAY  : None,
            Layer6.L6_DATATYPE_UINT32_ARRAY : None,  
            Layer6.L6_DATATYPE_INT_ARRAY    : None, 
            Layer6.L6_DATATYPE_UINT64_ARRAY : None,  
            Layer6.L6_DATATYPE_LONG_ARRAY   : None, 
            Layer6.L6_DATATYPE_FLOAT_ARRAY  : None, 
            Layer6.L6_DATATYPE_DOUBLE_ARRAY : None, 
            Layer6.L6_DATATYPE_L6MSG        : None
        }
        deepmodes = ['', 'ptr']
        accessmodes = ['', 'named', 'at_index']
        for typename, typeid in types.items():
            for deepmode in deepmodes:
                Layer6Msg.def_add_method(typename, typeid, Layer6Msg.l6type_to_size[typeid], deepmode, '', unmarshallers[typeid])
        for typename, typeid in types.items():
            for deepmode in deepmodes:
                for accessmode in accessmodes:
                    Layer6Msg.def_set_method(typename, typeid, Layer6Msg.l6type_to_size[typeid], deepmode, accessmode, marshallers[typeid])
        for typename, typeid in types.items():
            for deepmode in deepmodes:
                for accessmode in accessmodes:
                    Layer6Msg.def_get_method(typename, typeid, Layer6Msg.l6type_to_size[typeid], deepmode, accessmode, unmarshallers[typeid])
        #set gen flag
        Layer6Msg.__METHODS_GENERATED__ = 1
        

    def set_code(self, code):
        self.code = code

    def get_code(self):
        return self.code

    def dup(self, msg2=None):
        if msg2 == None: 
            msg2 = Layer6Msg()
        else:
            msg2.reset(1)   #reset destination msg
        #iterate thru fields and set
        for field in self.qfields:
            #self.debug_field(field)
            retfield = msg2.__set_field_data_in_msg(field.name, field.fid, -1, field.ftype, field.data, field.length, field.count, msg2.deep_copy_default)
            if(retfield == None):
                #some error in copying
                return None
        return msg2

# *********************************** FIELD FUNCTIONS *********************************
    def remove_field(self, fid):
        field = self.__get_field_w_id(fid)
        return self.__remove_field_in_msg(field)
    def remove_field_at_index(self, index):
        field = __get_field_at_index(msg, index)
        return self.__remove_field_in_msg(field)


    def remove_field_named(self, fieldname):
        field = self.__get_field_named(fieldname)
        return self.__remove_field_in_msg(field)

    #field ops
    def __get_field_at_index(self, index):
        if index >= len(self.qfields):
            self.error_code = Layer6.L6_ERR_FIELD_NOT_FOUND
            raise Layer6Error(self.error_code)            #return None    #field not found
        return self.qfields[index]

    def get_num_fields(self):
        return len(self.qfields)

    def get_field_type_at_index(self, index):
        field = self.__get_field_at_index(index)
        if(field):
            return field.ftype
        return -1    #field not found

    def get_field_id_at_index(self, index):
        field = self.__get_field_at_index(index)
        if(field):
            return field.fid
        return -1    #field not found

    def get_field_name_at_index(self, index):
        field = self.__get_field_at_index(index)
        if(field):
            if field.name:
                return field.name
            else:
                self.error_code = Layer6.L6_ERR_FIELD_UNNAMED
                return None
        return None   #field not found


    def is_field_named(self, fid):
        field = self.__get_field_w_id(fid)
        if(field):
            if(field.name):
                return True
            else:
                return False
        return -1    #field not found


    def is_field_named_at_index(self, index):
        field = self.__get_field_at_index(index)
        if(field):
            if field.name:
                return True
            else:
                return False
        return None   #field not found

    def free_field(self, field):
        if(field):
            field.reset()
        del field

    def get_field_unit_size(self, fid):
        field = self.__get_field_w_id(fid)
        if(field):
            return field.length
        return -1    #field not found

    def get_field_unit_size_named(self, name):
        field = self.__get_field_named(name)
        if(field):
            return field.length
        return -1    #field not found

    def get_field_size_in_bytes(self, fid):
        field = self.__get_field_w_id(fid)
        if(field):
            return field.length * field.count
        return -1    #field not found

    def get_field_size_in_bytes_named(self, name):
        field = self.__get_field_named(name)
        if(field):
            return field.length * field.count
        return -1    #field not found

    def get_field_count_at_index(self, index):
        field = self.__get_field_at_index(index)
        if(field):
            return field.count
        return -1    #field not found

    def get_field_count(self, fid):
        field = self.__get_field_w_id(fid)
        if(field):
            return field.count
        return -1    #field not found

    def get_field_count_named(self, name):
        field = self.__get_field_named(name)
        if(field):
            return field.count
        return -1    #field not found
    def __get_field_w_id(self, fid, raise_if_missing=True):
        field = None
        try:
            field = self.htbl_fid[fid]
        except (KeyError):
            field = None
            self.error_code = Layer6.L6_ERR_FIELD_NOT_FOUND
            if raise_if_missing:
                raise Layer6Error(self.error_code)
        return field

    def __get_field_named(self, name, raise_if_missing=True):
        field = None
        try:
            field = self.htbl_fname[name]
        except (KeyError):
            field = None
            self.error_code = Layer6.L6_ERR_FIELD_NOT_FOUND
            if raise_if_missing:
                raise Layer6Error(self.error_code)
        return field

    def get_field_name(self, fid):
        field = self.__get_field_w_id(fid)
        if(field):
            return field.name
        return None

    def get_field_type(self, fid):
        field = self.__get_field_w_id(fid)
        return field.ftype

    def get_field_type_named(self, name):
        return self.getFieldTypeNamed(name)

    def getFieldTypeNamed(self, name):
        field = self.__get_field_named(name)
        return field.ftype

#generate methods on import
Layer6Msg.gen_methods()