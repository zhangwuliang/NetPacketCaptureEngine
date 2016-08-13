#include "RegisterArpCaptureThread.h"
#include "DaemonProcess.h"

#include "Util.h"

namespace ARP_CAPTURE_CLIENT
{

extern DaemonProcess* g_DaemonProcess;

RegisterArpCaptureThread::RegisterArpCaptureThread()
{
}

RegisterArpCaptureThread::~RegisterArpCaptureThread()
{
}

int RegisterArpCaptureThread::Init()
{
}
	
int RegisterArpCaptureThread::UnInit()
{
}

int RegisterArpCaptureThread::StopThread(void)
{
}

int RegisterArpCaptureThread::ThreadMain(void* pArg)
{
	int count = 1;
	int socketfd = 0;
	SocketState sockState;

	Util::SetSocketState(SOCKET_NO_CREATE);
	
	do
	{
		sockState = Util::GetSocketState();
		g_log.Log(INFO, "[%s-%d-%s]: Regist to Server Engine, count=[%d], state=[%d]...", __FILE__, __LINE__, __FUNCTION__, count++, sockState);
		
		if (SOCKET_NO_CREATE == sockState)
		{
			socketfd = Util::RegisterMode(SERVER, PORT, BS_CMD_ARPCAPTURE_REGIST);
			if (socketfd > 0)
			{
				if (!g_DaemonProcess->m_arpCaptureEngine.NotifyForTask(socketfd))
				{
					close(socketfd);
					socketfd = 0;
					
					g_log.Log(ERROR, "[%s-%d-%s]: Arp capture daemon is registed", __FILE__, __LINE__, __FUNCTION__);
				}
				else
				{
					g_log.Log(INFO, "[%s-%d-%s]: Regist to BS Engine successful!", __FILE__, __LINE__, __FUNCTION__);
				}
			}
			else
			{
				g_log.Log(INFO, "[%s-%d-%s]: Regist to BS Engine failed ...", __FILE__, __LINE__, __FUNCTION__);
			}
		}

		sleep(30);
		
	}while(!IsStop());

	return RET_SUCCESS;
	
}

}