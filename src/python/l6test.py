from layer6 import *

import socket
import sys
import os, traceback, re
import socket
import urllib
import random
import binascii

import threading
import SocketServer

class Layer6Test(SocketServer.BaseRequestHandler):
    def __init__(self, request, client_address, server):
        SocketServer.BaseRequestHandler.__init__(self, request, client_address, server)
        self.port = 6063
        self.isServer = 0

    def setup(self):
        print 'Server:', str(self.client_address), 'connected.'
        pass

    def finish(self):
        print 'Server:', str(self.client_address), 'disconnecting...'

    def handle(self):
        data = self.request.recv(4096)
        try:
            opstr = self.handleClient(data, len(data))
        except Layer6Error as e:
            opstr = "Unexpected L6 error (%s) - %s" % (sys.exc_info()[0], e)
            print opstr
        self.request.sendall(opstr)
        print "Server: wrote %d, closing connection" % (len(opstr))

    @staticmethod        
    def dump(msg, indent):
        nfields = msg.get_num_fields()
        opstr = ''
        for i in range(nfields):
            opstr += ("\n%s%d:" % (indent, i))
            fid = -1
            fid = msg.get_field_id_at_index(i)
            opstr += (" id=%d" % fid)
            if msg.is_field_named_at_index(i):
                opstr += (" name='%s'" % msg.get_field_name_at_index(i))
            else:
                opstr += (" name=''")

            ftype = msg.get_field_type_at_index(i)
            opstr += (" type=%d " % ftype)
            
            if ftype == Layer6.L6_DATATYPE_STRING:
                opstr += ("(string) data=\"%s\"" % msg.get_string_ptr_at_index(i))
            elif ftype == Layer6.L6_DATATYPE_BYTES:
                opstr += ("(byte [%d]) data=%s " % (msg.get_field_count_at_index(i), str(msg.get_byte_array_at_index(i))))
            elif ftype == Layer6.L6_DATATYPE_SHORT:
                opstr += ("(short) data=%d" % msg.get_short_at_index(i))
            elif ftype == Layer6.L6_DATATYPE_SHORT_ARRAY:
                opstr += ("(short [%d]) data=%s" % (msg.get_field_count_at_index(i), str(msg.get_short_array_at_index(i))))
            elif ftype == Layer6.L6_DATATYPE_INT32:
                opstr += ("(int) data=%d" % msg.get_int_at_index(i))
            elif ftype == Layer6.L6_DATATYPE_INT32_ARRAY:
                opstr += ("(int [%d]) data=%s" % (msg.get_field_count_at_index(i), str(msg.get_int_array_at_index(i))))
            #elif ftype == Layer6. L6_DATATYPE_INT64:
            elif ftype == Layer6.L6_DATATYPE_LONG:
                opstr += ("(long long int) data=%ld" % msg.get_long_at_index(i))
            #elif ftype == Layer6. L6_DATATYPE_INT64_ARRAY:
            elif ftype == Layer6.L6_DATATYPE_LONG_ARRAY:
                opstr += ("(long long int [%d]) data=%s" % (msg.get_field_count_at_index(i), str(msg.get_long_array_at_index(i))))
            elif ftype == Layer6.L6_DATATYPE_FLOAT:
                opstr += ("(float) data=%f" % msg.get_float_at_index(i))
            elif ftype == Layer6.L6_DATATYPE_FLOAT_ARRAY:
                opstr += ("(float [%d]) data=%s" % (msg.get_field_count_at_index(i), str(msg.get_float_array_at_index(i))))
            elif ftype == Layer6.L6_DATATYPE_DOUBLE:
                opstr += ("(double) data=%f" % msg.get_double_at_index(i))
            elif ftype == Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                opstr += ("(double [%d]) data=%s" % (msg.get_field_count_at_index(i), str(msg.get_double_array_at_index(i))))
            elif ftype == Layer6. L6_DATATYPE_L6MSG:
                opstr += "(l6msg [%d]) data={" % (msg.get_field_count_at_index(i))
                sub_msg = msg.get_layer6_msg_ptr_at_index(i)
                if(sub_msg == None):
                    ret ="\nError msg=%s submsg=%s..." % (msg.get_debug_info(), sub_msg.get_debug_info())
                    print ret
                    return ret
                opstr += (Layer6Test.dump(sub_msg, indent+'\t'))
                sub_msg.free()
                opstr += "}"
        return opstr

    @staticmethod
    def deserialize(buf, length):
        left = 0
        #deserialize
        print "Server: deserializing.... "
        msg = Layer6Msg()
    
        ret = msg.deserialize(buf, length)
        if(ret < -1000):
            ret = "Error:  %s" % msg.get_debug_info()
            print ret
            return ret 
        if (ret > 0): left = ret
        numf = msg.get_num_fields()
        print "Server: deserialized %d left=%d, num fields=%d" % (ret, left, numf)
        opstr = Layer6Test.dump(msg, "\t")
        print "Server: msg dump [len=%d]=%s" % (ret, opstr)
        msg.free()
        print "Server: freed recvd msg "
        return opstr

    @staticmethod
    def handleClient(data, ret, buf=None, length=0):
        offset = 0
        
        if(buf != None):
            opst = Layer6Test.deserialize(buf, length)
            print "Server: deserialized %d bytes .... " % offset
            return 0

        recv_buf = array.array('c', data) #'\0'*2048)
        
        print "Server: Handling new client... " 
        #ret = sock.recv_into(recv_buf)
        
        if(ret < 0):
            print ("Server: Error no bytes read.... ")
            sock.close()
            return -1
        else:
            print "Server: %d bytes read.... " % ret

        opstr = Layer6Test.deserialize(recv_buf, ret)
        opstr += "\nDone!!!"
        return opstr

    @staticmethod
    def runClient(host, p, nosockets, fname=None):
        #char *buffer = (char*)malloc(sizeof(char)*4096)
        #buffer = []
        length = 0
        ret = 0
        if(p <= 0):
            p = 6063
        
        darr = [1.2, 3.45, 5.67, 7.89, 0.91]
        iarr = range(9, -1, -1)
        print "Client: starting up "
        
        msg = Layer6Msg()
        add_subsubmsg = True
        #set data
        try:
            print ("short, ")
            msg.set_short(1, 43)
            
            print ("int, ")
            msg.set_int(2, -23)
            
            print ("long, ")
            msg.set_long_named("l", long(3847))
            
            print ("float, ")
            msg.set_float_named("float", 39.31)
            
            print ("double, ")
            msg.add_double(1278.409823)

            print ("overwrite float, ")
            msg.set_float_at_index(3, 49.312);
            
            print ("double array, ")
            msg.set_double_array_named("da", darr)
            
            print ("int array, ")
            msg.set_int_array_ptr(4, iarr)
            
            print ("string, ")
            msg.set_string(5,"Yeeehaaaah!")
            
            print ("submsg... ")
            sub_msg = Layer6Msg()
            sub_msg.set_string_named("sub-f1", "sub msg field 1")
            sub_msg.set_string_ptr(5,"sub msg field 2!")
            sub_msg.set_string_ptr_named("temp", "I'm just gonna get deleted in a few lines...")

            msg.set_layer6_msg_ptr_named("sub", sub_msg)

            print "\nClient: Added SubMsg w/ nfields = %d, size=%d..." % (sub_msg.get_num_fields(), sub_msg.size())
            print "to msg, now having nfields = %d, size=%d..." % (msg.get_num_fields(), msg.size())
            tempintarr = [1,2,3,5,7,11,13]
            sub_msg.set_int_array_ptr(3, tempintarr)
            
            print "\nClient: Added field to SubMsg w/ nfields = %d, size=%d..." % (sub_msg.get_num_fields(), sub_msg.size())
            print "to msg, now having nfields = %d, size=%d..." % (msg.get_num_fields(), msg.size())
            
            if add_subsubmsg:
                sub_submsg = Layer6Msg()
                sub_submsg.set_short_array_named("short-array", [1, 3, 4, 5])
                sub_submsg.set_string_named("sub-sub-key1", "qwerty")
                sub_submsg.set_string_named("yowzah", "Lorem ipsum dolor sit amet.")
                sub_msg.set_layer6_msg_ptr_named("subsub", sub_submsg)
                
                sub_submsg_temp = Layer6Msg()
                sub_submsg_temp.set_byte_array_named("garbage", [4] * 16)
                
                sub_submsg.set_layer6_msg_named("subsubTmp", sub_submsg_temp)
                sub_msg.set_int(32, 32)
                
                print "Client: Msg nfields =", msg.get_num_fields(), ", serialized exp=[", msg.size(), "] bytes"
                
                #removing
                sub_submsg.remove_field_named("subsubTmp")
                sub_msg.remove_field(32)
                
                #set another copy
                msg.set_layer6_msg(99, sub_submsg);

            sub_msg.remove_field_named("temp")

            print "\nClient: Removed field from SubMsg w/ nfields = %d, size=%d..." % (sub_msg.get_num_fields(), sub_msg.size())
            print "to msg, now having nfields = %d, size=%d..." % (msg.get_num_fields(), msg.size())

            #get and re-add a submsg if it's there
            sub_submsg = msg.get_layer6_msg_ptr(99);
            
            if sub_submsg:  #should always get here unless we disable exceptions (which is not currently possible)
                #add a new field
                larr = [112, 35, 81321, 34, 5589, 144]
                sub_submsg.add_long_array_ptr(larr)
                
                #remove an old field
                sub_submsg.remove_field_named("yowzah")
            else:
                print "\nClient: Could not get submsg field at id %d in msg w/ nfields = %d, size=%d..." \
                            % (99, msg.get_num_fields(), msg.size()) 
            
            msg.add_byte_array([9, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0]);
            
            print "\nClient: Msg nfields = %d, serializing..." % (msg.get_num_fields())
            
            left = 0 
            buf = array.array('c', '\0' * msg.size())
            #buf = msg.serialize()
            length, left = msg.serialize_to(buf, len(buf))
        #except (RuntimeError, TypeError, NameError):
        except Layer6Error as e:
            print "Unexpected L6 error (", sys.exc_info()[0],") - ", e, " debug[%s]" % msg.get_debug_info()
            #return -1
            raise
        
        ret = msg.get_size_in_bytes()
        
        print "\n\nClient: Msg bytes expected=%d actual=%d" % (ret, len(buf)) # + ":" +dprint(buf, 0, len))
        #print "\nClient: buf=[%s]" % binascii.hexlify(buf)
        
        if(nosockets):
            if(fname):
                fp = open(fname, "wb")
                buf.tofile(fp)
                fp.close()
            else:
               Layer6Test.handleClient(None, -1, buf, ret)
            return 0
        
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect( (host, p) )
        
        if not length:
            buf = "Somethign went wrong... nothing to send. Go ahead and crash."
            length = len(buf)

        totalsent = Layer6Test.send_full(sock, buf, length)
        
        print "\nClient: Sent msg %d bytes on %d" % (ret, sock.fileno()) #["+new String(buf,0,len)+"]")
        s_msg, total = Layer6Test.recv_full(sock)
        print "\nClient: Server sez len=%d [%s]" % (total, s_msg)
        sock.close()
        print ("\nClient: exiting ")
        return 0


    @staticmethod
    def send_full(sock, buf, length):
        totalsent = 0
        while totalsent < length:
            sent = sock.send(buf[totalsent:])
            if sent == 0:
                raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent
        return totalsent

    @staticmethod
    def recv_full(sock):
        while 1:
            msg = array.array('c', '\0' * 4096) #[4096]
            total = 0
            nbytes = sock.recv_into(msg)
            if nbytes == 0:
                break
            elif nbytes < 0:
                raise RuntimeError("socket connection broken")
                #break
            #msg = msg + chunk
            total = total + nbytes
            s_msg = msg.tostring().decode('ascii')
            if (s_msg.find("Done!!!") >= 0):
                break
        return s_msg, total

    @staticmethod
    def writefile(fname):
        try:
            Layer6Test.runClient(None, -1, 1, fname)
        except:
            print "Unexpected error:", sys.exc_info()[0]
            traceback.print_exc(file=sys.stdout)
        #ret

    @staticmethod
    def readfile(fname):
        try:
            buf = array.array('c')
            fp = open(fname, "rb")
            l = os.stat(fname).st_size
            print "file = ", fname, " size =", l
            buf.fromfile(fp, l)
            fp.close()
            Layer6Test.handleClient(None, -1, buf, l)
        except:
            print "Unexpected error:", sys.exc_info()[0]
            traceback.print_exc(file=sys.stdout)
        #ret

    @staticmethod
    def run(argv):
        server = None
        Layer6Test.port = 6063
        if len(argv) > 1:
            #server
            Layer6Test.isServer = 1
            Layer6Test.port = int(argv[1])
            SocketServer.ThreadingTCPServer.allow_reuse_address = True
            server = SocketServer.ThreadingTCPServer(('', Layer6Test.port), Layer6Test)
            thr = threading.Thread(target=server.serve_forever)
            thr.setDaemon(True)
            thr.start()            
        else:
            Layer6Test.isServer = 0
    
        if (len(argv) > 2):
            Layer6Test.isServer = 0
            Layer6Test.runClient("localhost", 0, 0)
        elif ((len(argv) > 3) or (len(argv) > 1 and argv[1] == "0")):
            Layer6Test.runClient("localhost", 0, 1)
        else:
            while 1:
                cmd = raw_input("cmd:")
                if (cmd == ''):
                    continue
                if (cmd[0:1] == 'c'):
                    h = "localhost"
                    p = Layer6Test.port
                    tokens = cmd.split(' ')
                    if (len(tokens) > 1):
                        if (len(tokens) > 2):
                            #get host and port
                            h = tokens[1]
                            p = int(tokens[2])
                        else:
                            #get port
                            p = int(tokens[1])
                    if p > 0:
                        print "\nconnecting to host/port: ", h, p
                        Layer6Test.runClient(h, p, 0)
                    else:
                        Layer6Test.runClient(h, Layer6Test.port, 1)
                elif (cmd[0:2] == 'fw'):
                    Layer6Test.writefile(fname = cmd.split(' ')[1])
                elif (cmd[0:2] == 'fr'):
                    Layer6Test.readfile(fname = cmd.split(' ')[1])
                elif(cmd[0] =='q'):
                    break
        return

if __name__ == '__main__':
    Layer6Test.run(sys.argv)