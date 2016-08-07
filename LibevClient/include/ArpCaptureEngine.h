#ifndef __ARP_CAPTURE_ENGINE_H__
#define __ARP_CAPTURE_ENGINE_H__

#include <cpp_common/BaseThread.h>
#include <cpp_common/BaseLock.h>
#include "Session.h"

#include <ev++.h>

namespace ARP_CAPTURE_CLIENT
{

class ArpCaptureEngine : public BaseThread
{
public:
	ArpCaptureEngine(void);
	~ArpCaptureEngine(void);

	int Init();
	int UnInit();

	virtual int ThreadMain(void* pArg);

	int StopThread(void);

	bool NotifyForTask(int fd);

	int ProcessHandShakeTask();

	static void sessionClosed(Session* session);

	static void TaskNotifyHandler(struct ev_loop* loop, ev_async *watcher, int revents);
	static void handleShakeReadCallBack(struct ev_loop* loop, ev_io *watcher, int revents);
	static void handleShakeWriteCallBack(struct ev_loop* loop, ev_io *watcher, int revents);

public:
	Session  m_arpSession;
	
private:
	BaseLock m_stateLock;
	
	struct ev_loop *m_loop;
	struct ev_async m_task_notify;
	struct ev_async m_exit_notify;
	
};

}

#endif