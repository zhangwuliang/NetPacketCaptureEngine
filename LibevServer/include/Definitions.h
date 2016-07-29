#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <cpp_common/Log.h>
#include <cpp_common/BaseLock.h>
#include <cpp_common/TaskQueue.hpp>
#include <cpp_common/ObjectManager.hpp>

namespace LIBEVSERVER
{
	extern const char* LOG_CONFIG_FILENAME;
    extern const char* LOCK_FILENAME;
    extern const char* PID_FILENAME;
	
	extern const int RET_SUCCESS;
	extern const int RET_ERROR;

	extern Loger g_log;

	extern const char* SCAN_CONFIG_FILENAME;
	
	extern const unsigned char g_fixFlag[4];

	extern const unsigned int MAX_CONN_TASK_NUM;
	extern const unsigned int MIN_CONN_TASK_NUM;

	const int DATA_LEN = 4096;

	extern BaseLock g_configLock;
	
	typedef struct 
	{
		list_node_t cs_list;
		char        data[DATA_LEN];
	}DataQueue;

	typedef struct
	{
		DataQueue *data;
	}DataTask;

	extern TaskQueue<DataTask>      *g_DataTask;
	extern ObjectManager<DataQueue> *g_DataBuffer;

	extern unsigned int g_packetMaxnum;
}

#endif

