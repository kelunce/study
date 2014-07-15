// LibeventTest.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#define __int16 unsigned short
#else

#include <WinSock2.h>

#endif

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/util.h>



static const int PORT = 9995;


#pragma pack(push,1)
struct cs_data_st{
	__int16 len;
	unsigned char str[48];
};
struct sc_data_st{
	__int16 len;
	unsigned int data;
};
#pragma pack(pop)

// BKDR Hash Function
unsigned int BKDRHash(char *str, unsigned int len)
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (len>0)
	{
		len--;
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

void conn_readcb(struct bufferevent *bev, void *user_data)
{
	struct evbuffer *input, *output;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);
	unsigned int packetSize = 0;
	unsigned int buffsize = 0;
	while((buffsize = evbuffer_get_length(input)) > 0)
	{
		if(sizeof(__int16) > buffsize)
			break;

        // �����������뻺����
		__int16 *pDataLen = (__int16 *)evbuffer_pullup(input,sizeof(__int16));
		__int16 iBuffLen = *pDataLen;
		if( (sizeof(__int16) + iBuffLen) > buffsize)
			break;
		// ��������
        // ��Ҫ����memcpy(buf, evbuffer_pullup(input,packetSize), packetSize);
        packetSize = iBuffLen + sizeof(__int16);
		const unsigned char *buff = evbuffer_pullup(input, packetSize);
		//�������ݰ�buff
		
        //test ֻ��һ�����ݰ�
		cs_data_st *data = (cs_data_st *)buff;
		unsigned int ret = BKDRHash((char*)&(data->str[0]),sizeof(cs_data_st) - sizeof(__int16));
		sc_data_st tmp;
		tmp.len = data->len;
		tmp.data = ret;
		evbuffer_add(output,&tmp,sizeof(sc_data_st));


        // ���洦����������ͷ�
		evbuffer_drain(input, packetSize) ;
		printf("server calc answer %d\n",ret);
		//ʹ��bufferevent_write����evbuffer_add��������
		//evbuffer_write��evbuffer_read������ֱ�����׽�����д/������,�ڹر�����ǰһ��flush this cache�ǵ���
		//evbuffer_add������ʲôʱ����ȡ����write buff���õ����ˮλ,Ĭ��0
	}
}

static void conn_writecb(struct bufferevent *bev, void *user_data)
{
	struct evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) 
    {
        printf("flushed answer\n");
        bufferevent_free(bev);
    }
}

// һ��ӵ��������ص����ر�����
static void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
    if (events & BEV_EVENT_EOF) 
    {
		/* connection has been closed, do any clean up here */
        printf("Connection closed.\n");
    } 
    else if (events & BEV_EVENT_ERROR) 
    {
		/* check errno to see what error occurred */
        printf("Got an error on the connection: %s\n",
            strerror(EVUTIL_SOCKET_ERROR()));/*XXX win32*/
	} 
	else if (events & BEV_EVENT_TIMEOUT) {
		/* must be a timeout event handle, handle it */
		/* ... */
	}

    /* None of the other events can happen here */
    bufferevent_free(bev);// close conn
}

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *user_data)
{
    struct event_base *base = (struct event_base *)user_data;
    struct bufferevent *bev;

    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) 
    {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(base);
        return;
    }
	//һ�����,������Ҫд�ص�
    bufferevent_setcb(bev, conn_readcb, /*conn_writecb*/NULL, conn_eventcb, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
	// �����������read buff�����(Ĭ��0)�����(Ĭ������)ˮλ,���ﲻ��Ҫ����,��ʹ����һ���ֽ�Ҳ�ص�
	//bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
	evutil_make_socket_nonblocking(fd);
}

int main(int argc, char **argv)
{
    struct event_base *base;
    struct evconnlistener *listener;
    struct event *signal_event;

    struct sockaddr_in sin;

#ifdef WIN32
    WSADATA wsa_data;
    WSAStartup(0x0201, &wsa_data);
#endif

    base = event_base_new();
    if (!base) 
    {
        fprintf(stderr, "Could not initialize libevent!\n");
        return 1;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);

    listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
        LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr*)&sin,
        sizeof(sin));

    if (!listener) 
    {
        fprintf(stderr, "Could not create a listener!\n");
        return 1;
    }

    event_base_dispatch(base);

    evconnlistener_free(listener);
  
    event_base_free(base);

    printf("done\n");
    return 0;
}
