#include "Definitions.h"

namespace ARP_CAPTURE_CLIENT
{
	
	const char* LOG_CONFIG_FILENAME = "/usr/local/arpcapture/client/log4cxx_arp_capture_client.properties";
    const char* PID_FILENAME        = "/usr/local/arpcapture/client/arp_capture_client.pid";
	const char* CONFIG_FILENAME     = "/usr/local/arpcapture/client/arp_capture_client.json";
	
	const int RET_SUCCESS = 0;
	const int RET_ERROR = -1;
	
	Loger g_log;

	const char* SERVER = "127.0.0.1";
	const int   PORT   = 11000;
	
	const unsigned int MAX_FILE_FD          = 2048;

	const unsigned int COMMAND_HEAD_LEN = sizeof(CmdHead);

	const unsigned char g_fixFlag[4]    = {0x77, 0x88, 'B', 'C'};

	const unsigned int DATA_LEN = 4096;

	unsigned int g_packetMaxnum  = 1024;

	MainConfig g_mainConfig;
	
}

