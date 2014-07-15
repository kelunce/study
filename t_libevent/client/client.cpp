﻿#include<Winsock2.h>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#define PORT 9995
#define BUFFER 1024


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


unsigned int BKDRHash(const unsigned char  *str, unsigned int len)
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

void main(int argc,char *argv[])
{
	WSADATA wsaData;
	SOCKET client;
	int port=PORT;

	int iLen;  //从服务器接受的数据长度
	
	struct sockaddr_in serv;  //服务器端地址


	if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
	{
		printf("Winsock load failed\n");
		return;
	}

	serv.sin_family=AF_INET;
	serv.sin_port=htons(port);
	serv.sin_addr.s_addr=inet_addr("192.168.12.7");

	client=socket(AF_INET,SOCK_STREAM,0);
	if(client==INVALID_SOCKET)
	{
		printf("scoket() failed:%d\n",WSAGetLastError());
		return;
	}

	if(connect(client,(struct sockaddr*)&serv,sizeof(serv))==INVALID_SOCKET)
	{
		printf("connect() failed:%d\n",WSAGetLastError());
		return;
	}
	else
	{
		char buf[BUFFER];  //接受数据的缓冲
		cs_data_st tmp;
		tmp.len = sizeof(tmp.str);
		LARGE_INTEGER liPerfFreq,liPerfStart,liPerfEnd;
		QueryPerformanceFrequency(&liPerfFreq);//获取每秒多少CPU Performance Tick
		while(1)
		{
			//接受数据缓冲区初始化
			memset(buf,0,sizeof(buf));
			iLen = 0;
			memset(tmp.str, 0, sizeof(tmp.str));
			unsigned int curtime = GetTickCount();
			sprintf((char *)&(tmp.str[0]),"Elapsed time:%u secs.\n",curtime/1000); 
			QueryPerformanceCounter(&liPerfStart);
			send(client,(const char*)&tmp,sizeof(cs_data_st),0 );
			while(iLen<sizeof(sc_data_st))
			{
				int tmp = recv(client,buf+iLen,sizeof(buf)-iLen,0);
				if(tmp==SOCKET_ERROR)
				{
					printf("recv() failed:%d\n",WSAGetLastError());
					return;
				}
				iLen += tmp;
			}

			sc_data_st *data = (sc_data_st *)buf;
			QueryPerformanceCounter(&liPerfEnd);
			int endlock = GetTickCount();
			double cost = ((double)liPerfEnd.QuadPart - (double)liPerfStart.QuadPart)/(double)liPerfFreq.QuadPart;
			printf("freq:%lld(double:%lf) diff(ms):%lf \n",liPerfFreq.QuadPart,(double)liPerfFreq.QuadPart, 1000*cost);
			printf("cost = %d \n", endlock - curtime);
			printf("recv() data lend=%d from server:%d  ==? %d \n",iLen, data->data,BKDRHash(tmp.str, tmp.len));
			Sleep(2000);
		}
		
		closesocket(client);
		WSACleanup();

		printf("press any key to continue");
		
	}
}