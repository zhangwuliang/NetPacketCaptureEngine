#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/fsuid.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "Util.h"
#include "Definitions.h"
#include "DaemonProcess.h"

using namespace LIBEVSERVER;

namespace LIBEVSERVER
{
	DaemonProcess * g_DaemonProcess = NULL;
}

int main(int argc, char** argv)
{
	//init log
	g_log.Initialize(LOG_CONFIG_FILENAME, "LibevServer");

	
	// Initialize the daemon
	if (RET_ERROR == Util::CreateDaemonProcess())
	{
		g_log.Log(ERROR, "[%s-%d-%s]: Become daemon initialize failed.", __FILE__, __LINE__, __FUNCTION__);
		return RET_ERROR;
	}

	/*
     * There is only one Daemon instance
     */
    if (Util::DaemonIsRunning(PID_FILENAME))
	{
		g_log.Log(ERROR, "[%s-%d-%s]: Main process: An instance daemon is  running, exit", __FILE__, __LINE__, __FUNCTION__);
        return RET_ERROR;
	}
	
	g_DaemonProcess = new DaemonProcess();
	if (!g_DaemonProcess->Start()) 
	{
        g_log.Log(INFO, "[%s-%d-%s]: Daemon normally stopped.", __FILE__, __LINE__, __FUNCTION__);
    }
    else 
	{
        g_log.Log(ERROR, "[%s-%d-%s]: Daemon abnormally stopped.", __FILE__, __LINE__, __FUNCTION__);
    }

	delete g_DaemonProcess;
	g_DaemonProcess = NULL;
	
    unlink(PID_FILENAME);
	
	return 0;
}

