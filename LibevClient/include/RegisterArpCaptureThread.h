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

	virtual int ThreadMain(void* pArg);
};

}

#endif

