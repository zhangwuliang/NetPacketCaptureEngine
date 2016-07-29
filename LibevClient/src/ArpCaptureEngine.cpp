#include "ArpCaptureEngine.h"
#include "Definitions.h"
#include "Util.h"
#include <cpp_common/SocketUtils.h>

namespace LIBEVCLIENT
{

void threadExit(struct ev_loop *loop, ev_async *watcher, int revents)
{
	ArpCaptureEngine *pThread = (ArpCaptureEngine*)watcher->data;
	if (pThread)
	{
		pThread->Stop();
	}
}

ArpCaptureEngine::ArpCaptureEngine(void)
{
	m_arpSession.setRegisitState(false);
}

ArpCaptureEngine::~ArpCaptureEngine(void)
{
}

int ArpCaptureEngine::Init()
{
	g_log.Log(INFO, "[%s-%d-%s]:============ArpCaptureEngine INIT================", __FILE__, __LINE__, __FUNCTION__);

	//create event loop
	m_loop = ev_loop_new(EVFLAG_AUTO);
	if (m_loop == NULL)
	{
		return RET_ERROR;
	}

	//create ev_async watcher
	m_task_notify.data = this;
	ev_async_init(&m_task_notify, this->TaskNotifyHandler);
	ev_async_start(m_loop, &m_task_notify);

	m_exit_notify.data = this;
	ev_async_init(&m_exit_notify, threadExit);
	ev_async_start(m_loop, &m_exit_notify);

	//for signal
	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	int res = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	if (res != 0)
	{
		g_log.Log(INFO, "[%s-%d-%s]: block sigpipe error", __FILE__, __LINE__, __FUNCTION__);
	}

	return RET_SUCCESS;

}

int ArpCaptureEngine::UnInit()
{
	if (ev_is_active(&m_task_notify))
	{
		ev_async_stop(m_loop, &m_task_notify);
	}

	if (ev_is_active(&m_exit_notify))
	{
		ev_async_stop(m_loop, &m_exit_notify);
	}

	if (m_loop != NULL)
	{
		ev_loop_destroy(m_loop);
	}

	return RET_SUCCESS;
}

int ArpCaptureEngine::ThreadMain(void* pArg)
{
	g_log.Log(INFO, "[%s-%d-%s]:============ArpCaptureEngine RUN================", __FILE__, __LINE__, __FUNCTION__);
	while(!IsStop())
	{
		ev_run(m_loop, EVRUN_ONCE);
	}

	return 0;
}

int ArpCaptureEngine::StopThread(void)
{
	ev_async_send(m_loop, &m_exit_notify);

	void* pReturnValue = 0;
	int ret = pthread_join(GetThreadID(), &pReturnValue);

	return ret;
}

bool ArpCaptureEngine::NotifyForTask(int fd)
{
	BSLock bsLock(m_stateLock);
	if (m_arpSession.getRegisitState())
	{
		return false;
	}

	m_arpSession.setRegisitState(true);
	m_arpSession.sockfd = fd;
	ev_async_send(m_loop, &m_task_notify);

	return true;
}

int ArpCaptureEngine::ProcessHandShakeTask()
{
	BSLock bsLock(m_stateLock);
	if (!m_arpSession.getRegisitState() || m_arpSession.sockfd <= 0)
	{
		return RET_ERROR;
	}

	SocketUtils::setNonblock(m_arpSession.sockfd);

	m_arpSession.bufPos = 0;
	m_arpSession.ev_loop = m_loop;
	m_arpSession.ev_read.data = &m_arpSession;
	m_arpSession.ev_write.data = &m_arpSession;
	memset(m_arpSession.buf, 0, DATA_LEN);

	ev_io_init(&m_arpSession.ev_read, handleShakeReadCallBack, m_arpSession.sockfd, EV_READ);
	ev_io_init(&m_arpSession.ev_write, handleShakeWriteCallBack, m_arpSession.sockfd, EV_WRITE);

	m_arpSession.Init();
	ev_io_start(m_loop, &m_arpSession.ev_read);

	g_log.Log(DEBUG, "[%s-%d-%s]: libev work, socket:[%d]", __FILE__, __LINE__, __FUNCTION__, m_arpSession.sockfd);

	return RET_SUCCESS;
	
}

void ArpCaptureEngine::sessionClosed(Session* session)
{
	if (ev_is_active(&session->ev_write))
	{
		ev_io_stop(session->ev_loop, &session->ev_write);
	}

	if (ev_is_active(&session->ev_read))
	{
		ev_io_stop(session->ev_loop, &session->ev_read);
	}

	close(session->sockfd);
	session->cleanWorkQueue();
	session->UnInit();

	//TODO
	session->setRegisitState(false);
	Util::SetSocketState(SOCKET_NO_CREATE);
	
}

void ArpCaptureEngine::TaskNotifyHandler(struct ev_loop* loop, ev_async *watcher, int revents)
{
	ArpCaptureEngine *pThis = (ArpCaptureEngine*)watcher->data;

	if (!pThis)
	{
		return;
	}

	pThis->ProcessHandShakeTask();
}

void ArpCaptureEngine::handleShakeReadCallBack(struct ev_loop* loop, ev_io *watcher, int revents)
{
	g_log.Log(INFO, "[%s-%d-%s]:============handleShakeReadCallBack RUN================", __FILE__, __LINE__, __FUNCTION__);
}


void ArpCaptureEngine::handleShakeWriteCallBack(struct ev_loop* loop, ev_io *watcher, int revents)
{
	g_log.Log(INFO, "[%s-%d-%s]:============handleShakeWriteCallBack RUN================", __FILE__, __LINE__, __FUNCTION__);
}

}

