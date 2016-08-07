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

	int WorkInit();

	int WorkUnInit();
	
	int Start();

	void doCapturePacket(unsigned int state);
	
private:
	void InitSignHandler();
	
public:
	ArpCaptureEngine          m_arpCaptureEngine;
	RegisterArpCaptureThread  m_registerArpCaptureThread;
	LoadConfigThread          m_loadConfigThread;
	CapturePacketThread*      m_capturePacketThread;

private:
	bool m_start;
		
};

}

#endif