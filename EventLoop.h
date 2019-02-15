#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <sys/epoll.h>
#include <sys/types.h>

#include <string>
#include <cstdlib>
#include <mutex>
#include <memory>
#include <map>
#include <functional>

#include "Network.h"

enum EFileEvent
{
	FE_READ_ABLE  = 1,
	FE_WRITE_ABLE = 2,
};

struct FileEvent;
struct FiredEvent;


class Epoller
{
public:
	Epoller(){
		_iEpfd = epoll_create(1024);
	}

	int epoll_add(int iFd,int iMask);

	int epoll_mod(int iFd,int iMask);

	int epoll_del(int iFd,int iMask);

	//virtual int epoll_wait(std::map<int,FiredEvent> &mFiredEvent) { return 0;}

	int getEpfd(){ return _iEpfd;}

private:

	int _iEpfd;
};

typedef  std::function<void(int,void *,int)> funcFileProc;


class EventLoop : public Epoller
{
public:

	EventLoop(int iSetSize);

	~EventLoop(){ if(_pevents) delete [] _pevents;}

	int addFileEvent(int iFd, const FileEvent &tEvent);

	int delFileEvent(int iFd);

	void proccessEvent();

	int createClient(int iFd);

	int epoll_wait(std::map<int,FiredEvent> &mFiredEvent);	

	std::shared_ptr<CTcpClient> getClient(int iFd);

	int addListenServer();

	int regClient(int iFd);

	int startLoop();

private:

	CTcpServer *_tcpServer;

	struct epoll_event* _pevents;
	int _iSetSize;

	std::mutex _mutex;
	//注册的event
	std::map<int,FileEvent> _mapFd2FileEvent; 
	//就绪的event
	std::map<int,FiredEvent> _mapFd2FiredEvent; 
	//客户端连接
	std::map<int, std::shared_ptr<CTcpClient>> _mapFd2ClientSP;

};

struct FileEvent
{
	int iMask;
	funcFileProc rProc;
	funcFileProc wProc;
};

struct FiredEvent
{
	int iFd;
	int iMask;
};



#endif
