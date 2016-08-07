#include "DaemonProcess.h"
#include "Definitions.h"

#include <signal.h>
#include <pthread.h>

namespace ARP_CAPTURE_CLIENT
{

static volatile bool g_StopDaemon = false;

static void StopEngine(int s)
{
	g_StopDaemon = true;
}

DaemonProcess::DaemonProcess()
{
}

DaemonProcess::~DaemonProcess()
{
}

void DaemonProcess::InitSignHandler()
{
	struct sigaction sigTerm, sigInt, sigKill;

	sigTerm.sa_handler = StopEngine;
	sigemptyset(&sigTerm.sa_mask);
	sigTerm.sa_flags = 0;
	sigaction(SIGTERM, &sigTerm, 0);

	sigInt.sa_handler = StopEngine;
	sigemptyset(&sigInt.sa_mask);
	sigInt.sa_flags = 0;
	sigaction(SIGINT, &sigInt, 0);

	sigKill.sa_handler = StopEngine;
	sigemptyset(&sigKill.sa_mask);
	sigKill.sa_flags = 0;
	sigaction(SIGKILL, &sigKill, 0);

}	


int DaemonProcess::WorkInit()
{
	this->InitSignHandler();
	
	m_loadConfigThread.LoadArpCaptureConfig();
	m_loadConfigThread.Run();
	
	m_arpCaptureEngine.Init();
	m_arpCaptureEngine.Run();

	m_registerArpCaptureThread.Init();
	m_registerArpCaptureThread.Run();

	int deviceNum = g_mainConfig.interfaces.size();
	m_capturePacketThread = new CapturePacketThread[deviceNum];
	
	for(int i = 0; i < deviceNum; i++)
	{
		m_capturePacketThread[i].Init(g_mainConfig.interfaces[i]);
		m_capturePacketThread[i].Run();		
	}
	
	return RET_SUCCESS;
	
}


int DaemonProcess::WorkUnInit()
{
	if (m_capturePacketThread != NULL)
	{
		delete[] m_capturePacketThread;
		m_capturePacketThread = NULL;
	}

	m_registerArpCaptureThread.StopThread();
	m_registerArpCaptureThread.UnInit();
	
	m_arpCaptureEngine.StopThread();
	m_arpCaptureEngine.UnInit();

	m_loadConfigThread.StopThread();
	m_loadConfigThread.UnInit();

	
	return RET_SUCCESS;
}

int DaemonProcess::Start()
{
	if (WorkInit() != RET_SUCCESS)
	{
		g_log.Log(ERROR, "[%s-%d-%s]: Work init failed!\n", __FILE__, __LINE__, __FUNCTION__);
		return RET_ERROR;
	}
		
	while(!g_StopDaemon)
	{
		sleep(1);
	}
	
	g_log.Log(INFO, "[%s-%d-%s]: Wait for child process exit...", __FILE__, __LINE__, __FUNCTION__);

	WorkUnInit();

	return RET_SUCCESS;
}

void DaemonProcess::doCapturePacket(unsigned int state)
{
	int deviceNum = 0;

	if (state > 0)
	{
		if (!m_start)
		{
			deviceNum = g_mainConfig.interfaces.size();

			for (int i = 0; i<deviceNum; i++)
			{
				if (RET_ERROR == m_capturePacketThread[i].Init(g_mainConfig.interfaces.at(i)))
				{
					continue;
				}
				m_capturePacketThread[i].Run();
			}

			m_start = true;
		}
	}
	else
	{
		if (m_start)
		{
			deviceNum = g_mainConfig.interfaces.size();

			for (int i = 0; i<deviceNum; i++)
			{
				m_capturePacketThread[i].StopThread();
				g_log.Log(INFO, "[%s-%d-%s]: capture process exit: %d...", __FILE__, __LINE__, __FUNCTION__, i);
			}

			for (int i = 0; i<deviceNum; i++)
			{
				m_capturePacketThread[i].UnInit();
			}

			m_start = false;
		}
	}
}

}
