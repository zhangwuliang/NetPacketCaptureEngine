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
#include "json/json.h"

#include <cpp_common/BaseLock.h>

#include "Definitions.h"

using namespace LIBEVCLIENT;

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

	static void SetSocketState(SocketState state);

	static SocketState GetSocketState();

	static int RegisterMode(const char* server, const unsigned int port, CommandType commandType);

	static int CreateSocket(const char* server, const unsigned int port);
	
	static void SetCmdHead(CmdHead *cmdHead, CommandType cmdType, unsigned int versionID, RepResult repStatus, unsigned int length);

	static int getJsonInt(Json::Value value, const std::string item);

	static std::string getJsonString(Json::Value value, const std::string item);
		
};

#endif