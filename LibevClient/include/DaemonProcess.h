#ifndef __DAEMON_PROCESS_H__
#define __DAEMON_PROCESS_H__

#include "ArpCaptureEngine.h"
#include "RegisterArpCaptureThread.h"

namespace LIBEVCLIENT
{

class DaemonProcess
{
public:
	DaemonProcess();
	~DaemonProcess();

	int Start();

private:
	void InitSignHandler();
	
public:
	ArpCaptureEngine    arpCaptureEngine;
	RegisterArpCaptureThread registerArpCaptureThread;
};

}

#endif