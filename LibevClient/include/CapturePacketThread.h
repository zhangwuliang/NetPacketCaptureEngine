#ifndef __CAPTURE_PACKET_THREAD_H__
#define __CAPTURE_PACKET_THREAD_H__

#include <cpp_common/BaseThread.h>
#include <pcap.h>
#include <pcap-bpf.h>

#include "ev.h"

namespace ARP_CAPTURE_CLIENT
{

class CapturePacketThread : public BaseThread
{
public:
	CapturePacketThread(void);

	~CapturePacketThread(void);

	int Init(const std::string& interfaces);

	int UnInit();

	virtual int ThreadMain(void* pArg);

	int StopThread(void);

private:
	/**
	* store the interfaces opend by pcap which should be captured
	* eth0, eth1, eth2 or any
	*/
	bool                    m_capture;
	pcap_t                  *m_PcapHandle;
    struct ev_loop          *m_loop;
    struct ev_async         m_exit_notify;
	
};

}

#endif
