#include "Definitions.h"

namespace LIBEVCLIENT
{
	
	const char* LOG_CONFIG_FILENAME = "./log4cxx_libevclient.properties";
    const char* PID_FILENAME        = "/var/run/libevclient.pid";
	
	const int RET_SUCCESS = 0;
	const int RET_ERROR = -1;
	
	Loger g_log;

	const char* SERVER = "127.0.0.l";
	const int   PORT   = 11000;
	
	const unsigned int MAX_FILE_FD          = 2048;

	const unsigned int COMMAND_HEAD_LEN = sizeof(CmdHead);

	const unsigned char g_fixFlag[4]    = {0x77, 0x88, 'B', 'C'};

	const unsigned int DATA_LEN = 4096;

	unsigned int g_packetMaxnum  = 1024;
	
}

