#ifndef __BASE_THREAD_H__
#define __BASE_THREAD_H__

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
using namespace std;

class BaseThread
{
public:
	BaseThread():m_stop(0)
	{
        m_ThreadID = 0;
        pthread_attr_init(&m_Attr);
    }

	virtual ~BaseThread() 
	{
	}

	int SetStackSize(size_t size)
	{
        return pthread_attr_setstacksize(&m_Attr, size);
    }

	static void* ThreadRoutine(void* pArg)
	{
		BaseThread* pThread = (BaseThread*)pArg;
		pThread->ThreadMain(pArg);

		return NULL;
	}

	int Run()
	{
		int ret = pthread_create(&m_ThreadID, &m_Attr, ThreadRoutine, this);

		if(ret) 
		{
            m_ThreadID = 0;
        }

		ret |= pthread_attr_destroy(&m_Attr);

		return ret;
	}

	 void Stop()
	 {
        m_stop = 1;
     }
	 
    int IsStop() 
	{
		return m_stop; 
	}
	
    pthread_t GetThreadID() 
	{
		return m_ThreadID; 
	}
	
    void SetStopFlag(int stop)
	{ 
		m_stop = stop; 
	}

	
	virtual int ThreadMain(void* pArg) = 0;

private:
    volatile int m_stop;
    pthread_t m_ThreadID;
    pthread_attr_t m_Attr;
};

#endif