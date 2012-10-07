package org.layer6.test;

import java.net.*;
import java.io.*;
import java.util.*;

import org.layer6.*;

public class Layer6Test implements Runnable {
    boolean isServer;
    Socket socket;
    ServerSocket svr;
    public int port = 6989;

    public Layer6Test() {
    }

    public String dprint(byte [] buffer, int offset, int length) {
        StringBuffer sbuf = new StringBuffer(2*length + 16);
        sbuf.append("\n[");
        for(int i = 0; i < length; i++) {
            sbuf.append(buffer[offset+i]);
            sbuf.append(' ');
            if((i > 0) && (i%32 == 0)) {
                sbuf.append('\n');
            }
        }
        sbuf.append(']');
        return sbuf.toString();
    }
    
    String dump(Layer6Msg msg, String indent) throws Layer6Exception {
        String str = "";
        int i = 0;
        int id;
        int type;
        String name;

        int len = msg.getNumFields();
        for(i = 0; i < len; i++) {
            str += "\n"+indent+i+": ";
            id = msg.getFieldIdAt(i);
            str += ("id=" + id);
            
            name = null;
            if(msg.isFieldNamedAt(i)) {
                name = msg.getFieldNameAt(i);
                str+=(" name='"+name+"'");
            }
            else {
                str+=(" name=''");
            }
            type = msg.getFieldTypeAt(i);
            str +=(" type=" + type + " ");
            switch(type) {
                case Layer6.L6_DATATYPE_STRING:
                    str +=("(string) data=\""
                        + (name != null ? msg.getString(name) :
                            (id >= 0)   ? msg.getString(id)   :
                                          msg.getStringAt(i)));
                    str +=("\"");
                    break;

                case Layer6.L6_DATATYPE_BYTES:
                    byte [] barr = (name != null ? msg.getByteArray(name) :
                                     (id >= 0)   ? msg.getByteArray(id)   :
                                                   msg.getByteArrayAt(i));
                    str +=("(byte ["+barr.length+"]) data=");
                    str += Arrays.toString(barr);
                    str +=("");
                    break;

                case Layer6.L6_DATATYPE_SHORT:
                    str +=("(short) data="
                        + (name != null ? msg.getShort(name) :
                            (id >= 0)   ? msg.getShort(id)   :
                                          msg.getShortAt(i)));
                    break;

                case Layer6.L6_DATATYPE_SHORT_ARRAY:
                    short [] sarr = (name != null ? msg.getShortArray(name) :
                                      (id >= 0)   ? msg.getShortArray(id)   :
                                                    msg.getShortArrayAt(i));
                    str +=("(short ["+sarr.length+"]) data=");
                    str += Arrays.toString(sarr);
                    str +=("");
                    break;

                case Layer6.L6_DATATYPE_INT32:
                    str +=("(int) data="
                        + (name != null ? msg.getInt(name) :
                            (id >= 0)   ? msg.getInt(id)   :
                                          msg.getIntAt(i)));
                    break;

                case Layer6.L6_DATATYPE_INT32_ARRAY:
                    int [] iarr = (name != null ? msg.getIntArray(name) :
                                    (id >= 0)   ? msg.getIntArray(id)   :
                                                  msg.getIntArrayAt(i));                    str +=("(int ["+iarr.length+"]) data=");
                    str += Arrays.toString(iarr);
                    str +=("");
                    break;

                //case Layer6.L6_DATATYPE_INT64:
                case Layer6.L6_DATATYPE_LONG:
                    str +=("(long long int) data="
                        + (name != null ? msg.getLong(name) :
                            (id >= 0)   ? msg.getLong(id)   :
                                          msg.getLongAt(i)));
                    break;

                //case Layer6.L6_DATATYPE_INT64_ARRAY:
                case Layer6.L6_DATATYPE_LONG_ARRAY:
                    long [] larr = (name != null ? msg.getLongArray(name) :
                                     (id >= 0)   ? msg.getLongArray(id)   :
                                                   msg.getLongArrayAt(i));                    str +=("(long long int ["+larr.length+"]) data=");
                    str += Arrays.toString(larr);                    str +=("");
                    break;

                case Layer6.L6_DATATYPE_FLOAT:
                    str +=("(float) data="
                        + (name != null ? msg.getFloat(name) :
                            (id >= 0)   ? msg.getFloat(id)   :
                                          msg.getFloatAt(i)));
                    break;

                case Layer6.L6_DATATYPE_FLOAT_ARRAY:
                    float [] farr = (name != null ? msg.getFloatArray(name) :
                                      (id >= 0)   ? msg.getFloatArray(id)   :
                                                    msg.getFloatArrayAt(i));                    str +=("(float ["+farr.length+"]) data=");
                    str += Arrays.toString(farr);                    str +=("");
                    break;

                case Layer6.L6_DATATYPE_DOUBLE:
                    str +=("(double) data="
                        + (name != null ? msg.getDouble(name) :
                            (id >= 0)   ? msg.getDouble(id)   :
                                          msg.getDoubleAt(i)));
                    break;

                case Layer6.L6_DATATYPE_DOUBLE_ARRAY:
                    double [] darr = (name != null ? msg.getDoubleArrayRef(name) :
                                       (id >= 0)   ? msg.getDoubleArrayRef(id)   :
                                                     msg.getDoubleArrayRefAt(i));                    str +=("(double ["+darr.length+"]) data=");
                    str += Arrays.toString(darr);
                    str +=("");
                    break;
                    
                case Layer6.L6_DATATYPE_L6MSG:
                    Layer6Msg subMsg = (name != null ? msg.getLayer6Msg(name) :
                                         (id >= 0)   ? msg.getLayer6Msg(id)   :
                                                       msg.getLayer6MsgAt(i));
                    str +=("(l6msg ["+subMsg.size()+"]) data={");
                    str+=dump(subMsg, indent+"\t");
                    str +=("}");
                    break;
                
                default:
                    throw new Layer6Exception(Layer6.L6_ERR_UNHANDLED_FIELD_TYPE);
            }
        }
        return str;
    }

