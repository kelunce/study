// LibeventTest.cpp : 定义控制台应用程序的入口点。
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

        // 这里整理输入缓冲区
		__int16 *pDataLen = (__int16 *)evbuffer_pullup(input,sizeof(__int16));
		__int16 iBuffLen = *pDataLen;
		if( (sizeof(__int16) + iBuffLen) > buffsize)
			break;
		// 数据完整
        // 不要复制memcpy(buf, evbuffer_pullup(input,packetSize), packetSize);
        packetSize = iBuffLen + sizeof(__int16);
		const unsigned char *buff = evbuffer_pullup(input, packetSize);
		//处理数据包buff
		
        //test 只有一种数据包
		cs_data_st *data = (cs_data_st *)buff;
		unsigned int ret = BKDRHash((char*)&(data->str[0]),sizeof(cs_data_st) - sizeof(__int16));
		sc_data_st tmp;
		tmp.len = data->len;
		tmp.data = ret;
		evbuffer_add(output,&tmp,sizeof(sc_data_st));


        // 上面处理完成了再释放
		evbuffer_drain(input, packetSize) ;
		printf("server calc answer %d\n",ret);
		//使用bufferevent_write或者evbuffer_add发送数据
		//evbuffer_write和evbuffer_read，用于直接在套接字上写/读数据,在关闭连接前一般flush this cache是调用
		//evbuffer_add具体在什么时候发送取决于write buff设置的最低水位,默认0
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

// 一般接到这个错误回调都关闭连接
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
	//一般而言,都不需要写回调
    bufferevent_setcb(bev, conn_readcb, /*conn_writecb*/NULL, conn_eventcb, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
	// 这里可以设置read buff的最低(默认0)和最高(默认无穷)水位,这里不需要设置,即使读到一个字节也回调
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
