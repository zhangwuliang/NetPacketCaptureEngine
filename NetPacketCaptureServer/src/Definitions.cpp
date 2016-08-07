#include "Definitions.h"

namespace NET_PACKET_CAPTURE_SERVER
{
	
	const char* LOG_CONFIG_FILENAME = "/usr/local/arpcapture/server/log4cxx_net_packet_capture_server.properties";
    const char* PID_FILENAME        = "/usr/local/arpcapture/server/net_packet_capture_server.pid";
	
	const int RET_SUCCESS = 0;
	const int RET_ERROR = -1;
	
	Loger g_log;

	const char* SCAN_CONFIG_FILENAME = "/usr/local/arpcapture/server/net_packet_capture_server.json";
	
	const unsigned char g_fixFlag[4]    = {0x77, 0x88, 'B', 'C'};

	TaskQueue<DataTask>                 *g_DataTask             = NULL;
    ObjectManager<DataQueue>            *g_DataBuffer           = NULL;

	BaseLock                            g_configLock;

	const unsigned int MAX_CONN_TASK_NUM    = 4096;
	const unsigned int MIN_CONN_TASK_NUM    = 1024;

	//const int DATA_LEN = 4096;
	unsigned int g_packetMaxnum  = 1024;
	
}

