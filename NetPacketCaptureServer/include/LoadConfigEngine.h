#ifndef __LOAD_CONFIG_ENGINE_H__
#define __LOAD_CONFIG_ENGINE_H__

#include <cpp_common/BaseThread.h>
#include <sys/inotify.h>

namespace NET_PACKET_CAPTURE_SERVER
{

#define INOTIFY_EVENT_SIZE  (sizeof(struct inotify_event))
#define INOTIFY_BUFLEN      (1024 * (INOTIFY_EVENT_SIZE + 16))

class LoadConfigEngine : public BaseThread
{
public:
	LoadConfigEngine(void);
	~LoadConfigEngine(void);

	virtual int ThreadMain(void *pArg);

	int StopThread(void);

	int LoadScanConfig();
	int LoadModuleConfig();

	int Init();

	int UnInit();

private:
	int           m_fd;
	unsigned char m_buf[INOTIFY_BUFLEN];
	
};

}
#endif