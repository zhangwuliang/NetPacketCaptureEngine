#include "ArpCaptureEngine.h"
#include "Definitions.h"
#include "Util.h"
#include <errno.h>
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
	g_log.Log(INFO, "[%s-%d-%s]: NotifyForTask!", __FILE__, __LINE__, __FUNCTION__);
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
	g_log.Log(INFO, "[%s-%d-%s]: ProcessHandShakeTask!", __FILE__, __LINE__, __FUNCTION__);
	
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

	//send test data
	m_arpSession.pushData2WriteQueue("Hello, World!\n", 13);

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
	g_log.Log(INFO, "[%s-%d-%s]: TaskNotifyHandler!", __FILE__, __LINE__, __FUNCTION__);
	
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

	int rev = 0;
	if (EV_ERROR & revents)
	{
		g_log.Log(INFO, "[%s-%d-%s]: Get invalid event", __FILE__, __LINE__, __FUNCTION__);
        return;
	}

	if (revents & EV_READ)
	{
		Session *session = (Session*)watcher->data;
		rev = recv(session->sockfd, session->buf+session->bufPos, DATA_LEN-session->bufPos, 0);
		if (rev > 0)
		{
			session->bufPos += rev;
again:
			if (session->bufPos < COMMAND_HEAD_LEN)
			{
				return;
			}

			CmdHead *cmdHead = (CmdHead*)session->buf;
			if (strncmp((const char*)cmdHead->flag, (const char*)g_fixFlag, 4))
			{
				 g_log.Log(ERROR, "[%s-%d-%s]: Arp Capture socket [%d] recv len:[%d] send error data head:[0x%x:0x%x:0x%x:0x%x]",
	                                __FILE__, __LINE__, __FUNCTION__, session->sockfd, rev,
	                                cmdHead->flag[0], cmdHead->flag[1], cmdHead->flag[2], cmdHead->flag[3]);

				 //session->setBufState(1);
				 session->setBufPos(1);
				 goto again;
			}

			switch(cmdHead->cmdType)
			{
				case BS_CMD_REP_REGIST_ARPCAPTURE:
				{
					g_log.Log(DEBUG, "[%s-%d-%s]: Command type: BS_CMD_REP_REGIST_ARPCAPTURE", __FILE__, __LINE__, __FUNCTION__);

					CmdArpCaptureData cmdArpCaptureData;
					Util::SetCmdHead(&cmdArpCaptureData.cmdHead, BS_CMD_ARPCAPTURE_DATA, 0, RST_SUCCESS, CMD_ARPCAPTURE_DATA_LEN);
					//WriteBuffer *buf = new WriteBuffer((const char*)(&cmdArpCaptureData), CMD_ARPCAPTURE_DATA_LEN);
					//if (buf != NULL)
					{
						//session->pushBuf2WriteQueue(buf);
						session->pushData2WriteQueue((char*)(&cmdArpCaptureData), CMD_ARPCAPTURE_DATA_LEN);
						if (!ev_is_active(&session->ev_write))
						{
							ev_io_start(session->ev_loop, &session->ev_write);
						}
					}
					//session->setBufState(COMMAND_HEAD_LEN);
					session->setBufPos(COMMAND_HEAD_LEN);
				
				}
				break;
				case BS_CMD_ARPCAPTURE_REP_DATA:
				{
					g_log.Log(DEBUG, "[%s-%d-%s]: Command type: BS_CMD_ARPCAPTURE_REP_DATA", __FILE__, __LINE__, __FUNCTION__);
					//session->setBufState(COMMAND_HEAD_LEN);
					session->setBufPos(COMMAND_HEAD_LEN);
				}
				break;

			default:
				g_log.Log(ERROR, "[%s-%d-%s]: Error Arp Capture Command type: [0x%8x]", __FILE__, __LINE__, __FUNCTION__, cmdHead->cmdType);
	            //session->setBufState(4);
	            session->setBufPos(4);
	            break;
			}

			goto again;
		}
		else if (rev == 0)
		{
			g_log.Log(ERROR, "[%s-%d-%s]: Arp Capture read socket is closed: %d", __FILE__, __LINE__, __FUNCTION__, session->sockfd);
		}
		else
		{
			if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
		 	{
				return;
			}
			else
			{
				g_log.Log(ERROR, "[%s-%d-%s]: Arp Capture read socket [%d] data error: %d", __FILE__, __LINE__, __FUNCTION__, session->sockfd, errno);
			}
		}

		ev_io_stop(loop, watcher);
		sessionClosed(session);
		return;
	}
}


void ArpCaptureEngine::handleShakeWriteCallBack(struct ev_loop* loop, ev_io *watcher, int revents)
{
	g_log.Log(INFO, "[%s-%d-%s]:============handleShakeWriteCallBack RUN================", __FILE__, __LINE__, __FUNCTION__);

	if (EV_ERROR & revents)
	{
		g_log.Log(ERROR, "[%s-%d-%s]: Get invalid event", __FILE__, __LINE__, __FUNCTION__);
        return;
	}

	if (EV_WRITE & revents)
	{
		Session *session = (Session*)watcher->data;

		BSLock bsLock(session->queueLock);
		WriteBuffer *buf = NULL;

write_again:
		if (session->writeQueue.empty())
		{
			g_log.Log(INFO, "[%s-%d-%s]:============handleShakeWriteCallBack empty================", __FILE__, __LINE__, __FUNCTION__);
			ev_io_stop(loop, watcher);
			return;
		}

		buf = session->writeQueue.front();
		g_log.Log(ERROR, "[%s-%d-%s]: Before Send", __FILE__, __LINE__, __FUNCTION__);
		ssize_t written = send(watcher->fd, "Hello, World!", 13, 0);
		g_log.Log(ERROR, "[%s-%d-%s]:  Send, fd=%d, write=%d\n", __FILE__, __LINE__, __FUNCTION__, watcher->fd, written);
		//ssize_t written = send(watcher->fd, buf->dpos(), buf->nbytes(), 0);
		if (written > 0)
		{
			buf->pos += written;
			if (buf->nbytes() == 0)
			{
				session->writeQueue.pop_front();
				delete buf;
				buf = NULL;

				goto write_again;
			}
			
		}
		else if (written == 0)
		{
			g_log.Log(ERROR, "[%s-%d-%s]: Arp Capture write socket is closed: %d", __FILE__, __LINE__, __FUNCTION__, watcher->fd);
		}
		else
		{
			if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
			{
				return;
			}
			else
			{
				 g_log.Log(ERROR, "[%s-%d-%s]: Arp Capture write socket [%d] data error: %d", __FILE__, __LINE__, __FUNCTION__, watcher->fd, errno);
			}
		}

		ev_io_stop(loop, watcher);
		sessionClosed(session);
		
	}
}

}

