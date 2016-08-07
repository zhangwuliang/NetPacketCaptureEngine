#include "ArpCaptureThread.h"
#include "DaemonProcess.h"
#include "ServerCommon.h"
#include "Util.h"
#include "json/json.h"

namespace NET_PACKET_CAPTURE_SERVER
{

extern DaemonProcess* g_DaemonProcess;

static void threadExit(struct ev_loop *loop, ev_async *watcher, int revents)
{
	ArpCaptureThread *pThread = (ArpCaptureThread*)watcher->data;

	if (NULL != pThread)
	{
		pThread->Stop();
	}
}

int pushDataTask(DataTask *task)
{
	int ret = g_DataTask->PushTask(task);
	if (RET_ERROR == ret)
	{
		g_log.Log(ERROR, "[%s-%d-%s]: PushTask failed...", __FILE__, __LINE__, __FUNCTION__);
        return RET_ERROR;
	}

	//notify thread to handle
//	g_DaemonProcess->postInfoCurlThread.NotifyForTask();
    return RET_SUCCESS;
}

int getDataTask(DataTask *task)
{
	return g_DataTask->GetTask(*task);
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
		g_log.Log(INFO, "[%s-%d-%s]: ArpCaptureThread getRegisitState ERROR", __FILE__, __LINE__, __FUNCTION__);
		
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
	CmdArpCaptureData *arpCaptureData = (CmdArpCaptureData*)(session->buf);

	Json::Value root;
	Json::Value arrayObj;
	Json::Value item;

	item["type"]            = "scan";
    item["active"]          = arpCaptureData->arpCaptureData.active;
    item["name"]            = "";
    std::string strIP       = arpCaptureData->arpCaptureData.ip;
    item["ip"]              = strIP;
    std::string strMac      = (char*)arpCaptureData->arpCaptureData.mac;
    item["mac"]             = strMac;
    item["os"]              = "";
    item["device"]          = "";
    item["group"]           = "";
    item["methods"]         = SM_METHOD_ARP;
    item["vlan"]            = arpCaptureData->arpCaptureData.vlanID;

	arrayObj.append(item);
	root["module"]           = "devicescan";
	root["logdata"]          = arrayObj;

	std::string result = root.toStyledString();
	g_log.Log(DEBUG, "[%s-%d-%s]: recv from client:[%s]", __FILE__, __LINE__, __FUNCTION__, result.c_str());
	DataTask task;
	DataQueue *dataTask = g_DataBuffer->AllocateObject();
	if (NULL != dataTask)
	{
		memset(dataTask->data, 0, DATA_LEN);
		memcpy(dataTask->data, result.c_str(), result.size());
		task.data = dataTask;
		pushDataTask(&task);
	}
	else
	{
		g_log.Log(DEBUG, "[%s-%d-%s]: task is limit, do not work", __FILE__, __LINE__, __FUNCTION__);
	}

	session->setBufPos(arpCaptureData->cmdHead.length);
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
	int rev = 0;
	if (EV_ERROR & revents)
	{
		g_log.Log(INFO, "[%s-%d-%s]: Get invalid event", __FILE__, __LINE__, __FUNCTION__);
        return;
	}

	if (revents & EV_READ)
	{
		Session *session = (Session*)watcher->data;
		rev = recv(session->sockfd, session->buf+session->bufPos, DATA_LEN - session->bufPos, 0);
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
                session->setBufPos(1);
                goto again;
            }

			switch(cmdHead->cmdType)
			{
				case BS_CMD_ARPCAPTURE_DATA:
				{
					if (session->bufPos < cmdHead->length)
					{
						return;
					}
					doArpCaptureData(session);
				}
				break;
				default:
				{
					g_log.Log(ERROR, "[%s-%d-%s]: Error Arp Capture Command type: [0x%8x]", __FILE__, __LINE__, __FUNCTION__, cmdHead->cmdType);
                    session->setBufPos(4);	
				}
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
                return;
            else 
			{
				//TODO some thing
                g_log.Log(ERROR, "[%s-%d-%s]: Arp Capture read socket [%d] data error: %d", __FILE__, __LINE__, __FUNCTION__, session->sockfd, errno);
            }
        }

		//socket is closed or error
		ev_io_stop(loop, watcher);
		sessionClosed(session);

		return;
		
	}
}

void ArpCaptureThread::handShakeWriteCallBack(struct ev_loop * loop, ev_io *watcher, int revents)
{
	g_log.Log(INFO, "[%s-%d-%s]: ArpCaptureThread handShakeWriteCallBack", __FILE__, __LINE__, __FUNCTION__);
}


};
