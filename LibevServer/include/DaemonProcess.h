#ifndef __DAEMON_PROCESS_H__
#define __DAEMON_PROCESS_H__

#include "ArpCaptureThread.h"
#include "ServerEngine.h"
#include "LoadConfigEngine.h"

namespace LIBEVSERVER
{

class  DaemonProcess
{
public:
	DaemonProcess();
	~DaemonProcess();

	int Start();
	int Init();
	int UnInit();

private:
	int signalHandle();
	void freeMemory();

public:

	ArpCaptureThread    arpCaptureThread;
    /*HostScanThread    hostScanThread;
    SwitchScanThread    switchScanThread;*/
    
    ServerEngine    serverEngine;

private:
	LoadConfigEngine loadConfigEngine;
	
};

}

#endif