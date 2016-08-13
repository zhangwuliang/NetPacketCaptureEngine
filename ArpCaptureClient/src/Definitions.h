#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <cpp_common/Log.h>
#include <vector>
#include <sys/time.h>
#include <sys/types.h>

namespace ARP_CAPTURE_CLIENT
{
	typedef unsigned char  u_int8_t;
	typedef unsigned short u_int16_t;
	typedef unsigned int   u_int32_t;

	extern const char* LOG_CONFIG_FILENAME;
    extern const char* LOCK_FILENAME;
    extern const char* PID_FILENAME;
	extern const char* CONFIG_FILENAME;
	
	extern const int RET_SUCCESS;
	extern const int RET_ERROR;

	extern const char* SERVER;
	extern const int   PORT  ;
	
	extern Loger g_log;

	extern const unsigned int MAX_FILE_FD;

	extern const unsigned char g_fixFlag[4];

	extern const unsigned int DATA_LEN;

	extern unsigned int g_packetMaxnum;
	
	typedef enum
	{
		SOCKET_NO_CREATE = 0,
		SOCKET_CREATE,
		SOCKET_SHAKE_HANDS,
		SOCKET_ESTABLISHED
	}SocketState;

	
	typedef enum
	{
		BS_CMD_START				= 0xfedc1000,
		
		BS_CMD_ARPCAPTURE_START 	= 0xfedc2100,
		BS_CMD_ARPCAPTURE_REGIST,
		BS_CMD_REP_REGIST_ARPCAPTURE,
		BS_CMD_ARPCAPTURE_DATA, 	//arp capture to server engine
		BS_CMD_ARPCAPTURE_REP_DATA,
		BS_CMD_ARPCAPTURE_END		= 0xfedc2199,
		
		BS_CMD_END					= 0xfedcf000
		
	} CommandType;

	typedef enum
	{
	    RST_SUCCESS,
	    RST_ERROR,
	    RST_ERR_VERSION,
	    RST_ERR_COMMAND,
	} RepResult;
	
	typedef struct 
	{
		unsigned char	flag[4];
		unsigned int	versionID;
		CommandType 	cmdType;
		RepResult		repStatus;
		unsigned int	reserves;
		unsigned int	length;
	} __attribute__ ((packed)) CmdHead;
	
	extern const unsigned int COMMAND_HEAD_LEN;

	typedef struct 
	{
		unsigned short active;
		char           ip[16];
		char           mac[18];
		unsigned short vlanID;
	}__attribute__ ((packed)) ArpCaptureData;

	const unsigned int ARPCAPTURE_DATA_LEN = sizeof(ArpCaptureData);

	typedef struct 
	{
		CmdHead        cmdHead;
		ArpCaptureData arpCaptureData;
	} __attribute__ ((packed)) CmdArpCaptureData;

	const unsigned int CMD_ARPCAPTURE_DATA_LEN = sizeof(CmdArpCaptureData);

	typedef struct 
	{
		std::vector<std::string> interfaces;
	}MainConfig;

	extern MainConfig g_mainConfig;

	
	#define NIPQUAD(addr) \
	  ((unsigned char *)&addr)[0], \
	  ((unsigned char *)&addr)[1], \
	  ((unsigned char *)&addr)[2], \
	  ((unsigned char *)&addr)[3]

	#define NIPQUAD_FMT "%u.%u.%u.%u"

	//The struct of info should be captured
	typedef struct _dev_info
	{
	    u_int16_t active;
	    u_int8_t  dev_mac[6];
	    u_int32_t dev_ip;
	    u_int16_t vlan;
	    //The current time
	    timeval find_time;
	}__attribute__((packed)) DEV_INFO, *P_DEV_INFO;

}

#endif
