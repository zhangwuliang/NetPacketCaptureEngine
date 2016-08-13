#include "Util.h"
#include <errno.h>

using namespace ARP_CAPTURE_CLIENT;

const int IDLE_TIME                     = 30;
const int INTVL_TIME                    = 5;
const int KEEP_COUNT                    = 3;

static   SocketState    g_socketState   = SOCKET_NO_CREATE;
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

void Util::SetSocketState(SocketState state)
{
	BSLock bsLock(g_socketStateLock);
	g_socketState = state;
}

SocketState Util::GetSocketState()
{
	BSLock bsLock(g_socketStateLock);
	return g_socketState;
}

int Util::RegisterMode(const char* server, const unsigned int port, CommandType commandType)
{
	int         socketfd;
	int         ret;
	int         totle;
	SocketState state;

	CmdHead cmdHead;
	SetCmdHead(&cmdHead, commandType, 0, RST_SUCCESS, COMMAND_HEAD_LEN);

	do
	{
		state = GetSocketState();
		g_log.Log(INFO, "[%s-%d-%s]: Socket state [%d]...", __FILE__, __LINE__, __FUNCTION__, (int)state);

		if (SOCKET_NO_CREATE == state)
		{
			socketfd = CreateSocket(server, port);
			if (socketfd > 0)
			{
				SetSocketState(SOCKET_CREATE);
				totle = 0;
				
				goto send_data;
			}
		}
		else if (SOCKET_CREATE == state)
		{
			//send register info
		send_data:
			ret = send(socketfd, (char*)(&cmdHead)+totle, COMMAND_HEAD_LEN-totle, 0);
			if (ret > 0) 
			{
				totle += ret;
				if (COMMAND_HEAD_LEN == totle)
				{
					SetSocketState(SOCKET_SHAKE_HANDS);
					totle = 0;
					return socketfd;
				}
			}
			else
			{
				close(socketfd);
				SetSocketState(SOCKET_NO_CREATE);
				totle = 0;
			}
		}
		else
		{
			g_log.Log(ERROR, "[%s-%d-%s]: UNKNOW Socket state [%d]...", __FILE__, __LINE__, __FUNCTION__, (int)state);
		}

		sleep(5);
		
	}while(1);

	return RET_SUCCESS;
	
}

int Util::CreateSocket(const char* server, const unsigned int port)
{
	int socketfd = 0;
	struct sockaddr_in serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));

	g_log.Log(INFO, "[%s-%d-%s]: create socket : [%s] : [%d]", __FILE__, __LINE__, __FUNCTION__, server, port);

	socketfd = socket(AF_INET,SOCK_STREAM,0);
    if (-1 == socketfd)
	{
        g_log.Log(INFO, "[%s-%d-%s]: create socket error:%s", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
        return -1;
    }

    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
    if (inet_aton(server, &serverAddr.sin_addr) == 0) 
	{
        g_log.Log(ERROR, "[%s-%d-%s]: Server IP Address Error!", __FILE__, __LINE__, __FUNCTION__);
        close(socketfd);
        return -1;
    }

    
    socklen_t server_addr_length = sizeof(serverAddr);
    if (connect(socketfd, (struct sockaddr*) &serverAddr, server_addr_length) < 0)
	{
        g_log.Log(INFO, "[%s-%d-%s]: Can Not Connect To: %s", __FILE__, __LINE__, __FUNCTION__, server);
        close(socketfd);
        return -1;
    }

    return socketfd;
}

void Util::SetCmdHead(CmdHead *cmdHead, CommandType cmdType, unsigned int versionID, RepResult repStatus, unsigned int length)
{
	memset(cmdHead, 0, COMMAND_HEAD_LEN);
	memcpy((void*)cmdHead->flag, (const char*)g_fixFlag, 4);
	cmdHead->cmdType   = cmdType;
    cmdHead->versionID = versionID;
    cmdHead->repStatus = repStatus;
    cmdHead->length    = length;
}

int Util::getJsonInt(Json::Value value, const std::string item)
{
	if (!value[item].isNull() && value[item].isInt())
	{
		return value.get(item.c_str(), 0).asInt();
	}

	return 0;
}

std::string Util::getJsonString(Json::Value value, const std::string item)
{
	if (!value[item].isNull() && value[item].isString())
	{
		return value.get(item.c_str(), "").asString();
	}

	return "";
}

