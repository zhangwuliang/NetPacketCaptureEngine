#include "LoadConfigThread.h"
#include "Definitions.h"
#include "json/json.h"

#include <errno.h>
#include <unistd.h>
#include <fstream>

namespace ARP_CAPTURE_CLIENT
{

LoadConfigThread::LoadConfigThread():m_fd(0)
{
}

LoadConfigThread::~LoadConfigThread()
{
}

int LoadConfigThread::Init()
{
	m_fd = inotify_init();
	if (m_fd < 0)
	{
		g_log.Log(ERROR, "[%s-%d-%s]: init failed", __FILE__, __LINE__, __FUNCTION__);
        m_fd = 0;
        return RET_ERROR;
	}

	int wd = inotify_add_watch(m_fd, CONFIG_FILENAME, IN_MODIFY /*IN_ALL_EVENTS*/);

	if (wd < 0) 
	{
        g_log.Log(ERROR, "[%s-%d-%s]: Can't add watch for: %s", __FILE__, __LINE__, __FUNCTION__, CONFIG_FILENAME);
        close(m_fd);
        m_fd = 0;
        return RET_ERROR;
    }

	return RET_SUCCESS;
}

int LoadConfigThread::UnInit()
{
	if (m_fd > 0)
	{
		 close(m_fd);
		 m_fd = 0;
	}
	
	return RET_SUCCESS;
}

int LoadConfigThread::ThreadMain(void *pArg)
{
	ssize_t len                     = 0;
    ssize_t evl                     = 0;
    struct inotify_event * event    = NULL;

    LoadConfigThread* loadConfigThread = static_cast<LoadConfigThread*>(pArg);

    while (!IsStop()) 
	{		
        UnInit();
        if (RET_ERROR == Init())
		{
            sleep(3);
            continue;
        }
		
        len = read(m_fd, m_buf, INOTIFY_BUFLEN);
        g_log.Log(DEBUG, "[%s-%d-%s]:Config file %s is modified , read: [%d]", __FILE__, __LINE__, __FUNCTION__,  CONFIG_FILENAME, len);
        if (len > 0) 
		{
            evl = 0;
            while (evl < len) 
			{
                event = (struct inotify_event*)(&m_buf[evl]);

				sleep(1);
                if ( RET_SUCCESS != loadConfigThread->LoadArpCaptureConfig())
				{
					g_log.Log(ERROR, "[%s-%d-%s]: Load config %s error, please check the config file.",  __FILE__, __LINE__, __FUNCTION__, CONFIG_FILENAME);
				}
     
                evl += INOTIFY_EVENT_SIZE + event->len;
	        }
        }
		else if (0 == len) 
        {
            UnInit();
            g_log.Log(ERROR, "[%s-%d-%s]: read fd is closed, try open again...", __FILE__, __LINE__, __FUNCTION__);
        }
		else
		{
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
        	{
                continue;
        	}
            else
			{	
				//TODO some thing
               UnInit();
               g_log.Log(ERROR, "[%s-%d-%s]: read %s error: %d, try open again...", __FILE__, __LINE__, __FUNCTION__,  CONFIG_FILENAME, errno);
            }
        }
		
		sleep(1);
		
	}

	return RET_SUCCESS;
}



int LoadConfigThread::StopThread(void)
{
	void* pReturnValue = 0;

	return pthread_join(GetThreadID(), &pReturnValue);
}

int LoadConfigThread::LoadArpCaptureConfig()
{
	int iRet        = RET_SUCCESS;
    int             local;
    Json::Reader    reader;
    Json::Value     root;

    std::ifstream ifs;
    ifs.open(CONFIG_FILENAME, std::ios::binary);
    if (!reader.parse(ifs, root, false)) 
	{
        g_log.Log(ERROR, "[%s-%d-%s]: parse file error", __FILE__, __LINE__, __FUNCTION__);
        return RET_ERROR;
    }

	g_log.Log(ERROR, "[%s-%d-%s]: parse file successful", __FILE__, __LINE__, __FUNCTION__);
	
	Json::Value ARP_CAPTURE = root["arp_capture"];

	for(int i = 0; i < ARP_CAPTURE["interfaces"].size(); i++)
	{
		g_mainConfig.interfaces.push_back(ARP_CAPTURE["interfaces"][i].asString());
		g_log.Log(DEBUG, "[%s-%d-%s]: interfaces:[%s]", __FILE__, __LINE__, __FUNCTION__, ARP_CAPTURE["interfaces"][i].asString().c_str());
	}

	ifs.close();
    return iRet;

	
}

}
