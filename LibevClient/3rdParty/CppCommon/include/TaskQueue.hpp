#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include "BaseLock.h"

template <class Task>
class TaskQueue
{
public:
    TaskQueue(unsigned int maxTaskNum);
    ~TaskQueue();

    int GetTask(Task& task);
    int PushTask(const Task* pTask);

    int GetTasks(Task* pTask);
    int PushTasks(const Task* pTask, int cnt);

    unsigned int GetCapacity()
	{
        BSLock lock(m_lock);
        return m_maxTaskNum - m_taskNum;
    }

private:
    unsigned int GetTaskNum();

    unsigned int    m_in;
    unsigned int    m_out;
    unsigned int    m_taskNum;
    BaseLock        m_lock;
    Task*           m_taskArray;
    unsigned int    m_maxTaskNum;
	
};



template <class Task>
TaskQueue<Task>::TaskQueue(unsigned int maxTaskNum):
    m_in(0), m_out(0), m_taskNum(0), m_maxTaskNum(maxTaskNum)
{
    m_taskArray = new Task[m_maxTaskNum];
    if(!m_taskArray) 
	{
        throw "failed to creat task";
    }
}

template <class Task>
TaskQueue<Task>::~TaskQueue()
{
    if(m_taskArray) 
	{
        delete[] m_taskArray;
        m_taskArray = NULL;
    }
}

template <class Task>
int TaskQueue<Task>::GetTask(Task& task)
{
    BSLock lock(m_lock);
    if(GetTaskNum() <= 0) 
	{
        return -1;
    }
	else 
	{
        --m_taskNum;
        task = m_taskArray[m_out++];
        m_out = m_out % m_maxTaskNum;
        return 0;
    }
}

template <class Task>
int TaskQueue<Task>::PushTask(const Task* pTask)
{
    BSLock lock(m_lock);
    if(GetTaskNum() == m_maxTaskNum) 
	{
        return -1;
    }
	else 
	{
        ++m_taskNum;
        m_taskArray[m_in++] = *pTask;
        m_in = m_in % m_maxTaskNum;
        return 0;
    }
}

template <class Task>
unsigned int TaskQueue<Task>::GetTaskNum()
{
    //private: no lock here
    return m_taskNum;
}

template <class Task>
int TaskQueue<Task>::GetTasks(Task* pTask)
{
   BSLock lock(m_lock);
   int cnt = m_taskNum;
   if(cnt <= 0)
       return 0;

   for(int i = 0; i < cnt; ++i) 
   {
       --m_taskNum;
       pTask[i] = m_taskArray[m_out++];
       m_out = m_out % m_maxTaskNum;
   }
   return cnt;
}

template <class Task>
int TaskQueue<Task>::PushTasks(const Task* pTask, int cnt)
{
    BSLock lock(m_lock);
    int i = 0;
    for (; i < cnt && GetTaskNum() < m_maxTaskNum;++i) 
	{
        ++m_taskNum;
        m_taskArray[m_in++] = pTask[i];
        m_in = m_in % m_maxTaskNum;
    }
	
    return i;
}


#endif
