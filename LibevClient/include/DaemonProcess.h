#ifndef __DAEMON_PROCESS_H__
#define __DAEMON_PROCESS_H__

#include "ArpCaptureEngine.h"
#include "RegisterArpCaptureThread.h"
#include "CapturePacketThread.h"
#include "LoadConfigThread.h"

namespace LIBEVCLIENT
{

class DaemonProcess
{
public:
	DaemonProcess();
	~DaemonProcess();

	int WorkInit();

	int WorkUnInit();
	
	int Start();

private:
	void InitSignHandler();
	
public:
	ArpCaptureEngine         m_arpCaptureEngine;
	RegisterArpCaptureThread m_registerArpCaptureThread;
	LoadConfigThread         m_loadConfigThread;
	CapturePacketThread*     m_capturePacketThread;
};

}

#endif