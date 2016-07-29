#include "Session.h"

namespace LIBEVCLIENT
{

Session::Session(): socketClosed(false), bRegisit(false), ev_loop(NULL), bufPos(0)
{
    
}

Session::~Session()
{
}

int Session::Init()
{
	//create event loop
	if (ev_loop == NULL)
	{
		return RET_ERROR;
	}

	//create ev_async watcher
	task_notify.data = this;
	ev_async_init(&task_notify, this->TaskNotifyHandler);
	ev_async_start(ev_loop, &task_notify);

	cleanWorkQueue();

	return RET_SUCCESS;
}

void Session::TaskNotifyHandler(struct ev_loop *loop, ev_async *watcher, int revents)
{
	Session *pThis = (Session*)watcher->data;

	if (!pThis)
	{
		return;
	}

	if (!ev_is_active(&pThis->ev_write))
	{
		ev_io_start(pThis->ev_loop, &pThis->ev_write);
	}
}

void Session::UnInit()
{
	if (ev_loop == NULL)
	{
		return;
	}

	if (ev_is_active(&task_notify))
	{
		ev_async_stop(ev_loop, &task_notify);
	}
}

void Session::cleanWorkQueue_()
{
	std::list<WriteBuffer*>::iterator iter;
	for(iter = writeQueue.begin(); iter != writeQueue.end(); )
	{
		WriteBuffer *buf = *iter;
		writeQueue.erase(iter++);
		delete buf;
	}
}

void Session::cleanWorkQueue()
{
	BSLock bsLock(queueLock);
	cleanWorkQueue_();
}

bool Session::setSocketState(bool state)
{
	BSLock  vpLock(closeLock);
    if (socketClosed)
    {
        return true;
    }
    socketClosed = state;
    return false;
}

void Session::initSocketState(bool state)
{
    BSLock  bsLock(closeLock);
    socketClosed = state;
}

void Session::pushData2WriteQueue(char * data,int len)
{
	static int countPush = 0;

	if (NULL != data && bRegisit)
	{
		WriteBuffer *buf = new WriteBuffer((const char*)data, len);
		if (buf != NULL)
		{
			BSLock bsLock(queueLock);
			writeQueue.push_back(buf);
		}

		ev_async_send(ev_loop, &task_notify);
	}
}

void Session::checkWriteQueueSize()
{
	BSLock bsLock(queueLock);
	if (g_packetMaxnum <= writeQueue.size())
	{
		sessionState = false;
	}
}

void Session::setRegisitState(bool state)
{
    BSLock  bsLock(regisitLock);
    bRegisit = state;
}

bool Session::getRegisitState()
{
    BSLock bsLock(regisitLock);
    return bRegisit;
}


}
