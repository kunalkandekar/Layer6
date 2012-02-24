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
#include <unistd.h>

#include "layer6.h"
//#include "dmalloc.h"

int is_server;

int open_tcp_sock(int port) 
{
    struct sockaddr_in local;
    SOCKET sd;
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
    local.sin_port = htons(port) ;
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sd ,(struct sockaddr *)&local , sizeof(local)) < 0) 
    {
        perror("bind");
        return -1;
    }
    return sd;
}

int connect_sock(int sd, char* remote, int port) 
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
    unsigned int addr = inet_addr(remote);
    if(addr != INADDR_NONE)
    {
        /* connect to server */
        //printf("\ninet_aton\n");
        printf("1: connecting to  %s (%u): %d",remote, addr, port);
        remote_addr.sin_addr.s_addr = addr;
        return connect( sd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    }
    else 
    {
        hp = gethostbyname(remote);
        printf("\ngethostbyname %s = %p\n", remote, hp);
        //log("2: connecting to  %s:%d -- %x",remote, port, hp);
        if(!hp)
            return -1;
        remote_addr.sin_family = AF_INET ;
        remote_addr.sin_port = htons(port) ;
        remote_addr.sin_addr = *(struct in_addr*)hp->h_addr;

        /* connect to server */
        return connect( sd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    }
    return -1;
}


void dprint(char *buffer, int offset, int length) 
{
    int i = 0;
    printf("\n[");
    for(i = 0; i < length; i++) 
    {
        printf("%2x",(unsigned int)buffer[offset+i]);
        if((i > 0) && (i%32 == 0)) 
        {
            printf("\n");
        }
    }
    printf("]");
    return;
}

int check_error(int ret, int *line, int lineno)
{
    *line = lineno - 1;
    return (ret != 0);
}

int dump(l6msg msg, char *opstr, int oplen, char *indent)
{
    int i = 0;
    int j = 0;
    int id;
    int type;
    int ret;
    int count;
    int line = 0;

    char *startopstr = opstr;
    char *endopstr   = opstr + oplen - 16;
    if(!indent) 
        indent = "";

    int len = l6msg_get_num_fields(&msg);
    char error[256]; error[0] = 0;
    for(i = 0; i < len; i++) 
    {
        const char *name = NULL;
        id = -1;

        opstr += snprintf(opstr, (endopstr - opstr), "\n%s%2d:", indent, i);
        ret = l6msg_get_field_id_at_index(&msg, i, &id);

        opstr += snprintf(opstr, (endopstr - opstr), " id=%2d", id);
        if(l6msg_is_field_named_at_index(&msg, i)) 
        {
            ret = l6msg_get_field_name_at_index(&msg, i, &name);
            opstr += snprintf(opstr, (endopstr - opstr), " name='%s'", name);
        }
        else 
        {
            opstr += snprintf(opstr, (endopstr - opstr), " name=''");
        }

        //l6msg_get_field_type(&msg, id, &type);
        l6msg_get_field_type_at_index(&msg, i, &type);
        opstr += snprintf(opstr, (endopstr - opstr), " type=[%d] ", type);
        //printf("[%d:id=%d type=%d name='%s'], ", i, id, type, (name ? name : ""));
        
        if(l6msg_is_field_array_at_index(&msg, i))
        {
            ret = (name ? l6msg_get_field_count_named(&msg, name, &count) :
                    ((id >= 0) ? l6msg_get_field_count(&msg, id, &count) :
                            l6msg_get_field_count_at_index(&msg, i, &count)));

            if(check_error(ret, &line, __LINE__))
            {
                int w = snprintf(error, sizeof(error), 
                                "\nError @ %d in %s: fid=%d (%s) msg=%s...", 
                                    line, __FILE__, id, (name ? name : ""), l6msg_get_debug_info(&msg)); 
                if(w > (endopstr - opstr))
                {
                    opstr -= (w - (endopstr - opstr) + 4);
                    opstr += snprintf(opstr, (endopstr - opstr), "...%s", error);
                }
                printf("%s...", error); fflush(stdout); 
                break;
            }
        }
        
        switch(type) 
        {
            case L6_DATATYPE_STRING: 
            {
                const char *data;    //[64];
                ret = (name ? l6msg_get_string_ptr_named(&msg, name, &data) :
                        ((id >= 0) ? l6msg_get_string_ptr(&msg, id, &data) :
                                l6msg_get_string_ptr_at_index(&msg, i, &data)));

                if(check_error(ret, &line, __LINE__)) break;

                opstr += snprintf(opstr, (endopstr - opstr), "(string) data=\"%s\"", data);
                break;
            }

            case L6_DATATYPE_BYTES: 
            {
                opstr += snprintf(opstr, (endopstr - opstr), "(byte [%d]) data=[", count);
                char *data;
                ret = (name ? l6msg_get_byte_array_ptr_named(&msg, name, &data, &count) :
                        ((id >= 0) ? l6msg_get_byte_array_ptr(&msg, id, &data, &count) :
                                l6msg_get_byte_array_ptr_at_index(&msg, i, &data, &count)));
                for(j = 0; j < count; j++) 
                    opstr += snprintf(opstr, (endopstr - opstr), "%02X ", data[j]);
                opstr += snprintf(opstr - (count > 0 ? 1 : 0), (endopstr - opstr), "]  ") - 2;
                //dprint(data, 0, count);
                break;
            }

            case L6_DATATYPE_SHORT: 
            {
                short data;
                ret = (name ? l6msg_get_short_named(&msg, name, &data) :
                        ((id >= 0) ? l6msg_get_short(&msg, id, &data) :
                                l6msg_get_short_at_index(&msg, i, &data)));
                if(check_error(ret, &line, __LINE__)) break;

                opstr += snprintf(opstr, (endopstr - opstr), "(short) data=%d", data);
                break;
            }

            case L6_DATATYPE_SHORT_ARRAY: 
            {
                opstr += snprintf(opstr, (endopstr - opstr), "(short [%d]) data=[", count);
                short *data = (short*)malloc(sizeof(short)*count);
                ret = (name ? l6msg_get_short_array_named(&msg, name, data, 0, count) :
                        ((id >= 0) ? l6msg_get_short_array(&msg, id, data, 0, count) :
                                l6msg_get_short_array_at_index(&msg, i, data, 0, count)));
                if(check_error(ret, &line, __LINE__)) break;

                for(j = 0; j < count; j++) 
                    opstr += snprintf(opstr, (endopstr - opstr), "%d, ", data[j]);
                opstr += snprintf(opstr - (count > 0 ? 2 : 0), (endopstr - opstr), "]  ") - 2;

                free(data);
                break;
            }

            case L6_DATATYPE_INT32: 
            {
                int data;
                ret = (name ? l6msg_get_int_named(&msg, name, &data) :
                        ((id >= 0) ? l6msg_get_int(&msg, id, &data) :
                                l6msg_get_int_at_index(&msg, i, &data)));
                if(check_error(ret, &line, __LINE__)) break;

                opstr += snprintf(opstr, (endopstr - opstr), "(int) data=%d",data);
                break;
            }

            case L6_DATATYPE_INT32_ARRAY: 
            {
                opstr += snprintf(opstr, (endopstr - opstr), "(int [%d]) data=[", count);
                int start = 0; //count = count / 2;  //get only half the array at an offset of 3
                int *data = (int*)malloc(sizeof(int)*count);

                ret = (name ? l6msg_get_int_array_named(&msg, name, data, start, count) :
                        ((id >= 0) ? l6msg_get_int_array(&msg, id, data, start, count) :
                                l6msg_get_int_array_at_index(&msg, i, data, start, count)));
                if(check_error(ret, &line, __LINE__)) break;

                for(j = 0; j < count; j++) 
                    opstr += snprintf(opstr, (endopstr - opstr), "%d, ",data[j]);
                opstr += snprintf(opstr - (count > 0 ? 2 : 0), (endopstr - opstr), "]  ") - 2;
                free(data);
                break;
            }

            //case L6_DATATYPE_INT64:
            case L6_DATATYPE_LONG: 
            {
                long long int data;
                ret = (name ? l6msg_get_long_named(&msg, name, &data) :
                        ((id >= 0) ? l6msg_get_long(&msg, id, &data) :
                                l6msg_get_long_at_index(&msg, i, &data)));
                if(check_error(ret, &line, __LINE__)) break;

                opstr += snprintf(opstr, (endopstr - opstr), "(long long int) data=%lld", data);
                break;
            }

            //case L6_DATATYPE_INT64_ARRAY:
            case L6_DATATYPE_LONG_ARRAY: 
            {
                opstr += snprintf(opstr, (endopstr - opstr), "(long long int [%d]) data=[", count);
                long long int *data = (long long int*)malloc(sizeof(long long int)*count);
                ret = (name ? l6msg_get_long_array_named(&msg, name, data, 0, count) :
                        ((id >= 0) ? l6msg_get_long_array(&msg, id, data, 0, count) :
                                l6msg_get_long_array_at_index(&msg, i, data, 0, count)));
                if(check_error(ret, &line, __LINE__)) break;

                for(j = 0; j < count; j++) 
                    opstr += snprintf(opstr, (endopstr - opstr), "%lld, ", data[j]);

                opstr += snprintf(opstr - (count > 0 ? 2 : 0), (endopstr - opstr), "]  ") - 2;
                                free(data);
                break;
            }

            case L6_DATATYPE_FLOAT: 
            {
                float data;
                ret = (name ? l6msg_get_float_named(&msg, name, &data) :
                        ((id >= 0) ? l6msg_get_float(&msg, id, &data) :
                                l6msg_get_float_at_index(&msg, i, &data)));
                if(check_error(ret, &line, __LINE__)) break;

                opstr += snprintf(opstr, (endopstr - opstr), "(float) data=%0.4f",data);
                break;
            }

            case L6_DATATYPE_FLOAT_ARRAY: 
            {
                opstr += snprintf(opstr, (endopstr - opstr), "(float [%d]) data=[", count);
                float *data = (float*)malloc(sizeof(float)*count);
                ret = (name ? l6msg_get_float_array_named(&msg, name, data, 0, count) :
                        ((id >= 0) ? l6msg_get_float_array(&msg, id, data, 0, count) :
                                l6msg_get_float_array_at_index(&msg, i, data, 0, count)));
                if(check_error(ret, &line, __LINE__)) break;

                for(j = 0; j < count; j++) 
                    opstr += snprintf(opstr, (endopstr - opstr), "%0.4f, ",data[j]);
                opstr += snprintf(opstr - (count > 0 ? 2 : 0), (endopstr - opstr), "]  ") - 2;

                free(data);
                break;
            }

            case L6_DATATYPE_DOUBLE: 
            {
                double data;
                ret = (name ? l6msg_get_double_named(&msg, name, &data) :
                        ((id >= 0) ? l6msg_get_double(&msg, id, &data) :
                                l6msg_get_double_at_index(&msg, i, &data)));
                if(check_error(ret, &line, __LINE__)) break;

                opstr += snprintf(opstr, (endopstr - opstr), "(double) data=%0.4f",data);
                break;
            }

            case L6_DATATYPE_DOUBLE_ARRAY: 
            {
                opstr += snprintf(opstr, (endopstr - opstr), "(double [%d]) data=[", count);
                double *data;
                //data = (double*)malloc(sizeof(double)*count);
                //ret = l6msg_getdataay(&msg, id, data, 0, count);
                ret = (name ? l6msg_get_double_array_ptr_named(&msg, name, &data, &count) :
                        ((id >= 0) ? l6msg_get_double_array_ptr(&msg, id, &data, &count) :
                                l6msg_get_double_array_ptr_at_index(&msg, i, &data, &count)));
                if(check_error(ret, &line, __LINE__)) break;

                for(j = 0; j < count; j++) 
                    opstr += snprintf(opstr, (endopstr - opstr), "%0.4f, ", data[j]);
                opstr += snprintf(opstr - (count > 0 ? 2 : 0), (endopstr - opstr), "]  ") - 2;                break;
            }
            
            case L6_DATATYPE_L6MSG: 
            {
                opstr += snprintf(opstr, (endopstr - opstr), "(l6msg [%d]) data= {", count);
                l6msg sub_msg = NULL;
                //l6msg_init(&subMsg);
                ret = (name ? l6msg_get_layer6_msg_ptr_named(&msg, name, &sub_msg) :
                        ((id >= 0) ? l6msg_get_layer6_msg_ptr(&msg, id, &sub_msg) :
                                l6msg_get_layer6_msg_ptr_at_index(&msg, i, &sub_msg)));
                if(check_error(ret, &line, __LINE__)) break;                

                char indentsub[32];
                sprintf(indentsub, "%s\t", indent);
                opstr += dump(sub_msg, opstr, (endopstr - opstr), indentsub);
                opstr += snprintf(opstr, (endopstr - opstr), "}");
                //printf("\nServer: dumping deserialized sub-msg "); fflush(stdout);
                //l6msg_free(&subMsg);
                break;
            }

            default: 
            {
                int w = snprintf(error, sizeof(error), 
                                "\nError: UNKNOWN TYPE %d ---SHOULD NOT HAPPEN!!! fid=%d (%s) msg=%s...", 
                                    type, id, (name ? name : ""), l6msg_get_debug_info(&msg)); 
                if(w > (endopstr - opstr))
                {
                    opstr -= (w - (endopstr - opstr) + 4);
                    opstr += snprintf(opstr, (endopstr - opstr), "...%s", error);
                }
                printf("%s...", error); fflush(stdout);
                break;
            }
        }
        if(ret != 0) 
        {
            int w = snprintf(error, sizeof(error), 
                            "\nError @ %d in %s: fid=%d (%s) msg=%s...", 
                                line, __FILE__, id, (name ? name : ""), l6msg_get_debug_info(&msg)); 
            if(w > (endopstr - opstr))
            {
                opstr -= (w - (endopstr - opstr) + 4);
                opstr += snprintf(opstr, (endopstr - opstr), "...%s", error);
            }

            printf("\n\n%s...", error); fflush(stdout);
            break;
        }
        //check if we reached the end if opstr
        if(opstr >= endopstr)
        {
            opstr -= 4;
            opstr += snprintf(opstr, (endopstr - opstr), "...  "); 
            break;
        }         
    }
    return (opstr - startopstr);
}

int deserialize(char *buffer, int len, char *opstr, int oplen) 
{
    int left = 0;
    //deserialize
    printf("\nServer: deserializing.... ");
    l6msg msg;
    l6msg_init(&msg);

    int ret = l6msg_deserialize(&msg, buffer, len, &left);
    if(ret < 0) 
    {
        printf("\nError: [%s]\nDebug: [%s]", l6msg_get_error_str(&msg), l6msg_get_debug_info(&msg));
        return -1;
    }
    len = l6msg_get_num_fields(&msg);
    printf("\nServer: deserialized %d left=%d, num fields=%d", ret, left, len); fflush(stdout);
    ret = dump(msg, opstr, oplen, "\t");
    printf("\nServer: msg dump [len=%d]=%s", ret, opstr); fflush(stdout);

    l6msg_free(&msg);        
    printf("\nServer: freed recvd msg "); fflush(stdout);

    return ret;
}

int handle_client(int sd, char *buffer, int len) 
{
    char opstr[8192];
    int offset = 0;
    
    if(buffer != NULL) 
    {
        offset = deserialize(buffer, len, opstr, sizeof(opstr));
        printf("\nServer: deserialized %d bytes .... ", offset);        
        return 0;
    }

    len = 0;
    buffer = (char*)malloc(sizeof(char)*2048);

    printf("\nServer: Handling new client... %d", sd);
    len = recv(sd, buffer, 2048, 0);

    if(len < 0) 
    {
        printf("\nServer: Error no bytes read.... ");
        //close(sd);
        return -1;
    }
    else 
    {
        printf("\nServer: %d bytes read.... ", len);//+ dprint(buffer, 0, len));
    }

    offset = deserialize(buffer, len, opstr, sizeof(opstr));

    len = send(sd, opstr, offset, 0);
    //printf("\nServer: %d bytes sent.... ", len);
    offset = sprintf(opstr, "\nDone!");
    len = send(sd, opstr, offset, 0);
    //printf("\nServer: another %d bytes sent.... ", len);
    
    close(sd);

    printf("\nServer: connection closed");
    free(buffer);
    return 0;
}

int run_server(int port) 
{
    struct sockaddr_in remote;
    int    addr_len;
    if(port <= 0) 
    {
        port = 6989;
    }

    SOCKET svr = open_tcp_sock(port);

    int ret = listen(svr, 10);
    if(ret < 0) 
    {
        printf("\nServer: Unable to listen: %d errno=%d", ret, errno);
        return -1;
    }
    printf("\nServer: Running on port %d... ", port);

    while(1) 
    {
        addr_len = sizeof(struct sockaddr);
        int newconn = accept(svr, (struct sockaddr*)&remote, (socklen_t*)&addr_len);
        //remote.sin_addr.s_addr = ntohl(remote.sin_addr.s_addr);
        //remote.sin_port = ntohs(remote.sin_port);
        printf("\nServer: Accepted connection: %d from %s:%d",
            newconn, inet_ntoa(remote.sin_addr), remote.sin_port);
        handle_client(newconn, NULL, 0);
        //close(newconn);
    }
    return 0;
}



int run_client(char *host, int port, int altfd) {
    char buffer[4096];  // = (char*)malloc(sizeof(char)*4096);
    int len = 0;
    int ret = 0;
    if(port <= 0) 
    {
        port = 6989;
    }
    
    int add_subsubmsg = 1;

    double darr[] = {1.2, 3.45, 5.67, 7.89, 0.91};
    int iarr[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    printf("\nClient: starting up\n"); fflush(stdout);

    l6msg msg;
    l6msg_init(&msg);

    //set data
    printf("short, "); fflush(stdout);
    ret = l6msg_set_short(&msg, 1, (short)43);
    if(ret < 0) 
    {
        printf("\nError in set short (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }

    printf("int, "); fflush(stdout);
    ret = l6msg_set_int(&msg, 2, -23);
    if(ret < 0) 
    {
        printf("\nError in set int (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }
    
    /*
    //Uncomment this block to test error handling
    short t[5];
    ret = l6msg_get_short_array(&msg, 2, t, 0, 5);
    if(ret < 0) 
    {
        printf("\nError in get int (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }
    */
    
    printf("long, "); fflush(stdout);
    ret = l6msg_set_long_named(&msg, "l", 3847);
    if(ret < 0) 
    {
        printf("\nError in set long (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }

    printf("float, "); fflush(stdout);
    ret = l6msg_set_float_named(&msg, "float",(float)39.31);
    if(ret < 0)
    {
        printf("\nError in set float (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }

    printf("double, "); fflush(stdout);
    ret = l6msg_add_double(&msg, 1278.409823);
    if(ret < 0) 
    {
        printf("\nError in add double (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }
    
    printf("overwrite float, "); fflush(stdout);
    ret = l6msg_set_float_at_index(&msg, 3,(float)49.312);
    if(ret < 0)
    {
        printf("\nError in overwrite float (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }
    

    printf("double array, "); fflush(stdout);
    ret = l6msg_set_double_array_named(&msg, "da", darr, 5);
    if(ret < 0) 
    {
        printf("\nError in set double array (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }

    printf("int array, "); fflush(stdout);
    ret = l6msg_set_int_array_ptr(&msg, 4, iarr, 10);
    if(ret < 0) 
    {
        printf("\nError in set int array ptr (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }

    printf("string, "); fflush(stdout);
    ret = l6msg_set_string(&msg, 5,"Yeeehaaaah!");
    if(ret < 0) 
    {
        printf("\nError in set string (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        return -1;
    }

    printf("submsg... "); fflush(stdout);
    l6msg sub_msg;
    l6msg_init(&sub_msg);
    ret = l6msg_set_string_named(&sub_msg, "sub-f1", "sub msg field 1");
    if(ret < 0) 
    {
        printf("\nError in set string named (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        l6msg_free(&sub_msg);
        return -1;
    }
    ret = l6msg_set_string_ptr(&sub_msg, 5,"sub msg field 2!");
    if(ret < 0) 
    {
        printf("\nError in set string ptr (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        l6msg_free(&sub_msg);
        return -1;
    }
    
    ret = l6msg_set_string_ptr_named(&sub_msg, "temp", "I'm just gonna get deleted in a few lines...");
    if(ret < 0) 
    {
        printf("\nError in set named string ptr (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        l6msg_free(&sub_msg);
        return -1;
    }
    
    ret = l6msg_set_layer6_msg_ptr_named(&msg, "sub", &sub_msg);
    if(ret < 0) 
    {
        printf("\nError in set sub msg (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        l6msg_free(&sub_msg);
        return -1;
    }
    printf("\nClient: Added sub msg w/ nfields = %d, size=%d...", l6msg_get_num_fields(&sub_msg), l6msg_size(&sub_msg)); fflush(stdout);
    printf("to msg, now having nfields = %d, size=%d...", l6msg_get_num_fields(&msg), l6msg_size(&msg)); fflush(stdout);
    int tempintarr[] = {1,2,3,5,7,11,13};
    ret = l6msg_set_int_array_ptr(&sub_msg, 3, tempintarr, sizeof(tempintarr)/sizeof(int));
    if(ret < 0) 
    {
        printf("\nError int set int array ptr (%d in %s): %s", __LINE__, __FILE__, l6msg_get_debug_info(&sub_msg));
        l6msg_free(&msg);
        l6msg_free(&sub_msg);
        return -1;
    }

    printf("\nClient: Added field to sub msg w/ nfields = %d, size=%d...", l6msg_get_num_fields(&sub_msg), l6msg_size(&sub_msg)); fflush(stdout);
    printf("to msg, now having nfields = %d, size=%d...", l6msg_get_num_fields(&msg), l6msg_size(&msg)); fflush(stdout);

	if(add_subsubmsg) 
	{
        l6msg sub_submsg;
        l6msg_init(&sub_submsg);
        short *sarr= (short*)malloc(4 * sizeof(short)); sarr[0] = 1; sarr[1] = 3; sarr[2] = 4; sarr[3] = 5;
        l6msg_set_short_array_named(&sub_submsg, "short-array", sarr, 4);
        free(sarr);
        l6msg_set_string_named(&sub_submsg, "sub-sub-key1", "qwerty");
        l6msg_set_string_ptr_named(&sub_submsg, "yowzah", "Lorem ipsum dolor sit amet."); //this may be a problem...

        //sub_submsg will go out of scope, but we should have a copy of it safe and sound        
        l6msg_set_layer6_msg_named(&sub_msg, "subsub", &sub_submsg);
        
        l6msg sub_submsg_temp;
        l6msg_init(&sub_submsg_temp);
        char *bytes = malloc(16);
        l6msg_set_byte_array_named(&sub_submsg_temp, "garbage", bytes, 16);
        free(bytes);
    
        l6msg_set_layer6_msg_named(&sub_submsg, "subsubTmp", &sub_submsg_temp);

        l6msg_set_int(&sub_msg, 32, 32);
        
        printf("\n\tClient: msg nfields = %d [%d bytes], sub msg nfields = %d [%d bytes]...", 
            l6msg_get_num_fields(&msg), l6msg_size(&msg),
            l6msg_get_num_fields(&sub_msg), l6msg_size(&sub_msg));  //<<"] bytes, buf=["<<sizeof(buffer)+"] bytes..."<<std::flush;
        //std::cout<<"\n\t\t subMsg.size="<<subMsg.size()<<", l6msg_size="<<l6msg_size();
        
        //removing fields
        l6msg_remove_field_named(&sub_submsg, "subsubTmp");
        l6msg_remove_field(&sub_msg, 32);
        
        //set another copy
        l6msg_set_layer6_msg(&msg, 99, &sub_submsg);
        
        //serlz for test
        int left;
        char buf2[4096];
        int l = l6msg_serialize(&sub_submsg, buf2, sizeof(buf2), &left);
        printf("\n\tTest serlz re=%d left=%d\n", l, left);
        
        l6msg_free(&sub_submsg_temp);
        l6msg_free(&sub_submsg);    //we can free since we've set a deep copy in the parent msg
	}

    ret = l6msg_remove_field_named(&sub_msg, "temp");
    if(ret < 0) 
    {
        printf("\nError removing field (%d in %s): [%s]", __LINE__, __FILE__, l6msg_get_debug_info(&sub_msg));
        l6msg_free(&msg);
        l6msg_free(&sub_msg);
        return -1;
    }
    printf("\nClient: Removed field from sub msg w/ nfields = %d, size=%d...", l6msg_get_num_fields(&sub_msg), l6msg_size(&sub_msg)); fflush(stdout);
    printf("to msg, now having nfields = %d, size=%d...", l6msg_get_num_fields(&msg), l6msg_size(&msg)); fflush(stdout);
    
    //get and re-add a submsg if it's there
    l6msg sub_submsg;
    ret = l6msg_get_layer6_msg_ptr(&msg, 99, &sub_submsg);
    if(ret >= 0) {
        //add a new field
        long long larr[] = {112, 35, 81321, 34, 5589, 144}; 
        l6msg_add_long_array_ptr(&sub_submsg, larr, sizeof(larr)/sizeof(unsigned long long));
        
        //remove an old field
        l6msg_remove_field_named(&sub_submsg, "yowzah");
    }
    else
    {
        printf("\nClient: Could not get submsg field at id %d in msg w/ nfields = %d, size=%d...", 99, l6msg_get_num_fields(&msg), l6msg_size(&msg)); 
        fflush(stdout);
    }

#if __STDC_VERSION__ >= 199901L
    ret = l6msg_add_byte_array(&msg, (char[]){9, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0}, 12);
#else
    ret = l6msg_add_byte_array(&msg, (char*)iarr, 12);
#endif

    printf("\nClient: Msg nfields = %d, serializing...", l6msg_get_num_fields(&msg)); fflush(stdout);

    int left;
    len = l6msg_serialize(&msg, buffer, 4096, &left);
    if(len < 0) 
    {
        printf("\n%s", l6msg_get_debug_info(&msg));
        l6msg_free(&msg);
        l6msg_free(&sub_msg);
        return -1;
    }

    ret = l6msg_get_size(&msg);

    printf("\n\nClient: Msg bytes expected=%d actual=%d left=%d", ret, len, left);// + ":" +dprint(buffer, 0, len));
    
    if(altfd) 
    {
        if(altfd < 0)
            ret = handle_client(-1, buffer, ret);
        else
            ret = write(altfd, buffer, ret);

        l6msg_free(&msg);
        l6msg_free(&sub_msg);

        printf("\nClient: exiting "); fflush(stdout);
        return ret;
    }
       

    SOCKET sock = open_tcp_sock(0);
    if(sock < 0) 
    {
        perror("\nClient: could not open socket - ");
        l6msg_free(&msg);
        l6msg_free(&sub_msg);
        return -1;
    }

    ret = connect_sock(sock, host, port);
    if(ret < 0) 
    {
        perror("\nClient: could not connect - ");
        if(1)
        {
            l6msg_free(&msg);
            l6msg_free(&sub_msg);
            return -1;
        }
        char opstr[4096];
        int offset = 0;
        printf("\nTrying locally---\n");        
        offset = deserialize(buffer, len, opstr, sizeof(opstr));
        opstr[offset] = '\0';
        printf("\nClient: %s", opstr);
        printf("\nDone!!");
    }
    else {
        ret = send(sock, buffer, len, 0);
        if(ret < 0) 
        {
            printf("\nClient: Error in sending bytes.... "); perror("\nClient: "); fflush(stdout);
            //close(sock);
            l6msg_free(&msg);
            l6msg_free(&sub_msg);
            return -1;
        }
    
        printf("\nClient: Sent msg %d bytes on %d", ret, sock); fflush(stdout);//["+new String(buffer,0,len)+"]");
    
        while(1) 
        {
            len = recv(sock, buffer, 4096, 0);
            if(len <= 0) 
            {
                printf("\nClient: Error in reading bytes.... "); fflush(stdout);
                //close(sock);
                l6msg_free(&msg);
                l6msg_free(&sub_msg);
                return -1;
            }
            buffer[len] = '\0';
            printf("\nClient: Server sez len=%d (%zd) {%s\n}", len, strlen(buffer), buffer); fflush(stdout);
            if(strstr(buffer, "Done!!!") != NULL) 
            {
                break;
            }
            break;
        }
    }
    //close(sock);
    printf("\nClient: freeing msg "); fflush(stdout);
    l6msg_free(&msg);
    printf("\nClient: freeing sub-msg "); fflush(stdout);
    l6msg_free(&sub_msg);
    printf("\nClient: exiting "); fflush(stdout);
    return 0;
}

void* run(void* is_svr) 
{
    if(is_svr) 
    {
        int port = (*(int*)is_svr);
        //printf("\n\t args=%d... ", port); fflush(stdout);
        run_server(port);
    }
    else 
    {
        run_client("localhost", 0, 0);
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

    is_server = 0;
    //printf("\n sizeof int=%d long int=%d long long int=%d", sizeof(int), sizeof(long int), sizeof(long long int));
    //printf("\n htons(-23)=%d", htons(-23));
    pthread_t thr;
    int port = 6989;
    
    if(argc > 1) 
    {
        //server
        is_server = atoi(argv[1]);
        port = is_server;
        
        //start server
        pthread_create(&thr, NULL, &run, &is_server);
    }
    else 
    {
        is_server = 0;
    }
    if(argc > 2) 
    {
        //sleep(1);
        is_server = 0;
        run(0);    //start client
        pthread_join(thr, NULL);
    }
    else if((argc > 3) || ((argc > 1) && !strcmp(argv[1], "0"))) 
    {
       run_client("localhost", 0, -1);
    }
    else 
    {
        while(1) 
        {
            printf("\ncmd:"); fflush(stdout);
            char cmd[32];
            char *host = "localhost";
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
                printf("\nconnecting to %s:%d:", host, p); fflush(stdout);
                run_client(host, p, 0);
            }
            else if(!strncmp(cmd, "fw ", 3)) 
            {
                char *fname = cmd + 3;
                fname[strlen(fname) - 1] = '\0';
                printf("\nWriting to file %s:", fname); fflush(stdout);
                int fd = open(fname, O_CREAT | O_WRONLY, 0664);
                if(fd >= 0) 
                {
                    run_client(host, 0, fd);
                    close(fd);
                }
                else
                {
                    perror("open");
                    printf("\nUnable to write to file %s.", fname);
                }
                    
            }
            else if(!strncmp(cmd, "fr ", 3)) 
            {
                char *fname = cmd + 3;
                fname[strlen(fname) - 1] = '\0';
                printf("\nReading from file %s:", fname); fflush(stdout);
                FILE *fp = fopen(fname, "rb");
                if(fp) 
                {
                    fseek(fp , 0 , SEEK_END);
                    int len = ftell(fp);
                    fseek(fp , 0 , SEEK_SET);
                    char *buffer = (char*) malloc (len);
                    int ret = fread(buffer, 1, len, fp);
                    if(ret != len) 
                    {
                        printf("Reading error, read %d expected %d\n", ret, len); 
                    }
                    else
                    {
                        handle_client(0, buffer, len);
                    }
                    free (buffer);
                    fclose(fp);
                }
                else 
                {
                    printf("\nFile %s not found.", fname); 
                    fflush(stdout);
                }
            }
            else if(cmd[0] =='q') 
            {
                exit(0);
            }
        }
    }
    return 0;
}