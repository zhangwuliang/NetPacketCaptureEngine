#ifndef __DAEMON_PROCESS_H__
#define __DAEMON_PROCESS_H__

#include "ArpCaptureEngine.h"
#include "RegisterArpCaptureThread.h"
#include "CapturePacketThread.h"
#include "LoadConfigThread.h"

namespace ARP_CAPTURE_CLIENT
{

class DaemonProcess
{
public:
	DaemonProcess();
	~DaemonProcess();
	
	int Start();
	
private:
	void InitSignHandler();
	
	int WorkInit();

	int WorkUnInit();
	
public:
	ArpCaptureEngine          m_arpCaptureEngine;
	RegisterArpCaptureThread  m_registerArpCaptureThread;
	LoadConfigThread          m_loadConfigThread;
	CapturePacketThread*      m_capturePacketThread;

private:
	int m_deviceNum;
	
};

}

#endif