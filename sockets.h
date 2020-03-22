#include<stdio.h>
#include<windows.h>
#pragma comment(lib,"WS2_32.lib")
#pragma warning(disable:4996)
//#define DGPS_PROT 5437              //GNSS中心的差分网址
//#define DGPS_IP "119.253.45.108"

#define DGPS_PROT 5002                //发布伪距差分改正数
#define DGPS_IP "121.196.196.190"

bool OpenDGPSSocket(SOCKET& sock)
{
	WSADATA wsaData;
	SOCKADDR_IN addrSrv;

	if(!WSAStartup(MAKEWORD(1, 1), &wsaData))
	{
		if( (sock = socket(AF_INET, SOCK_STREAM ,0)) != INVALID_SOCKET )
		{
			addrSrv.sin_addr.S_un.S_addr = inet_addr(DGPS_IP);
			addrSrv.sin_family = AF_INET;
			addrSrv.sin_port = htons(DGPS_PROT);
			connect(sock, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));	
			return true;
		}
	}
	return false;
}
