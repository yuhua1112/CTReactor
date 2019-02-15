#include "EventLoop.h"

int Epoller::epoll_add(int iFd,int iMask)
{
	struct epoll_event ev;
	memset(&ev,0,sizeof(ev));

	if( FE_READ_ABLE & iMask){
		ev.events |= EPOLLIN;
	}
	if( FE_WRITE_ABLE & iMask)
		ev.events |= EPOLLOUT;
	ev.data.fd = iFd;

	return epoll_ctl(_iEpfd,EPOLL_CTL_ADD,iFd,&ev);
}

int Epoller::epoll_mod(int iFd,int iMask)
{
	struct epoll_event ev;
	memset(&ev,0,sizeof(ev));	
	if( FE_READ_ABLE & iMask) 
		ev.events |= EPOLLIN;

	if( FE_WRITE_ABLE & iMask)
		ev.events |= EPOLLOUT;
	ev.data.fd = iFd;
	
	return epoll_ctl(_iEpfd,EPOLL_CTL_MOD,iFd,&ev);
}

int Epoller::epoll_del(int iFd,int iMask)
{
	struct epoll_event ev;
	memset(&ev,0,sizeof(ev));	
	if( FE_READ_ABLE & iMask) 
		ev.events |= EPOLLIN;

	if( FE_WRITE_ABLE & iMask)
		ev.events |= EPOLLOUT;
	ev.data.fd = iFd;
	
	return epoll_ctl(_iEpfd,EPOLL_CTL_DEL,iFd,0);
}
 

std::shared_ptr<CTcpClient> EventLoop::getClient(int iFd)
{
	auto it = _mapFd2ClientSP.find(iFd);
	if(it != _mapFd2ClientSP.end()){
		return it->second;
	}
	return nullptr;
}

int EventLoop::epoll_wait(std::map<int,FiredEvent> &mFiredEvent)
{
	int eventnum =  ::epoll_wait(getEpfd(),_pevents,_iSetSize,20000);
	//std::cout << "epoll_wait return:" << eventnum << std::endl;

	for(int i = 0; i < eventnum; i++){
		FiredEvent event;
		int iMask = 0;
		if(_pevents[i].events & EPOLLIN ) iMask |= FE_READ_ABLE;
		if(_pevents[i].events & EPOLLOUT) iMask |= FE_WRITE_ABLE;
		if(_pevents[i].events & EPOLLERR) iMask |= FE_WRITE_ABLE;
		if(_pevents[i].events & EPOLLHUP) iMask |= FE_WRITE_ABLE;

		event.iMask = iMask;
		event.iFd = _pevents[i].data.fd;
		mFiredEvent[event.iFd] = event;

		//std::cout << "epoll_wait fd:" << event.iFd <<"|" <<  _pevents[i].events  << std::endl;	
	}
	return eventnum;
}

EventLoop::EventLoop(int iSetSize)
{
	if(iSetSize <= 0 && iSetSize > 10000)
		_iSetSize = 10000;
	_pevents = new struct epoll_event[iSetSize];
	_iSetSize = iSetSize;
}


int EventLoop::addFileEvent(int iFd, const FileEvent &tEvent )
{
	std::lock_guard<std::mutex> lock(_mutex);
	_mapFd2FileEvent[iFd] = tEvent;
	return epoll_add(iFd,tEvent.iMask);
}

int EventLoop::delFileEvent(int iFd)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_mapFd2FileEvent.erase(iFd);

	epoll_del(iFd,0);
}

int EventLoop::createClient(int iFd)
{
	auto spClient = std::make_shared<CTcpClient>();
	spClient->iFd;
	_mapFd2ClientSP[iFd] = spClient;
	return 0;
}

void EventLoop::proccessEvent()
{	
	for(;;){
		std::map<int,FiredEvent> mFiredEvent;
		//std::cout <<"startLoop ..." << std::endl;
		int evnetnum = epoll_wait(mFiredEvent);
		//std::cout <<  evnetnum << " Event comming" << std::endl;

		for(auto &kv : mFiredEvent){
			if(kv.second.iMask & FE_READ_ABLE){
				auto it = _mapFd2FileEvent.find(kv.first);
				if(it != _mapFd2FileEvent.end()){
					it->second.rProc(kv.first,0,kv.second.iMask);
				}
			}

			if(kv.second.iMask & FE_WRITE_ABLE){
				//TODO:
			}

		}
	}
}

int EventLoop::regClient(int iFd)
{
	FileEvent event;
	event.iMask = FE_READ_ABLE;
	event.rProc = std::bind(&CTcpServer::readTcpData,_tcpServer,std::placeholders::_1,
		std::placeholders::_2,std::placeholders::_3);

	addFileEvent(iFd,event);

}

int EventLoop::addListenServer()
{
	_tcpServer = new CTcpServer();
	_tcpServer->setEventLoop(this);

	int iListenFd = _tcpServer->createTcpServer("10.64.50.189",28510);

	std::cout <<"create tcp server success : " << iListenFd << std::endl;

	CSockUtil::setNoBlocking(iListenFd);

	FileEvent event;
	event.iMask = FE_READ_ABLE;
	event.rProc = std::bind(&CTcpServer::acceptConnect,_tcpServer,std::placeholders::_1,
		std::placeholders::_2,std::placeholders::_3);

	addFileEvent(iListenFd,event);

}

int EventLoop::startLoop()
{
	addListenServer();

	proccessEvent();

}