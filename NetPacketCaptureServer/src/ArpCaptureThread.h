#ifndef __ARP_CAPTURE_THREAD_H__
#define __ARP_CAPTURE_THREAD_H__

#include "Session.h"
#include "Definitions.h"

#include <cpp_common/BaseThread.h>
#include <cpp_common/BaseLock.h>

namespace NET_PACKET_CAPTURE_SERVER
{

class ArpCaptureThread : public BaseThread
{
public:
	ArpCaptureThread(void);
	~ArpCaptureThread(void);

	int Init();
	int UnInit();

	virtual int ThreadMain(void *pArg);

	int StopThread(void);
	bool NotifyForTask(int fd);
	int ProcessHandShakeTask();
	void SendConfigToArpCapture();

	static void doArpCaptureData(Session *session);
	static void sessionClosed(Session *session);
    static void TaskNotifyHandler(struct ev_loop * loop, ev_async *watcher, int revents);
    static void handShakeReadCallBack(struct ev_loop * loop, ev_io *watcher, int revents);
    static void handShakeWriteCallBack(struct ev_loop * loop, ev_io *watcher, int revents);


private:
	int signalHandle();
	
private:
	BaseLock m_stateLock;
	Session  m_arpSession;

	//event loop for handshake query and async for task notification
    struct ev_loop *m_loop;
    struct ev_async m_task_notify;
    struct ev_async m_exit_notify;
};

}

#endif