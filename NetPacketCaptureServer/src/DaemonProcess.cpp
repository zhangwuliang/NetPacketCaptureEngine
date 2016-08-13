#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <vector>
#include <sys/stat.h>

#include "DaemonProcess.h"
#include "Definitions.h"

namespace NET_PACKET_CAPTURE_SERVER
{

extern DaemonProcess* g_DaemonProcess;

static volatile bool g_StopDaemon = false;

static void StopEngine(int)
{
    //TODO:something will rewrite
    g_log.Log(INFO, "[%s-%d-%s]: Get system signal...", __FILE__, __LINE__, __FUNCTION__);
    g_DaemonProcess->serverEngine.stop();
    g_StopDaemon = true;
}


DaemonProcess::DaemonProcess()
{
}

DaemonProcess::~DaemonProcess()
{
}

int DaemonProcess::Start()
{
	signalHandle();	

	//Load config
	if (RET_SUCCESS != loadConfigEngine.LoadServerConfig())
	{
		g_log.Log(ERROR, "[%s-%d-%s]: Load server config error", __FILE__, __LINE__, __FUNCTION__);
		return RET_ERROR;
	}

	if (RET_SUCCESS != Init())
	{
		g_log.Log(ERROR, "[%s-%d-%s]: DaemonProcess init error", __FILE__, __LINE__, __FUNCTION__);
		return RET_ERROR;
	}

	//init ServerEngine
	if (!serverEngine.start())
	{
		g_log.Log(ERROR, "[%s-%d-%s]: Starting  server engine process Faild...", __FILE__, __LINE__, __FUNCTION__);
        return RET_ERROR;
	}

	g_log.Log(INFO, "[%s-%d-%s]: Start server engine daemon success", __FILE__, __LINE__, __FUNCTION__);

	serverEngine.loop();

	while (!g_StopDaemon)
	{
		sleep(1);
	}

	//Daemon stop
    g_log.Log(INFO, "[%s-%d-%s]: Wait for child process exit...", __FILE__, __LINE__, __FUNCTION__);
	
	return RET_SUCCESS;
}

int DaemonProcess::Init()
{
	g_DataTask = new TaskQueue<DataTask>(MAX_CONN_TASK_NUM);
	if (NULL == g_DataTask)
	{
		goto new_err;
	}

	g_DataBuffer = new ObjectManager<DataQueue>(MAX_CONN_TASK_NUM);
	if (NULL == g_DataBuffer)
	{
		goto new_err;
	}
	
	arpCaptureThread.Init();
	arpCaptureThread.Run();

	loadConfigEngine.LoadServerConfig();
	loadConfigEngine.Run();

	return RET_SUCCESS;

new_err:
	freeMemory();
	g_log.Log(ERROR, "[%s-%d-%s]: new memory failed...", __FILE__, __LINE__, __FUNCTION__);
    return RET_ERROR;
	
}


int DaemonProcess::UnInit()
{
	arpCaptureThread.StopThread();
	arpCaptureThread.UnInit();

	/*  hostScanThread.StopThread();
    hostScanThread.UnInit();

    switchScanThread.StopThread();
    switchScanThread.UnInit();

    postInfoCurlThread.StopThread();
    postInfoCurlThread.UnInit();*/

    freeMemory();

	return RET_SUCCESS;
}

void DaemonProcess::freeMemory()
{
	if (NULL != g_DataTask)
	{
		delete g_DataTask;
		g_DataTask = NULL;
	}

	if (NULL != g_DataBuffer)
	{
		delete g_DataBuffer;
		g_DataBuffer = NULL;
	}
}

int DaemonProcess::signalHandle()
{
	struct sigaction actTerm, actInt, actKill;

	actTerm.sa_handler = StopEngine;
    sigemptyset(&actTerm.sa_mask);
    actTerm.sa_flags = 0;
    sigaction(SIGTERM, &actTerm, 0);

    actInt.sa_handler = StopEngine;
    sigemptyset(&actInt.sa_mask);
    actInt.sa_flags = 0;
    sigaction(SIGINT, &actInt, 0);

    actKill.sa_handler = StopEngine;
    sigemptyset(&actKill.sa_mask);
    actKill.sa_flags = 0;
    sigaction(SIGKILL, &actKill, 0);

	return RET_SUCCESS;
}

}


