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

int DaemonProcess::Start()
{
	this->InitSignHandler();

	arpCaptureEngine.Init();
	arpCaptureEngine.Run();

	registerArpCaptureThread.Run();
	
	while(!g_StopDaemon)
	{
		sleep(1);
	}

	g_log.Log(INFO, "[%s-%d-%s]: Wait for child process exit...", __FILE__, __LINE__, __FUNCTION__);

	return RET_SUCCESS;
}

}
