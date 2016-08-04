#ifndef __CAPTURE_PACKET_THREAD_H__
#define __CAPTURE_PACKET_THREAD_H__

#include <cpp_common/BaseThread.h>


namespace LIBEVCLIENT
{

class CapturePacketThread : public BaseThread
{
public:
	CapturePacketThread(void);

	~CapturePacketThread(void);

	int Init();

	int UnInit();

	virtual int ThreadMain(void* pArg);

	int StopThread(void);
	
};

}

#endif
