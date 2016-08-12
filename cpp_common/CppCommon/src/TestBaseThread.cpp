#include "BaseThread.h"
#include <unistd.h>

class ChildThread : public BaseThread
{
public:
	int ThreadMain(void* pArg)
	{
		while (!IsStop())
		{
			std::cout << "Child thread is running" << std::endl;
			sleep(2);
		}
	}

	int StopThread()
	{
		void* pReturnValue = 0;
		int ret = pthread_join(GetThreadID(), &pReturnValue);

		return ret; 
	}

};

int main()
{
	BaseThread* child = new ChildThread;

	child->Run();
	while(1)
	{
		sleep(10);
		break;
	}
	std::cout << "Thread Stop" << std::endl;
	child->Stop();

	
	std::cout << "Thread Restart" << std::endl;
	child->SetStopFlag(0);
	child->Run();
	while(1)
	{
		sleep(10);
		break;
	}
	std::cout << "Thread Stop" << std::endl;
	child->Stop();

	delete child;
	child = NULL;
	
	return 0;
}

