#include "Network.h"
#include "EventLoop.h"

using namespace std;

int CSockUtil::setNoBlocking(int iFd)
{
	int iOFlag = fcntl(iFd,F_GETFL);
	if(iOFlag == -1) 
		return -1;
	if( fcntl(iFd,F_SETFL,iOFlag | O_NONBLOCK) == -1)
		return -1;
	return 0;
}

int CSockUtil::setReuseAddr(int iFd)
{
	int yes = 1;
	int iRet = setsockopt(iFd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
	return iRet;
}

int CSockUtil::connectTcpServer(const std::string &sIP,int iPort)
{
	int iRet = -1;
	int iFd = socket(AF_INET,SOCK_STREAM,0);
	if(iFd <= 0){
		cout <<"socket error: " << errno <<"|" << strerror(errno) << endl;
		return iFd;
	}

	sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(iPort);
	inet_pton(AF_INET,sIP.c_str(),&addr.sin_addr);

	iRet = connect(iFd,(struct sockaddr*)&addr,sizeof(addr));
	if(iRet != 0){
		cout <<"connect error:" << errno <<"|" << strerror(errno) << endl;
		return iRet;
	}

	return iFd;
}

int CSockUtil::readData(int iFd)
{
	char buf[MAX_CLIENT_READ_BUFF_SIZE];
	memset(buf,0,MAX_CLIENT_READ_BUFF_SIZE);

	int readnum = read(iFd,buf,MAX_CLIENT_READ_BUFF_SIZE);
	if(readnum > 0){
		//std::cout << " read :" << buf << endl;
	}
	else if(readnum == 0){
		std::cout << " read close:" << readnum << endl;
	}
	if(readnum < 0){
		std::cout << " read error: " << errno <<"|" << strerror(errno) << endl;
	}
	return 0;
}

int CSockUtil::sentToServer(int iFd, const std::string sBug)
{
	int sendnum = send(iFd,sBug.c_str(),sBug.size(),0);
	if(sendnum < 0 ){
		cout <<" send eror: " << errno << "|" << strerror(errno) << endl;
		return -1;
	}
	return 0;
}

int CSockUtil::acceptTcpConnect(int iListenfd,std::string &sIP,int &iPort)
{
	int iRet = 0;
	sockaddr_in addr;
	socklen_t len = sizeof(addr);
	for(;;){
		iRet = accept(iListenfd,(struct sockaddr *) &addr,&len);
		if(iRet == -1){
			if(errno == EINTR)
				continue;
			else{
				cout << "accept error: " << errno <<"|" << strerror(errno) << endl;
				return iRet;
			} 
		}	
		break;	
	}

	char cIP[32] = { 0};
	inet_ntop(AF_INET, (void *)&addr.sin_addr,cIP,sizeof(cIP));

	sIP = cIP;
	iPort = ntohs(addr.sin_port);

	return iRet;
}


int  CTcpServer::createTcpServer(const string &sIP,int iPort)
{
	int iRet = -1;
	int iFd = socket(AF_INET,SOCK_STREAM,0);
	if(iFd <= 0){
		cout << "socket error: " << errno << "|" << strerror(errno) <<  endl;
		return iRet;
	}

	sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(iPort);
	inet_pton(AF_INET,sIP.c_str(),&addr.sin_addr);
	int lend = sizeof(addr);
	iRet = ::bind(iFd, (struct sockaddr*) &addr,lend);
	if(iRet != 0){
		cout << "bind error: " << errno << "|" << strerror(errno) << endl;
		return iRet;		
	}

	iRet = listen(iFd, 5);
	if( iRet != 0){
		cout <<"listen error:" << errno <<"|" << strerror(errno) << endl;
		return iRet;
	}

	return iFd;
}


void CTcpServer::acceptConnect(int iListenFd, void *privdata, int iMask)
{
	std::cout << iListenFd <<"|"  << "connect coming " << std::endl;
	string sIP; 
	int iPort;
	int iFd = CSockUtil::acceptTcpConnect(iListenFd,sIP,iPort);

	std::cout << " from ip:" << sIP << "|" << iPort << std::endl;

	CTcpClient client;
	client.iFd = iFd;
	//TODO:
	_eventloop->createClient(iFd);

	//注册客户端
	_eventloop->regClient(iFd);

}

void CTcpServer::readTcpData(int iFd, void *privdata, int iMask)
{
	auto spClient =  _eventloop->getClient(iFd);

	int readnum = read(iFd,spClient->cBuff,MAX_READ_BUFF_SIZE);

	if(readnum > 0 ){
		*(spClient->cBuff + readnum + 1 ) = '\0';
		//TODO:
		//cout << "read:" << spClient->cBuff << endl;

		write(iFd,spClient->cBuff,readnum);

	}
	else if(readnum == 0) {
		//TODO:close
		cout << "close:" << iFd << endl;
		close(iFd);
	}
	else {
		cout << "error:" << iFd << endl;
		//TODO:
	}

}

void CTcpServer::writeTcpData(int iFd,void *privdata, int iMask)
{
	//TODO:
}