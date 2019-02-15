#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <string>
#include <cstdlib>
#include <iostream>

//TODO:
#define MAX_READ_BUFF_SIZE 1024*1024
#define MAX_CLIENT_READ_BUFF_SIZE 128


class EventLoop;

class CTcpClient
{
public: 
	int   iFd;
	int   iMaxBuffSize;
	int   iCurrIdx;
	char  cBuff[MAX_READ_BUFF_SIZE];
};

class CTcpServer
{
public:
	int createTcpServer(const std::string &sIP,int iPort);
	int getLFd() { return _iFd;}

//callback
public:
	void setEventLoop( EventLoop *loop) { _eventloop = loop;}
	void acceptConnect(int iFd, void *privdata, int iMask);
	void readTcpData(int iFd, void *privdata, int iMask);
	void writeTcpData(int iFd,void *privdata, int iMask);

private:

	int _iFd;

	int _iPort;
	std::string _sIP;

	EventLoop* _eventloop;
};


class CSockUtil
{
public:
	static int connectTcpServer(const std::string &sIP,int iPort);
	static int sentToServer(int iFd, const std::string sBug);
	static int acceptTcpConnect(int iListenfd,std::string &sIP,int &iPort);
	static int setNoBlocking(int iFd);
	static int setReuseAddr(int iFd);
	static int readData(int iFd);	
};


#endif
