#ifndef __REGISTER_ARP_CAPTURE_THREAD_H__
#define __REGISTER_ARP_CAPTURE_THREAD_H__

#include <cpp_common/BaseThread.h>

#include "Definitions.h"

namespace LIBEVCLIENT
{

class RegisterArpCaptureThread : public BaseThread
{
public:
	RegisterArpCaptureThread();
	~RegisterArpCaptureThread();

	int Init();
	
	int UnInit();

	virtual int ThreadMain(void* pArg);

	int StopThread(void);
};

}

#endif

