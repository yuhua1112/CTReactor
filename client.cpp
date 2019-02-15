#include "EventLoop.h"
#include <chrono>
#include <thread>
#include <vector>

using namespace std;
using namespace chrono;

int testSend()
{
	int fd = CSockUtil::connectTcpServer("10.64.50.189",28510);
	if(fd <= 0){
		return -1;
	}

	if( CSockUtil::setReuseAddr(fd) < 0){
		return -1;
	}

	auto start = system_clock::now();

	for( int i = 0 ;i < 50000; i++){
		CSockUtil::sentToServer(fd,"hello reactor");
		CSockUtil::readData(fd);
	}

	auto end = system_clock::now();

	auto dura = duration_cast<milliseconds>(end - start); 

	std::cout <<"task end use:" << dura.count() << endl; 

	close(fd);

	std::this_thread::sleep_for(std::chrono::seconds(3));

	return 0;
}


int main(int argc,char *argv[])
{
	vector<std::thread> vWorker;

	for(int i = 0 ; i  < 5; i++){
		vWorker.emplace_back( []{
			testSend();
		});

	}

	for(auto &t : vWorker){
		t.join();
	}
	std::this_thread::sleep_for(std::chrono::seconds(60));
	return 0;
}