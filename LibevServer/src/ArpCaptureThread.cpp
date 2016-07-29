#include "ArpCaptureThread.h"
#include "ServerCommon.h"
#include "Util.h"

namespace LIBEVSERVER
{

static void threadExit(struct ev_loop *loop, ev_async *watcher, int revents)
{
	ArpCaptureThread *pThread = (ArpCaptureThread*)watcher->data;

	if (NULL != pThread)
	{
		pThread->Stop();
	}
}

ArpCaptureThread::ArpCaptureThread(void):m_loop(NULL)
{
	m_arpSession.setRegisitState(false);
}

ArpCaptureThread::~ArpCaptureThread(void)
{
}

int ArpCaptureThread::Init()
{
	signalHandle();

	// create event loop
	m_loop = ev_loop_new(EVFLAG_AUTO);
	if (m_loop == NULL)
	{
		return -1;
	}

	//create ev_async watcher
	m_task_notify.data = this;
	ev_async_init(&m_task_notify, this->TaskNotifyHandler);
	ev_async_start(m_loop, &m_task_notify);

	m_exit_notify.data = this;
	ev_async_init(&m_exit_notify, threadExit);
	ev_async_start(m_loop, &m_exit_notify);
	
	return RET_SUCCESS;
}

int ArpCaptureThread::UnInit()
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
		m_loop = NULL;
	}
	
	return RET_SUCCESS;
}

int ArpCaptureThread::signalHandle()
{
	//fo signal
	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	if ( pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) != 0 )
	{
		g_log.Log(INFO, "[%s-%d-%s]: block sigpipe error", __FILE__, __LINE__, __FUNCTION__);
		return RET_ERROR;
	}

	return RET_SUCCESS;
}

int ArpCaptureThread::ThreadMain(void *pArg)
{
	while(!IsStop())
	{
		ev_run(m_loop, EVRUN_ONCE);
	}

	return RET_SUCCESS;
}

int ArpCaptureThread::StopThread(void)
{
	ev_async_send(m_loop, &m_exit_notify);

	void *pReturnValue = 0;
	return pthread_join(GetThreadID(), &pReturnValue);
}

bool ArpCaptureThread::NotifyForTask(int fd)
{
	BSLock bsLock(m_stateLock);

	if (m_arpSession.getRegisitState())
	{
		g_log.Log(INFO, "[%s-%d-%s]: Arp Capture daemon is registed, drop old session", __FILE__, __LINE__, __FUNCTION__);
		sessionClosed(&m_arpSession);
	}

	m_arpSession.setRegisitState(true);
	m_arpSession.sockfd = fd;
	ev_async_send(m_loop, &m_task_notify);
	
	return true;
}

int ArpCaptureThread::ProcessHandShakeTask()
{
	BSLock bsLock(m_stateLock);
	if (!m_arpSession.getRegisitState() || m_arpSession.sockfd <= 0)
	{
		return RET_ERROR;
	}

	Util::SetSocketCntl(m_arpSession.sockfd);

	m_arpSession.bufPos = 0;
	m_arpSession.ev_loop = m_loop;
	m_arpSession.ev_read.data = &m_arpSession;
	m_arpSession.ev_write.data = &m_arpSession;
	memset(m_arpSession.buf, 0, DATA_LEN);

	ev_io_init(&m_arpSession.ev_read, handShakeReadCallBack, m_arpSession.sockfd, EV_READ);
	ev_io_init(&m_arpSession.ev_write, handShakeWriteCallBack, m_arpSession.sockfd, EV_WRITE);

	m_arpSession.Init();
	ev_io_start(m_loop, &m_arpSession.ev_read);

	//send data
	this->SendConfigToArpCapture();
	
	return RET_SUCCESS;
}

void ArpCaptureThread::SendConfigToArpCapture()
{
	if (!m_arpSession.getRegisitState())
	{
		g_log.Log(INFO, "[%s-%d-%s]: Arp Capture engine not regist", __FILE__, __LINE__, __FUNCTION__);
        return;
	}

	BSLock bsLock(g_configLock);
	CmdArpCaptureConfig cmdArpCaptureConfig;
	Util::SetCmdHead(&cmdArpCaptureConfig.cmdHead, BS_CMD_ARPCAPTURE_CONFIG, 0, RST_SUCCESS, CMD_ARPCAPTURE_CONFIG_LEN);

	cmdArpCaptureConfig.arpCaptureConfig.enable = 1/*g_arpCaptureConfig.enable*/;
	m_arpSession.pushData2WriteQueue((char*)(&cmdArpCaptureConfig), CMD_ARPCAPTURE_CONFIG_LEN);
}

void ArpCaptureThread::doArpCaptureData(Session *session)
{
}

void ArpCaptureThread::sessionClosed(Session *session)
{
	if (ev_is_active(&session->ev_write))
	{
		ev_io_stop(session->ev_loop, &session->ev_write);
	}

	close(session->sockfd);
	session->cleanWorkQueue();
	session->UnInit();

	session->setRegisitState(false);
}

void ArpCaptureThread::TaskNotifyHandler(struct ev_loop * loop, ev_async *watcher, int revents)
{
	ArpCaptureThread *pThis = (ArpCaptureThread*)watcher->data;

	if (NULL == pThis)
	{
		return;
	}

	pThis->ProcessHandShakeTask();
}

void ArpCaptureThread::handShakeReadCallBack(struct ev_loop * loop, ev_io *watcher, int revents)
{
}

void ArpCaptureThread::handShakeWriteCallBack(struct ev_loop * loop, ev_io *watcher, int revents)
{
}



	
};
