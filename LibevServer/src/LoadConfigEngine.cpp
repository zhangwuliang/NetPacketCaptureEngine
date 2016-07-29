#include "LoadConfigEngine.h"
#include "Definitions.h"
#include "DaemonProcess.h"

#include <errno.h>

namespace LIBEVSERVER
{

extern DaemonProcess g_DaemonProcess;

LoadConfigEngine::LoadConfigEngine(void) :m_fd(0)
{
}

LoadConfigEngine::~LoadConfigEngine(void)
{
}

int LoadConfigEngine::ThreadMain(void *pArg)
{
	ssize_t len                     = 0;
    ssize_t evl                     = 0;
    struct inotify_event * event    = NULL;

    LoadConfigEngine* loadConfigEngine = static_cast<LoadConfigEngine*>(pArg);

    while (!IsStop()) 
	{		
        UnInit();
        if (RET_ERROR == Init())
		{
            sleep(3);
            continue;
        }
		
        len = read(m_fd, m_buf, INOTIFY_BUFLEN);
        g_log.Log(DEBUG, "[%s-%d-%s]:Config file %s is modified , read: [%d]", __FILE__, __LINE__, __FUNCTION__,  SCAN_CONFIG_FILENAME, len);
        if (len > 0) 
		{
            evl = 0;
            while (evl < len) 
			{
                event = (struct inotify_event*)(&m_buf[evl]);

				sleep(1);
                if ( RET_SUCCESS != loadConfigEngine->LoadScanConfig())
				{
					g_log.Log(ERROR, "[%s-%d-%s]: Load config %s error, please check the config file.",  __FILE__, __LINE__, __FUNCTION__, SCAN_CONFIG_FILENAME);
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
               g_log.Log(ERROR, "[%s-%d-%s]: read %s error: %d, try open again...", __FILE__, __LINE__, __FUNCTION__,  SCAN_CONFIG_FILENAME, errno);
            }
        }
		
		sleep(1);
		
	}

	return RET_SUCCESS;
}

int LoadConfigEngine::StopThread(void)
{
	void *pReturnValue = 0;

	return pthread_join(GetThreadID(), &pReturnValue);
}

int LoadConfigEngine::LoadScanConfig()
{
	//read config
	return RET_SUCCESS;
}

int LoadConfigEngine::LoadModuleConfig()
{
	//read config
	return RET_SUCCESS;
}

int LoadConfigEngine::Init()
{
	m_fd = inotify_init();
	if (m_fd < 0)
	{
		g_log.Log(ERROR, "[%s-%d-%s]: init failed", __FILE__, __LINE__, __FUNCTION__);
        m_fd = 0;
        return RET_ERROR;
	}

	int wd = inotify_add_watch(m_fd, SCAN_CONFIG_FILENAME, IN_MODIFY /*IN_ALL_EVENTS*/);

	if (wd < 0) 
	{
        g_log.Log(ERROR, "[%s-%d-%s]: Can't add watch for: %s", __FILE__, __LINE__, __FUNCTION__, SCAN_CONFIG_FILENAME);
        close(m_fd);
        m_fd = 0;
        return RET_ERROR;
    }

	return RET_SUCCESS;
}

int LoadConfigEngine::UnInit()
{
	if (m_fd > 0)
	{
		 close(m_fd);
		 m_fd = 0;
	}
	
	return RET_SUCCESS;
}

}