    public void handleClient(InputStream ipStream, OutputStream opStream, boolean readOnly) {
        //opStream maybe null
        byte [] buffer = new byte[2048];
        int len = 0;
        String str = "";
        try {
            len = ipStream.read(buffer);
            if(len <= 0) {
                System.out.println("Server: Error no bytes read... ");
                return;
            }
            else {
                System.out.println("Server: Read "+len+" bytes... ");//+ dprint(buffer, 0, len));
            }
            try {    
                try {
                    Layer6Msg msg = new Layer6Msg();
                    
                    //deserialize
                    System.out.println("Server: deserializing ("+len+")... ");
                    msg.deserialize(buffer, 0, len);
                    String indent = "\n";
                    
    
                    len = msg.getNumFields();
                    System.out.println("Server: deserialized, num fields="+len);
                    str = dump(msg, "\t");    //dumpMsg(msg, indent);
                    System.out.println("Server: msg="+str);
                }
                catch(Layer6Exception l6Ex) {
                    //System.out.println("Server: " + l6Ex.getMessage());
                    l6Ex.printStackTrace();
                    System.out.println("...at " + str);
                }

                if(!readOnly){                    opStream.write(str.getBytes());
                    opStream.write("\nDone!!!".getBytes());
                    opStream.flush();
                }
            }
            catch(IOException ioEx) {
                ioEx.printStackTrace();
            }
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    public void runServer() {
        try {
            svr = new ServerSocket(port);
            System.out.println("Server: Running on port "+svr.getLocalPort()+"... ");
            while(true) {
                try {
                    socket = svr.accept();
                    System.out.println("Server: Handling new client... " + socket);
                    handleClient(socket.getInputStream(), socket.getOutputStream(), false);
                    socket.close();
                }
                catch(IOException ioEx) {
                    ioEx.printStackTrace();
                }
                System.out.println("Server: connection closed");
            }
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }
    
    public void handleSend(InputStream ipStream, OutputStream opStream, boolean writeOnly) {
        //ipStream may be null
        byte [] buffer = new byte[2048];
        int len = 0;
        int i = 0;
        int id;
        int type;
        String str;

        double [] darr = {1.2, 3.45, 5.67, 7.89, 0.91};
        int [] iarr = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        boolean addSubSubMsg = true;

        try {
            Layer6Msg msg = new Layer6Msg();
            try {
                msg.setShort(1, (short)43);
                msg.setInt(2, -23)
                    .setLong("l",3847)
                    .setFloat("float",(float)39.31)
                    .addDouble(1278.409823)
                    .setDoubleArray("da",darr)
                    .setIntArrayRef(4,iarr)
                    .setStringRef(5,"Yeeehaaaah!");

                //overwrite float
                msg.setFloatAt(3, (float)49.312);
            
                Layer6Msg subMsg = new Layer6Msg();
                //subMsg.setInt(0, 3456);
                //subMsg.setString("subkey", "qwerty");
                subMsg.setString("sub-f1", "sub msg field 1");
                subMsg.setStringRef(5,"sub msg field 2!");
                subMsg.setStringRef("temp", "I'm just gonna get deleted in a few lines...");
                msg.setLayer6MsgRef("sub", subMsg);
                
                int [] tempintarr = {1,2,3,5,7,11,13};
                subMsg.setIntArrayRef(3, tempintarr);

                
                if(addSubSubMsg) {
                    Layer6Msg subSubMsg = new Layer6Msg();
                    subSubMsg.setShortArray("short-array", new short[] {1,3,4,5})
                             .setString("sub-sub-key1", "qwerty")
                             .setString("yowzah", "Lorem ipsum dolor sit amet.");
                    subMsg.setLayer6Msg("subsub", subSubMsg);
                    
                    Layer6Msg subSubMsgTemp = new Layer6Msg();
                    subSubMsgTemp.setByteArray("garbage", new byte[16]);
                    
                    subSubMsg.setLayer6Msg("subsubTmp", subSubMsgTemp);
                    subMsg.setInt(32, 32);
                    
                    System.out.println("\tClient: Msg nfields = " + msg.getNumFields() +", serializing exp=["+msg.size()+"] bytes, buf=["+buffer.length+"] bytes...");
                    //System.out.println("\t\t subMsg.size="+subMsg.size()+", subSubMsg.size="+subSubMsg.size());
                    
                    //removing
                    subSubMsg.removeField("subsubTmp");
                    subMsg.removeField(32);
                    
                    msg.setLayer6Msg(99, subSubMsg);
                }
                subMsg.removeField("temp");
                
                //get and re-add a submsg if it's there
                Layer6Msg subSubMsg = msg.getLayer6MsgRef(99);
                
                if(subSubMsg != null) {  //should always get here unless we disable exceptions (which is not currently possible)
                    //add a new field
                    long [] larr = {112, 35, 81321, 34, 5589, 144};
                    subSubMsg.addLongArrayRef(larr);
                    
                    //remove an old field
                    subSubMsg.removeField("yowzah");
                }
                
                msg.addByteArray(new byte[] {9, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0});
                
                System.out.println("\tClient: Msg nfields = " + msg.getNumFields() +", serializing exp=["+msg.size()+"] bytes, buf=["+buffer.length+"] bytes...");

                len = msg.serialize(buffer, 0, buffer.length);
                System.out.println("\tExp= " + msg.size() +", actual="+len+" bytes...");
            }
            catch(Layer6Exception l6Ex) {
                //System.out.println("Client: " + l6Ex.getMessage());
                //l6Ex.printStackTrace();
                throw l6Ex;
            }

            System.out.println("\n\tClient: Msg bytes expected="+msg.getSizeInBytes()
                + " actual="+len);// + ":" +dprint(buffer, 0, len));
            opStream.write(buffer, 0, len);
            opStream.flush();
            System.out.println("\tClient: Wrote msg ... ");//["+new String(buffer,0,len)+"]");

            if(!writeOnly) {
                while(true) {
                    len = ipStream.read(buffer);
                    if(len <= 0) {
                        break;
                    }
                    str = new String(buffer, 0, len);
                    System.out.println("\tClient: Server sez - " + str);
                    if(str.indexOf("Done!!!") > 0) {
                        break;
                    }
                }
            }
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    public void runClient(String host, int p) {
        System.out.println("\tClient: starting up ");
        try {
            socket = new Socket(host, p);
            handleSend(socket.getInputStream(), socket.getOutputStream(), false);
            socket.close();
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
        System.out.println("\tClient: exiting ");
    }

    public void run() {
        if(isServer) {
            runServer();
        }
        else {
            runClient("localhost", port);
        }
    }

    public static void main(String [] args) {
        Layer6Test test = new Layer6Test();
        if(args.length > 0) {
            //server
            Layer6Test test2 = new Layer6Test();
            test2.isServer = true;
            try {
                test.port = test2.port = Integer.decode(args[0]);
            }
            catch(NumberFormatException nfEx) {
                test.port = test2.port = 6989;
            }
            Thread thr = new Thread(test2);
            thr.start();
        }
        else {
            test.isServer = false;
        }
        if(args.length > 1) {
            try {
                //test2.wait();
                Thread.sleep(1000);
                test.isServer = false;
                test.run();    //start client
            }
            catch(Exception ex) {
                ex.printStackTrace();
            }
        }
        else {
            //console 
            java.util.Scanner scanner = new java.util.Scanner(new InputStreamReader(System.in));
            //IRendezvousClient consolePeer = peer;
            while(true) {
                try {
                    System.out.print("\ncmd: ");
                    String cmd = scanner.nextLine().trim();
                    String [] tokens = cmd.split("\\s");
                    String host = "localhost";
                    int p = test.port;
                    if("c".equalsIgnoreCase(tokens[0])) {
                        test.isServer = false;
                        if(tokens.length > 1) {
                            if(tokens.length > 2) {
                                host = tokens[1];
                                p = Integer.decode(tokens[2]);
                            }
                            else {
                                p = Integer.decode(tokens[1]);
                            }
                        }
                        test.runClient(host, p);
                    }
                    else if("fw".equals(tokens[0]))  {
                        if(tokens.length > 1) {
                            String fname = tokens[1];
                            System.out.println("\nWriting to file " +fname+":");
                            try {
                                test.handleSend(null, new FileOutputStream(new File(fname)), true);
                            }
                            catch(IOException ioEx) {
                                ioEx.printStackTrace();
                            }

                        }
                        else {
                            System.out.println("Output filename not specified.");
                        }
                    }
                    else if("fr".equals(tokens[0]))  {
                        if(tokens.length > 1) {
                            String fname = tokens[1];
                            System.out.println("\nReading from file " +fname+":");
                            try {
                                test.handleClient(new FileInputStream(new File(fname)), null, true);
                            }
                            catch(IOException ioEx) {
                                ioEx.printStackTrace();
                            }
                        }
                        else {
                            System.out.println("Input filename not specified.");
                        }                        
                    }
                    else if("q".equalsIgnoreCase(tokens[0])) {
                        System.exit(0);
                    }
                }
                catch(Exception ex) {
                    ex.printStackTrace();
                }
            }
        }
    }
}
