#ifndef __LOAD_CONFIG_THREAD_H__
#define __LOAD_CONFIG_THREAD_H__

#include <cpp_common/BaseThread.h>
#include <sys/inotify.h>

namespace ARP_CAPTURE_CLIENT
{

#define INOTIFY_EVENT_SIZE  (sizeof(struct inotify_event))
#define INOTIFY_BUFLEN      (1024 * (INOTIFY_EVENT_SIZE + 16))

class LoadConfigThread : public BaseThread
{
public:
	LoadConfigThread(void);

	~LoadConfigThread(void);

	int StopThread(void);

	int LoadArpCaptureConfig();

	int Init();

	int UnInit();

	virtual int ThreadMain(void* pArg);

private:
	int           m_fd;
	unsigned char m_buf[INOTIFY_BUFLEN];
	
};

}

#endif