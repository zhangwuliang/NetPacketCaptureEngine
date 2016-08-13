#ifndef __UTIL_H__
#define __UTIL_H__

#include "Definitions.h"

#include <sys/prctl.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <cstring>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cpp_common/BaseLock.h>

#include "Definitions.h"
#include "ServerCommon.h"

using namespace NET_PACKET_CAPTURE_SERVER;

class Util
{
public:
	
	/*
	 * Run only one instance
	 */
	static int DesktopServerLockfile(int fd);

	static int DaemonIsRunning(const char *pidFileName);

	static int CreateDaemonProcess();

	static void SetSocketCntl(int socketfd);
	
	static void SetCmdHead(CmdHead *cmdHead, CommandType cmdType, unsigned int versionID, RepResult repStatus, unsigned int length);
		
};

#endif