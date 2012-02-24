#if defined _WIN32 || defined _WIN64
//Windows
#include <winsock2.h>
#include <process.h>
//constants
#include <ws2tcpip.h.>
#else
// *NIX
//sockets
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <sysexits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define SOCKET int
#endif

//std
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include <sys/stat.h>

//threads
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>

#include <iostream>

#include "layer6.hpp"

/** START OVERRIDE METHODS ***/

#define __USE_STL_ 0

#if __USE_STL_

extern "C" {

#define mvec_t       std::vector<void*>*

int my_mvec_init(void **v, int unit_size, int start) {
    //std::cout<<"MVEC INIT!!!"<<std::endl;
    //do nothing
    mvec_t *q = (mvec_t *)v;
    (*q) = new std::vector<void*>;
    return 0;
}

int my_mvec_free(void *v) {
    mvec_t q = (mvec_t)v;
    q->clear();
    delete q;
    return 0;
}

int my_mvec_append(void *v, void *field) {
    mvec_t q = (mvec_t)v;
    q->push_back(field);
    return 0;
}

int my_mvec_size(void *v) {
    mvec_t q = (mvec_t)v;
    return q->size();
}

int my_mvec_resize(void *v, float factor) {
    mvec_t q = (mvec_t)v;
    //vector, don't need resizing
    return q->size();
}

int my_mvec_index_of(void *v, void *field) {
    mvec_t q = (mvec_t)v;
    for(unsigned int i = 0; i < q->size(); i++) {
        if(field == q->at(i))
            return i;
    }
    return -1;
}

int my_mvec_remove_item(void *v, void *field) {
    mvec_t q = (mvec_t)v;
    std::vector<void*>::iterator itr = q->begin();
    while(itr != q->end()) {
        if(field == *itr) {
            q->erase(itr);
            break;
        }
        itr++;
    }
    return 0;
}

void* my_mvec_get(void *v, int itr) {
    mvec_t q = (mvec_t)v;
    void *field = NULL;
    if(itr < (int)q->size()) {
        field = q->at(itr);
    }
    return field;
}

void* my_mvec_remove(void *v, int index) {
    mvec_t q = (mvec_t)v;
    void *field = NULL;
    if(index < (int)q->size()) {
        std::vector<void*>::iterator itr = q->begin() + index;
        field = *itr;
        q->erase(itr);
    }
    return field;
}

void* my_mvec_remove(void *v) {
    mvec_t q = (mvec_t)v;
    void *field = NULL;
    field = q->back();
    q->pop_back();
    return field;
}

class my_htbl_base {
public:
    my_htbl_base(int t, int c) {
        type = t;
        cap  = c;
    }
    
    virtual ~my_htbl_base() {}
}

template <class K, class C = std::less>
class my_htbl : my_htbl_base {
    std::map<K, void*, C> hmap;

public:
    int type;
    int cap;

    my_htbl(int t, int c) : my_htbl_base(t, c) { }
    
    virtual ~my_htbl() { hmap.clear(); }

    int put(K key, void *field) {
        hmap[key] = field;
        return 0;
    }
    
    int get(K key, void **field) {
        std::map<K, void*, C>::iterator itr = hmap.find(key);
        if(itr != hmap.end()) {
            *field = itr->second;
            return 1;
        }
        return 0;
    }
    
    int remove(K key, void **field) {
        std::map<K, void*, C>::iterator itr = hmap.find((short)key);
        if(itr != hmap.end())  {
            *field = itr->second;
            map.erase(itr);
            return 0;
        }
        return 1;
    }

};

struct strlt {
    bool operator()(const char* s1, const char* s2) const {
        return strcmp(s1, s2) < 0;
    }
};

#define my_htbl_i my_htbl<short>
#define my_htbl_s my_htbl<const char*, strlt>

int my_htbl_init(void *vpmap, int type, int cap) {
    htbl_t **ppmap = (htbl_t**) vpmap;
    if(type == L6_MSG_ID_MAP_TYPE_STR) {
        (*ppmap) = new my_htbl_s(type, cap);
    else 
        (*ppmap) = new my_htbl_i(type, cap);
    return 0;
}

int my_htbl_free(void *pmap) {
    void **ppmap = (void**)pmap;
    my_htbl_base *htbl = (my_htbl_base*)*ppmap;
    delete htbl;
    return 0;
}

int my_htbl_str_put(void *htbl, const char *key, void *field) {
    return (*(htbl_t_s*)htbl)->put(key, field);
}

int my_htbl_put(void *htbl, int32_t key, void *field) { 
    return (*(htbl_t_i*)htbl)->put(key, field);
}

int my_htbl_str_get(void *htbl, const char *key, void **field) {
    return (*(htbl_t_s*)htbl)->get(key, field);
}

int my_htbl_get(void *htbl, int32_t key, void **field) {
    return (*(htbl_t_i*)htbl)->get(key, field);
}

int my_htbl_str_remove(void *htbl, const char *key, void** field) {
    return (*(htbl_t_s*)htbl)->remove(key, field);
}

int my_htbl_remove(void *htbl, int32_t key, void** field) { 
    return (*(htbl_t_i*)htbl)->remove(key, field);
}
}

/** END OVERRIDE METHODS ***/

#endif



int isServer;
int sock;
int svr;

int open_tcp_sock(int port) 
{
	struct sockaddr_in local;
	int sd;
	int len;
	sd = socket(AF_INET, SOCK_STREAM, 0) ;
	if(sd < 0) 
	{
		return sd;
	}

	//allow socket re-use	
	len = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &len, sizeof(len)) < 0) 
	{
		perror("setsockopt(SO_REUSEADDR)");
		return -1;
	}


	local.sin_family = AF_INET ;
	local.sin_port = htons (port) ;
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sd ,(struct sockaddr *)&local , sizeof(local))<0) 
	{
		perror("bind");
		return -1;
	}
	return sd;
}

