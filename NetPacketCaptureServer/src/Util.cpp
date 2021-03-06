#include "Util.h"
#include "ServerCommon.h"
#include <errno.h>


using namespace NET_PACKET_CAPTURE_SERVER;

const int IDLE_TIME                     = 30;
const int INTVL_TIME                    = 5;
const int KEEP_COUNT                    = 3;

//static   SocketState    g_socketState   = SOCKET_NO_CREATE;
static   BaseLock       g_socketStateLock;

int Util::DesktopServerLockfile(int fd)
{
	struct flock fl;

	if (fd == -1) 
	{
        return -1;
    }

	fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

	return fcntl(fd, F_SETLK, &fl);
}

int Util::DaemonIsRunning(const char *pidFileName)
{
	int fd,val;  
    char buf[10];  
  
    if ((fd = open(pidFileName, O_WRONLY|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO)) < 0) 
    {  
        g_log.Log(ERROR, "[%s-%d-%s]: daemon need run by root. open %s error", __FILE__, __LINE__, __FUNCTION__, pidFileName); 
        return -1;  
    }  
  
    if(DesktopServerLockfile(fd) < 0)  
    {  
        if (errno == EACCES || errno == EAGAIN)  
        {
            g_log.Log(ERROR, "[%s-%d-%s]: daemon has been in running", __FILE__, __LINE__, __FUNCTION__); 
        }
        else
        {  
             g_log.Log(ERROR, "[%s-%d-%s]: daemon other error", __FILE__, __LINE__, __FUNCTION__); 
        }

        return -1;  
    }  
  
    if (ftruncate(fd, 0) < 0) 
    {  
        g_log.Log(ERROR, "[%s-%d-%s]: ftruncate error.", __FILE__, __LINE__, __FUNCTION__);
        return -1;  
    }  
  
    sprintf(buf,"%d\n",getpid());  
    if (write(fd, buf, strlen(buf)) != strlen(buf)) 
    {  
        g_log.Log(ERROR, "[%s-%d-%s]: pid write error.", __FILE__, __LINE__, __FUNCTION__);
        return -1;  
    }  
  
    // close file descriptor  
    if ((val = fcntl(fd, F_GETFD, 0)) < 0)
    {  
        g_log.Log(ERROR, "[%s-%d-%s]: fcntl F_GETFD error.", __FILE__, __LINE__, __FUNCTION__);
        return -1;  
    }  
    
    val |= FD_CLOEXEC;  
    if (fcntl(fd, F_SETFD, val) < 0) 
    {  
        g_log.Log(ERROR, "[%s-%d-%s]: fcntl F_SETFD error.", __FILE__, __LINE__, __FUNCTION__);
        return -1;  
    }  
  
    return 0;  
}

int Util::CreateDaemonProcess()
{
	pid_t pid, sid;  

    if((pid = fork()) < 0) 
    { 
        return -1;  
    }
    else if(pid != 0)
    {  
        exit(0); /* parent exit */  
    }
    /* child continues */  
    sid = setsid(); /* become session leader */  
    if (sid < 0) 
    {  
        exit(-1);  
    }  
    /* change working directory */  
    if ((chdir("/")) < 0) 
    {  
        exit(-1);  
    }  
    umask(0); /* clear file mode creation mask */  

#if 0  
    close(0); /* close stdin */  
    close(1); /* close stdout */  
    close(2); /* close stderr */  
#endif

    g_log.Log(INFO, "[%s-%d-%s]: Application Control daemon successfully initialized.", __FILE__, __LINE__, __FUNCTION__);

	return RET_SUCCESS;
}


void Util::SetSocketCntl(int socketfd)
{
	int flags;
    flags = fcntl(socketfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(socketfd, F_SETFL, flags);

    int keepAlive       = 5;
    int keepIdle        = IDLE_TIME;
    int keepInterval    = INTVL_TIME;
    int keepCount       = KEEP_COUNT;

    setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
}

void Util::SetCmdHead(CmdHead *cmdHead, CommandType cmdType, unsigned int versionID, RepResult repStatus, unsigned int length)
{
	memset(cmdHead, 0, COMMAND_HEAD_LEN);
	memcpy((void*)cmdHead->flag, (const char*)g_fixFlag, 4);
	cmdHead->cmdType = cmdType;
    cmdHead->versionID = versionID;
    cmdHead->repStatus = repStatus;
    cmdHead->length = length;
}


