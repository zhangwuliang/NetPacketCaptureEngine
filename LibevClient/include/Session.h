#ifndef __SESSION_H__
#define __SESSION_H__

#include <cpp_common/BaseLock.h>
#include <cpp_common/List.h>

#include <ev++.h>

#include "Header.h"
#include "Definitions.h"

namespace LIBEVCLIENT
{


struct WriteBuffer
{
	char* data;
	ssize_t len;
	ssize_t pos;

	WriteBuffer(const char* bytes, ssize_t nbytes)
	{
		pos = 0;
		len = nbytes;
		data = new char[nbytes+1];
		memcpy(data, bytes, nbytes);
	}

	virtual ~WriteBuffer()
	{
		delete []data;
	}

	char* dpos()
	{
		return data+pos;
	}

	ssize_t nbytes()
	{
		return len-pos;
	}
		
};

struct Session
{
public:
	Session();
	~Session();

public:
	list_node_t             cs_list;
	int                     sockfd;
	char                    buf[4096];
	unsigned int            bufPos;
	bool                    sessionState;
	bool                    socketClosed;
	BaseLock                queueLock;
	std::list<WriteBuffer*> writeQueue;

	struct ev_loop *ev_loop;
	struct ev_io    ev_read;
	struct ev_io    ev_write;
	struct ev_async task_notify;

public:
	void cleanWorkQueue(void);
	int Init(void);
	void UnInit(void);
	static void TaskNotifyHandler(struct ev_loop *loop, ev_async *watcher, int revents);
	void setBufPos(unsigned int len);
	void initSocketState(bool state);
	bool setSocketState(bool state);
	void pushData2WriteQueue(char* data, int len);
	void checkWriteQueueSize();
	bool getRegisitState();
	void setRegisitState(bool state);

private:
	bool     bRegisit;
	BaseLock regisitLock;
	BaseLock closeLock;

private:
	void cleanWorkQueue_(void);
	
	
};


inline void Session::setBufPos(unsigned int len)
{
	if (bufPos == len)
	{
		memset(buf, 0, 4096);
		bufPos = 0;
	}
	else
	{
		memmove(buf, buf+len, bufPos-len);
		bufPos -= len;
		memset(buf+bufPos, 0, 4096-bufPos);
	}
}

}

#endif