int connect_sock(int sd, const char* remote, int port) 
{
	struct sockaddr_in remote_addr;
	struct hostent *hp;
	remote_addr.sin_family = AF_INET ;
	remote_addr.sin_port = htons( port ) ;
	if(remote == NULL) 
	{
		remote_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		printf("\ninaddr_any\n");
		return connect( sd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
	}
	if(inet_aton(remote, &remote_addr.sin_addr)) 
	{
		/* connect to server */
		//printf("\ninet_aton\n");
		//log("1: connecting to  %s: %d",remote, port);
		return connect( sd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
	}
	else 
	{
		hp = gethostbyname(remote);
		//std::cout<<"\ngethostbyname "<<hp->h_name<<":"<<port<<"\n"<<std::flush;
		//log("2: connecting to  %s:%d -- %x",remote, port, hp);
		if(!hp)
			return -1;
		remote_addr.sin_family = AF_INET ;
		remote_addr.sin_port = htons( port ) ;
		remote_addr.sin_addr = *(struct in_addr*)hp->h_addr;

		/* connect to server */
		return connect( sd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
	}
	return -1;
}


void dprint(char *buffer, int offset, int length) 
{
	int i = 0;
	std::cout<<"\n[";
	for(i = 0; i < length; i++) 
	{
		std::cout<<((unsigned int)buffer[offset+i])<<" ";
		if((i > 0) && (i%32 == 0)) 
		{
			std::cout<<"\n";
		}
	}
	std::cout<<"]"<<std::flush;
	return;
}

int dump(char *opstr, Layer6Msg &msg, const char *indent)
{
	int offset = 0;
	int type;
	int ret = 0;
	int len = msg.getNumFields();
	try 
	{
        msg.setExceptionsEnabled(true);

		for(int i = 0; i < len; i++) 
		{
            int id = -1;
			//offset = 0;
			offset += sprintf(opstr + offset,"\n%s%2d:", indent, i);
			ret = msg.getFieldIdAt(i, &id);
            //ignore any "field ID not assigned" errors
			offset += sprintf(opstr + offset," id=%2d", id);
			char *name = NULL;
			if(msg.isFieldNamedAt(i)) 
			{
                name = const_cast<char*>(msg.getFieldNameAt(i));
				offset += sprintf(opstr + offset," name='%s'", name);
			}
			else 
                offset += sprintf(opstr + offset," name=''");
			type = msg.getFieldTypeAt(i);
			offset += sprintf(opstr + offset," type=%d ", type);
			
			int count = 1;
			if(name ? msg.isFieldArray(name) :
                ((id >= 0) ? msg.isFieldArray(id) :
                             msg.isFieldArrayAt(i))) {
                count = (name ? msg.getFieldCount(name) :
                            ((id >= 0) ? msg.getFieldCount(id) :
                                         msg.getFieldCountAt(i)));
            }
			//printf("\nid=%d type=%d ", id, type);
			switch(type) 
			{
				case L6_DATATYPE_STRING: 
				{
					offset += sprintf(opstr + offset,"(string) data=%s", 
					           (name ? msg.getStringPtr(name) :
                                       ((id >= 0) ? msg.getStringPtr(id) :
                                                    msg.getStringPtrAt(i))) );
					break;
				}
	
				case L6_DATATYPE_BYTES: 
				{
					offset += sprintf(opstr + offset,"(byte [%d]) data=[", count);
					const char* const data = (name ? msg.getByteArrayPtr(name, &count) :
                                               ((id >= 0) ? msg.getByteArrayPtr(id, &count) :
                                                            msg.getByteArrayPtrAt(i, &count)));                    int j = 0;
                    for(j = 0; j < count; j++) 
                        offset += sprintf(opstr + offset, "%2X ", data[j]);
                    offset += sprintf(opstr + offset - (count > 0 ? 1 : 0), "]  ") - 2;
					//dprint(data, 0, count);
					break;
				}
	
				case L6_DATATYPE_SHORT: 
				{
					short data = (name ? msg.getShort(name) :
                                  ((id >= 0) ? msg.getShort(id) :
                                               msg.getShortAt(i)));
					offset += sprintf(opstr + offset,"(short) data=%d",data);
					break;
				}
	
				case L6_DATATYPE_SHORT_ARRAY: 
				{
					offset += sprintf(opstr + offset,"(short [%d]) data=[", count);
					short *data = new short[count];
                    ret = (name ? msg.getShortArray(name, data, 0, count) :
                                  ((id >= 0) ? msg.getShortArray(id, data, 0, count) :
                                               msg.getShortArrayAt(i, data, 0, count)));
					int j = 0;
					for(j = 0; j < count; j++) 
					{
						if(j < count - 1) 
						{
							offset += sprintf(opstr + offset,"%d, ",data[j]);
						}
						else 
						{
							offset += sprintf(opstr + offset,"%d", data[j]);
						}
					}
					offset += sprintf(opstr + offset,"]");
					delete [] data;
					break;
				}
	
				case L6_DATATYPE_INT32: 
				{
					int data = (name ? msg.getInt(name) :
                                  ((id >= 0) ? msg.getInt(id) :
                                               msg.getIntAt(i)));
					offset += sprintf(opstr + offset,"(int) data=%d",data);
					break;
				}
	
				case L6_DATATYPE_INT32_ARRAY: 
				{
					ret = (name ? msg.getFieldCount(name, &count) :    //repeat for testing
                                  ((id >= 0) ? msg.getFieldCount(id, &count) :
                                               msg.getFieldCountAt(i, &count)));
					offset += sprintf(opstr + offset,"(int [%d]) data=[", count);
					int *data = new int [count];
                    ret = (name ? msg.getIntArray(name, data, 0, count) :
                                  ((id >= 0) ? msg.getIntArray(id, data, 0, count) :
                                               msg.getIntArrayAt(i, data, 0, count)));
					if(ret != 0) 
					{
						printf("%s", msg.getErrorStr());
						//continue;
					}
					int j = 0;
					for(j = 0; j < count; j++) 
					{
						if(j < count - 1) 
						{
							offset += sprintf(opstr + offset,"%d, ",data[j]);
						}
						else 
						{
							offset += sprintf(opstr + offset,"%d", data[j]);
						}
					}
					offset += sprintf(opstr + offset,"]");
					delete [] data;
					break;
				}
	
				case L6_DATATYPE_INT64:
				//case L6_DATATYPE_LONG: 
				{
					long long int data = (name ? msg.getLong(name) :
                                                ((id >= 0) ? msg.getLong(id) :
                                                            msg.getLongAt(i)));
					offset += sprintf(opstr + offset,"(long long int) data=%lld", data);
					break;
				}
	
				case L6_DATATYPE_INT64_ARRAY:
				//case L6_DATATYPE_LONG_ARRAY: 
				{
					offset += sprintf(opstr + offset,"(long long int [%d]) data=[", count);
					long long int *data = new long long int[count];
                    ret = (name ? msg.getLongArray(name, data, 0, count) :
                                  ((id >= 0) ? msg.getLongArray(id, data, 0, count) :
                                               msg.getLongArrayAt(i, data, 0, count)));
					int j = 0;
					for(j = 0; j < count; j++) 
					{
						if(j < count - 1) 
						{
							offset += sprintf(opstr + offset,"%lld, ",data[j]);
						}
						else 
						{
							offset += sprintf(opstr + offset,"%lld", data[j]);
						}
					}
					offset += sprintf(opstr + offset,"]");
					delete [] data;
					break;
				}
	
				case L6_DATATYPE_FLOAT: 
				{
					float data = (name ? msg.getFloat(name) :
                                      ((id >= 0) ? msg.getFloat(id) :
                                                   msg.getFloatAt(i)));
					offset += sprintf(opstr + offset,"(float) data=%0.4f",data);
					break;
				}
	
				case L6_DATATYPE_FLOAT_ARRAY: 
				{
					offset += sprintf(opstr + offset,"(float [%d]) data=[", count);
					float *data = new float[count];
                    ret = (name ? msg.getFloatArray(name, data, 0, count) :
                                  ((id >= 0) ? msg.getFloatArray(id, data, 0, count) :
                                               msg.getFloatArrayAt(i, data, 0, count)));
					int j = 0;
					for(j = 0; j < count; j++) 
					{
						if(j < count - 1) 
						{
							offset += sprintf(opstr + offset,"%0.4f, ",data[j]);
						}
						else 
						{
							offset += sprintf(opstr + offset,"%0.4f", data[j]);
						}
					}
					offset += sprintf(opstr + offset,"]");
					delete [] data;
					break;
				}
	
				case L6_DATATYPE_DOUBLE: 
				{
					double data = (name ? msg.getDouble(name) :
                                      ((id >= 0) ? msg.getDouble(id) :
                                                   msg.getDoubleAt(i)));
					offset += sprintf(opstr + offset,"(double) data=%0.4f", data);
					break;
				}
	
				case L6_DATATYPE_DOUBLE_ARRAY: 
				{
					offset += sprintf(opstr + offset,"(double [%d]) data=[", count);
					const double* const data = (name ? msg.getDoubleArrayPtr(name, &count) :
                                                  ((id >= 0) ? msg.getDoubleArrayPtr(id, &count) :
                                                               msg.getDoubleArrayPtrAt(i, &count)));
					//double *data = (double*)malloc(sizeof(double)*count);
					//ret = msg.getDoubleArray(id, data, 0, count);
					int j = 0;
					for(j = 0; j < count; j++) 
					{
						if(j < count - 1) 
						{
							offset += sprintf(opstr + offset,"%0.4f, ",data[j]);
						}
						else 
						{
							offset += sprintf(opstr + offset,"%0.4f", data[j]);
						}
					}
					offset += sprintf(opstr + offset,"]");
					break;
				}
				
				case L6_DATATYPE_L6MSG: 
				{
					Layer6Msg *subMsg = (name ? msg.getLayer6MsgPtr(name) :
                                          ((id >= 0) ? msg.getLayer6MsgPtr(id) :
                                                       msg.getLayer6MsgPtrAt(i)));
					//Layer6Msg **psubMsg = &subMsg;
					char indentbuf[32];
					sprintf(indentbuf, "%s\t", indent);
                    offset += sprintf(opstr + offset,"(l6msg [%d]) data= {", count);
					offset += dump(opstr + offset, (*subMsg), indentbuf);
                    offset += sprintf(opstr + offset,"}");
					break;
				}
			}
			//offset += sprintf(opstr + offset,
		}
	}
	catch(int ex) 
	{
		std::cout<<"Exception! ["<<msg.getDebugInfo()<<"]"<<std::endl;
	}
	return offset;
}


int deserialize(const char *who, char *buffer, int len, char *opstr) 
{
	int left = 0;
	//deserialize
	std::cout<<"\n"<<who<<": deserializing.... "<<std::flush;
	Layer6Msg msg;

	int ret = msg.deserialize(buffer, len, &left);
	if(ret < 0) 
	{
		std::cout<<"\n"<<who<<": "<<msg.getErrorStr()<<std::flush;
		return -1;
	}
	std::cout<<"\n"<<who<<": deserialized "<<len<<" bytes left="<<left<<", num fields="<<msg.getNumFields()<<std::flush;
	ret = dump(opstr, msg, "\t");
	///std::cout<<"\nServer: msg dump="<<opstr<<std::flush;
	return ret;
}


int handleClient(int sd, char *buffer, int len) 
{
	int offset = 0;
	char *opstr = new char[2048];    try {
        if(buffer != NULL) {
        	offset = deserialize("Server", buffer, len, opstr);
            	std::cout<<"\nServer: deserialized "<<offset<<" bytes .... "<<std::flush;   
            std::cout<<"\nServer: "<<opstr<<std::endl;
            return 0;
        }
    
    	buffer = new char[2048];
    	len = 0;
    
    	std::cout<<"\nServer: Handling new client... "<<sd<<std::flush;
    	len = recv(sd, buffer, 2048, 0);
    
    	if(len < 0) 
    	{
    		std::cout<<"\nServer: Error ["<<len<<"] bytes read.... "<<std::flush;
    		//close(sd);
    		return -1;
    	}
    	else 
    	{
    		std::cout<<"\nServer: "<<len<<" bytes read.... "<<std::flush;//+ dprint(buffer, 0, len));
    	}
    
    	offset = deserialize("Server", buffer, len, opstr);
    
    	len = send(sd, opstr, offset, 0);
    	offset = sprintf(opstr, "\nDone!!!");
    	len = send(sd, opstr, offset, 0);
    }
    catch(int &ex) {
       std::cout<<"Server: Exception "<<ex<<" - "<<Layer6Msg::getErrorStrForCode(ex)<<std::endl;
    }
    close(sd);

	std::cout<<"\nServer: connection closed"<<std::flush;
	delete [] buffer;
	delete [] opstr;
	return 0;
}

int runServer(int port) 
{
	struct sockaddr_in remote;
	int	addr_len;
	if(port <= 0) 
	{
		port = 6989;
	}

	svr = open_tcp_sock(port);

	int ret = listen(svr, 10);
	if(ret < 0) 
	{
		std::cout<<"\nServer: Unable to listen: "<<ret<<std::flush;
		perror("listen");
		return -1;
	}
	std::cout<<"\nServer: Running... "<<std::flush;

	while(1) 
	{
		int newconn = accept(svr, (struct sockaddr*)&remote, (socklen_t*)&addr_len);
		remote.sin_addr.s_addr = ntohl(remote.sin_addr.s_addr);
		std::cout<<"\nServer: Accepted connection: "<<newconn<<" from "<<inet_ntoa(remote.sin_addr)<<":"<<remote.sin_port<<std::flush;
		handleClient(newconn, NULL, 0);
	}
	return 0;
}

int runClient(const char *host, int port, int altfd) 
{
	char *buffer = new char[4096];
	int len = 0;
	int ret = 0;
	int left = 0;
	if(port <= 0) 
	{
		port = 6989;
	}
	bool addSubSubMsg = true;

	double darr[] = {1.2, 3.45, 5.67, 7.89, 0.91};
	int iarr[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
	std::cout<<"\n\tClient: starting up "<<std::flush;

	Layer6Msg *msg = new Layer6Msg();
	Layer6Msg *subMsg = NULL;
		
	try 
	{
		//set data
		//std::cout<<"short, "<<std::flush;
		msg->setShort(1, (short)43);
	
		//std::cout<<"int, "<<std::flush;
		msg->setInt(2, -23);
	

        //Uncomment this block to test error handling
        /*
        short t[5];
        ret = msg->getShortArray(2, t, 0, 5);
        */

		//std::cout<<"long, "<<std::flush;
		msg->setLong("l", 3847);
	
		//std::cout<<"float, "<<std::flush;
		msg->setFloat("float", 39.31f);
	
		//std::cout<<"double, "<<std::flush;
		msg->addDouble(1278.409823);
		
		//std::cout<<"double array, "<<std::flush;
		msg->setDoubleArray("da", darr, 5);
	
		//std::cout<<"int array, "<<std::flush;
		std::vector<int> ivec( iarr, iarr + (sizeof(iarr) / sizeof(iarr[0])) );
		msg->setIntVector(4, ivec);
		//msg->setIntArrayPtr(4, iarr, 10);
	
		//std::cout<<"string, "<<std::flush;
		msg->setString(5, "Yeeehaaaah!");
		
		//std::cout<<"submsg... "<<std::flush;
		subMsg = new Layer6Msg();
		subMsg->setExceptionsEnabled(true);
		subMsg->setString("sub-f1", const_cast<char*>("sub msg field 1"));
		subMsg->setStringPtr(5, "sub msg field 2!");		
		subMsg->setStringPtr("temp", const_cast<char*>("I'm just gonna get deleted in a few lines..."));
		
		msg->setLayer6MsgPtr("sub", subMsg);

		std::cout<<"\nClient: Added subMsg w/ nfields ="<<subMsg->getNumFields()<<", size="<<subMsg->size()<<"..."<<std::flush;
		std::cout<<"to msg, now having nfields ="<<msg->getNumFields()<<", size="<<msg->size()<<"..."<<std::flush;
		int tempintarr[] = {1,2,3,5,7,11,13};
		subMsg->setIntArrayPtr(3, tempintarr, sizeof(tempintarr)/sizeof(int));
		
		if(addSubSubMsg) {
            Layer6Msg subSubMsg;
            short * sarr= new short[4]; sarr[0] = 1; sarr[1] = 3; sarr[2] = 4; sarr[3] = 5;
            subSubMsg.setShortArray("short-array", sarr, 4);
            delete [] sarr;
            subSubMsg.setString("sub-sub-key1", "qwerty");
            subSubMsg.setStringPtr("yowzah", "Lorem ipsum dolor sit amet.");
            subMsg->setLayer6Msg("subsub", subSubMsg);
            
            Layer6Msg subSubMsgTemp;
            char *bytes = new char[16];
            subSubMsgTemp.setByteArray("garbage", bytes, 16);
            delete [] bytes;
            
            subSubMsg.setLayer6Msg("subsubTmp", subSubMsgTemp);
            subMsg->setInt(32, 32);
            
            std::cout<<"\n\tClient: Msg nfields = "<<msg->getNumFields()<<", serializing exp=["<<msg->size()<<"] bytes, buf=["<<sizeof(buffer)+"] bytes..."<<std::flush;
            //std::cout<<"\n\t\t subMsg.size="<<subMsg.size()<<", subSubMsg.size="<<subSubMsg.size();
        
            //removing
            subSubMsg.removeField("subsubTmp");
            subMsg->removeField(32);

            //set another copy
            msg->setLayer6Msg(99, subSubMsg);
		}
	
		std::cout<<"\nClient: Added field to subMsg w/ nfields ="<<subMsg->getNumFields()<<", size="<<subMsg->size()<<"..."<<std::flush;
		std::cout<<"in msg, now having nfields ="<<msg->getNumFields()<<", size="<<msg->size()<<"..."<<std::flush;
	
		ret = subMsg->removeField("temp");
        if(ret < 0) {
            std::cout<<"\nError removing field ("<<__LINE__<<" in "<< __FILE__<<") - "<< subMsg->getDebugInfo()<<std::endl;
    		delete [] buffer;
    		delete msg;
    		if(subMsg)
    		  delete subMsg;
            return -1;
        }

		std::cout<<"\nClient: Removed field from subMsg w/ nfields ="<<subMsg->getNumFields()<<", size="<<subMsg->size()<<"..."<<std::flush;
		std::cout<<"in msg, now having nfields ="<<msg->getNumFields()<<", size="<<msg->size()<<"..."<<std::flush;
 
       //get and re-add a submsg if it's there
        msg->setExceptionsEnabled(false);
        Layer6Msg *sub_submsg = msg->getLayer6MsgPtr(99);
        if(!msg->getErrorCode()) {
            //add a new field
            long long larr[] = {112, 35, 81321, 34, 5589, 144}; 
            sub_submsg->addLongArrayPtr(larr, sizeof(larr)/sizeof(unsigned long long));
            
            //remove an old field
            sub_submsg->removeField("yowzah");
        }
        else
        {
            std::cout<<"\nClient: "<<msg->getDebugInfo()<<" - Could not get submsg field at id 99 in msg w/ nfields = "
                    <<msg->getNumFields()<<", size="<<msg->size()<<"..."<<std::endl;
        }
        msg->setExceptionsEnabled(true);

#if __STDC_VERSION__ >= 199901L
        msg->addByteArray((char[]){9, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0}, 12);
#else
        msg->addByteArray((char*)iarr, 12);
#endif

		std::cout<<"\n\tClient: Msg nfields = "<<msg->getNumFields()<<", serializing..."<<std::flush;
	
		len = 0;
		do
		{
			len = msg->serialize(buffer+len, 64, &left);
			if(len < 0) 
			{
				std::cout<<msg->getErrorStr()<<std::flush;
				return -1;
			}
			else
			{
				std::cout<<" "<<left<<", "<<std::flush;
			}
		}
		while(left);
	}
	catch(int &ex) 
	{
		std::cout<<"\nClient: Exception="<<ex<<" - "<<msg->getDebugInfo()<<" "<<std::endl;// + ":" +dprint(buffer, 0, len));
		delete [] buffer;
		delete msg;
		if(subMsg)
		  delete subMsg;
		return -1;
	}
	//dprint(buffer, 0, len);
	std::cout<<"\nClient: total serialized="<<len<<" bytes"<<std::flush;// + ":" +dprint(buffer, 0, len));

	ret = msg->getSize();

	std::cout<<"\nClient: Msg bytes expected="<<ret<<" actual="<<len<<std::flush;// + ":" +dprint(buffer, 0, len));
	
	if(altfd) {
        if(altfd < 0)
        	ret = handleClient(-1, buffer, ret);
        else
            ret = write(altfd, buffer, ret);

    	delete [] buffer;
    	delete msg;
    	if(subMsg)
    		delete subMsg;
        std::cout<<"\nClient: exiting "<<std::flush;
    	return ret;
	}

	sock = open_tcp_sock(0);
	if(sock < 0) 
	{
		perror("\n\tClient: could not open socket - ");
		return -1;
	}

	ret = connect_sock(sock, "localhost", port);
	if(ret < 0) 
	{
		perror("\nClient: could not connect - ");
		char opstr[4096];
		int offset = 0;
		std::cout<<"\nTrying locally---\n"<<std::flush;
		offset = deserialize("Client", buffer, 4096, opstr);
		opstr[offset] = '\0';
		std::cout<<"\nClient: "<<opstr<<std::flush;
	}
	else 
	{
		ret = send(sock, buffer, len, 0);
		if(ret < 0) 
		{
			std::cout<<"\nClient: Error in sending bytes.... "<<std::flush; 
			perror("\nsend"); fflush(stdout);
			//close(sock);
			return -1;
		}
	
		std::cout<<"\n\tClient: Sent msg ... "<<ret<<std::flush;//["+new String(buffer,0,len)+"]");
	
		while(1) 
		{
			len = recv(sock, buffer, 4096, 0);
			if(len < 0) 
			{
				std::cout<<"\nClient: Error in reading bytes.... "<<std::flush;
				//close(sock);
				return -1;
			}
			buffer[len] = '\0';
			std::cout<<"\n\tClient: Server sez len="<<len<<" {"<<buffer<<"\n}"<<std::flush;
			if(strstr(buffer, "Done!!!") == NULL) 
			{
				break;
			}
		}
		//close(sock);
	}
	std::cout<<"\nClient: exiting "<<std::flush;
	
	delete [] buffer;
	delete msg;
	if(subMsg)
		delete subMsg;
	return 0;
}

void* run(void* isSvr) 
{
	if(isSvr) 
	{
		int port = (*(int*)isSvr);
		//std::cout<<"\n\t args=%d... ", port); fflush(stdout);
		runServer(port);
		
	}
	else 
	{
        try{
            runClient("localhost", 0, 0);
	   }
	   catch(int &ex) {
           std::cout<<"Server: Exception "<<ex<<" - "<<Layer6Msg::getErrorStrForCode(ex)<<std::endl;
	   }
	}
	return 0;
}

int main(int argc, char* argv[]) 
{
#if defined _WIN32 || defined _WIN64
    WSAData wsaData;
	//init winsock 
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0) 
	{
        printf("Failed to find Winsock 2.1 or better.\n");
        return -1;
    }
#endif

	isServer = 0;
	//std::cout<<"\n sizeof int=%d long int=%d long long int=%d", sizeof(int), sizeof(long int), sizeof(long long int));
	//std::cout<<"\n htons(-23)=%d", htons(-23));
	pthread_t thr;
	int port = 6989;
	
#if __USE_STL_
    std::cout<<"Overriding default internal data structs with STL-based ones..."<<std::endl;
    l6_override_hashtable_methods(  my_htbl_init,  my_htbl_free,  my_htbl_put, my_htbl_get, 
                        my_htbl_remove,  my_htbl_str_put, my_htbl_str_get, my_htbl_str_remove);

    l6_override_vector_methods( my_mvec_init, my_mvec_free, my_mvec_resize, my_mvec_append, my_mvec_size,
                                my_mvec_get, my_mvec_remove, my_mvec_index_of, my_mvec_remove_item, my_mvec_remove);
#endif

	if(argc > 1) 
	{
		//server
		isServer = atoi(argv[1]);
		port = isServer;
		
		//start server
		pthread_create(&thr, NULL, &run, &isServer);
	}
	else 
	{
		isServer = 0;
	}
	if(argc > 2) 
	{
		//sleep(1);
		isServer = 0;
		run(0);	//start client
		pthread_join(thr, NULL);
	}
	else if((argc > 3) || !strcmp(argv[1], "0")) {
	   try {
	       runClient("localhost", 0, -1);
	   }
	   catch(int &ex) {
           std::cout<<"Server: Exception "<<ex<<" - "<<Layer6Msg::getErrorStrForCode(ex)<<std::endl;
	   }
	}
	else 
	{
		while(1) 
		{
		    const char *host = "localhost";
			std::cout<<"\ncmd:"<<std::flush;
			char cmd[32];
			fgets(cmd, sizeof(cmd), stdin);
			if(cmd[0] == 'c') 
			{
                int p = port;
                if(strlen(cmd) > 2) 
                {
                    char  *pstr  = NULL;
                    char  *token1 = strtok_r(cmd, " ", &pstr);
                    token1 = strtok_r(NULL, " ", &pstr);
                    if(token1)
                    {
                        char *token2 = strtok_r(NULL, " ", &pstr);
                        if(token2)
                        {
                            //get host and port
                            host = token1;
                            p = atoi(token2);
                        }
                        else
                            //get port
                            p = atoi(token1);
                    }
                }
				std::cout<<"\nconnecting to "<<host<<":"<<p<<std::flush;
				try {
    				runClient(host, p, 0);
        	    }
        	    catch(int &ex) {
                    std::cout<<"Server: Exception "<<ex<<" - "<<Layer6Msg::getErrorStrForCode(ex)<<std::endl;
        	    }
        	    catch(...) {
                    std::cout<<"Server: Unknown exception."<<std::endl;
        	    }
			}
            else if(!strncmp(cmd, "fw ", 3)) {
                char *fname = cmd + 3;
                fname[strlen(fname) - 1] = '\0';
                printf("\nWriting to file %s:", fname); fflush(stdout);
                int fd = open(fname, O_CREAT | O_WRONLY, 0664);
                if(fd >= 0) 
                {
                    runClient(host, 0, fd);
                    close(fd);
                }
                else
                    printf("\nUnable to write to file %s.", fname);                  

            }
            else if(!strncmp(cmd, "fr ", 3)) {
                char *fname = cmd + 3;
                fname[strlen(fname) - 1] = '\0';
                printf("\nReading from file %s:", fname); fflush(stdout);
                FILE *fp = fopen(fname, "rb");
                if(fp) {
                    fseek(fp , 0 , SEEK_END);
                    int len = ftell(fp);
                    fseek(fp , 0 , SEEK_SET);
                    char *buffer = (char*) malloc (len);
                    int ret = fread(buffer, 1, len, fp);
                    if(ret != len) {
                        printf("Reading error, read %d expected %d\n", ret, len); 
                    }
                    else {
                        handleClient(0, buffer, len);
                    }
                    free (buffer);
                    fclose(fp);
                }
                else {
                    printf("\nFile %s not found.", fname); 
                    fflush(stdout);
                }
            }
			else if(cmd[0] =='q') {
				exit(0);
			}
		}
	}
}

