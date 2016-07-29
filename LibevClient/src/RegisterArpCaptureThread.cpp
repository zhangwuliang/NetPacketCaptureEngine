#include "RegisterArpCaptureThread.h"

#include "DaemonProcess.h"

#include "Util.h"

namespace LIBEVCLIENT
{

extern DaemonProcess* g_DaemonProcess;

RegisterArpCaptureThread::RegisterArpCaptureThread()
{
}

RegisterArpCaptureThread::~RegisterArpCaptureThread()
{
}

int RegisterArpCaptureThread::ThreadMain(void* pArg)
{
	g_log.Log(INFO, "[%s-%d-%s]: Arp Capture thread register", __FILE__, __LINE__, __FUNCTION__);

	int count = 1;
	int socketfd = 0;
	SocketState sockState;

	Util::SetSocketState(SOCKET_NO_CREATE);

	do
	{
		sockState = Util::GetSocketState();
		if (SOCKET_NO_CREATE == sockState)
		{
			g_log.Log(INFO, "[%s-%d-%s]: Regist to BS Engine [%d]...", __FILE__, __LINE__, __FUNCTION__, count++);
			socketfd = Util::RegisterMode(SERVER, PORT, BS_CMD_ARPCAPTURE_REGIST);
			if (socketfd > 0)
			{
				if (!g_DaemonProcess->arpCaptureEngine.NotifyForTask(socketfd))
				{
					close(socketfd);
					g_log.Log(ERROR, "[%s-%d-%s]: Arp capture daemon test is registed", __FILE__, __LINE__, __FUNCTION__);
				}
			}
			else
			{
				g_log.Log(INFO, "[%s-%d-%s]: Regist to BS Engine failed ...", __FILE__, __LINE__, __FUNCTION__);
			}
		}

		sleep(5);
		
	}while(1);

	return 0;
	
}

}