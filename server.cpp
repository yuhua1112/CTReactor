#include "EventLoop.h"


int main(int argc,char *argv[])
{
	EventLoop eloop(1000);

	eloop.startLoop();

	return 0;
}