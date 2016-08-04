#include "DaemonProcess.h"
#include "Definitions.h"

#include <signal.h>
#include <pthread.h>

namespace LIBEVCLIENT
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
	
	m_arpCaptureEngine.Init();
	m_arpCaptureEngine.Run();

	m_registerArpCaptureThread.Init();
	m_registerArpCaptureThread.Run();

	m_loadConfigThread.LoadArpCaptureConfig();
	m_loadConfigThread.Run();

	/*m_capturePacketThread.Init();
	m_capturePacketThread.Run();*/
	
	return RET_SUCCESS;
}


int DaemonProcess::WorkUnInit()
{
	m_arpCaptureEngine.StopThread();
	m_arpCaptureEngine.UnInit();

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

	//WorkUnInit();
	
	g_log.Log(INFO, "[%s-%d-%s]: Wait for child process exit...", __FILE__, __LINE__, __FUNCTION__);

	return RET_SUCCESS;
}

}
