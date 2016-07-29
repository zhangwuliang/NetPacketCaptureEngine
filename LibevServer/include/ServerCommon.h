#ifndef __SERVER_COMMON_H__
#define __SERVER_COMMON_H__

namespace LIBEVSERVER
{

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
	
		BS_CMD_IPRANGE_CONFIG,
	
		BS_CMD_ARPCAPTURE_START 	= 0xfedc2100,
		BS_CMD_ARPCAPTURE_REGIST,
		BS_CMD_REP_REGIST_ARPCAPTURE,
		BS_CMD_ARPCAPTURE_CONFIG,
		BS_CMD_ARPCAPTURE_DATA, 	//arp capture to border scan engine
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

	const unsigned int COMMAND_HEAD_LEN = sizeof(CmdHead);
	
	typedef struct 
	{
    	unsigned int        enable;
	} __attribute__ ((packed)) ArpCaptureConfig;

	typedef struct 
	{
	    CmdHead             cmdHead;
	    ArpCaptureConfig    arpCaptureConfig;
	} __attribute__ ((packed)) CmdArpCaptureConfig;

	const unsigned int CMD_ARPCAPTURE_CONFIG_LEN = sizeof(CmdArpCaptureConfig);

}

#endif