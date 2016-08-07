#include "ServerEngine.h"
#include "ServerCommon.h"
#include "DaemonProcess.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace NET_PACKET_CAPTURE_SERVER
{

extern DaemonProcess * g_DaemonProcess;

static void *clientRegist(void* data)
{
	pthread_detach(pthread_self());

	int *fd = (int*)data;
	int rev;
	int sockfd;
	char buf[DATA_LEN] = {0};
	fd_set readfds;
	struct timeval tv;

	if (NULL == fd || *fd < 0)
	{
		goto th_exit;
	}

	sockfd = *fd;

	do
	{
		rev = 0;
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);

		if (select(sockfd+1, &readfds, NULL, NULL, &tv) > 0)
		{
			if (FD_ISSET(sockfd, &readfds))
			{
				rev = recv(sockfd, buf, DATA_LEN, 0);
				if (rev > 0 && rev >= COMMAND_HEAD_LEN)
				{
					CmdHead *cmdHead = (CmdHead*)buf;
					if (strncmp((const char*)cmdHead->flag, (const char*)g_fixFlag, 4))
					{
						g_log.Log(ERROR, "[%s-%d-%s]: send error data head", __FILE__, __LINE__, __FUNCTION__);
                        goto th_exit;
					}

					switch(cmdHead->cmdType)
					{
					case BS_CMD_ARPCAPTURE_REGIST:
						g_log.Log(DEBUG, "[%s-%d-%s]: Arp capture daemon regist commad", __FILE__, __LINE__, __FUNCTION__);
						if (!g_DaemonProcess->arpCaptureThread.NotifyForTask(sockfd))
						{
							close(sockfd);
							sockfd = 0;
							g_log.Log(ERROR, "[%s-%d-%s]: Arp capture daemon is registed", __FILE__, __LINE__, __FUNCTION__);
						}
						break;

					/*case BS_CMD_HOSTSCAN_REGIST:
						break;*/
					default:
						g_log.Log(ERROR, "[%s-%d-%s]: error client regist command type: [0x%8x]", __FILE__, __LINE__, __FUNCTION__, cmdHead->cmdType);
						break;
					}
				}
			}
		}
		
	}while(0);

th_exit:
	if (NULL != fd)
	{
		delete fd;
		fd = NULL;
	}

	pthread_exit(NULL);
	return NULL;

}

ServerEngine::ServerEngine()
{
}

ServerEngine::~ServerEngine()
{
}

bool ServerEngine::start()
{
	return createServer();
}

void ServerEngine::stop()
{
}

void ServerEngine::loop(int flags)
{
	ev::loop_ref loop = ev::get_default_loop();
	loop.run(flags);
}

void ServerEngine::unloop(ev::how_t how)
{
	ev::loop_ref loop = ev::get_default_loop();
	loop.unloop(how);
}

bool ServerEngine::run()
{
	return true;
}

bool ServerEngine::init()
{
	return true;
}

bool ServerEngine::createServer()
{
	socketfd = socket(AF_INET, SOCK_STREAM, 0); //SOCK_CLOEXEC SOCK_STREAM
    if (socketfd < 0)
	{
        g_log.Log(ERROR, "[%s-%d-%s]: create socket failure, errno:%d", __FILE__, __LINE__, __FUNCTION__, errno);
        return false;
    }

    if (!SocketUtils::setReuseaddr(socketfd)) 
	{
        g_log.Log(ERROR, "[%s-%d-%s]: set reuseaddr failure, errno:%d", __FILE__, __LINE__, __FUNCTION__, errno);
        goto socket_err;
    }

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = INADDR_ANY;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(11000/*g_bsConf.listenPort*/);

    if (bind(socketfd, (struct sockaddr *)&address, sizeof(address)) < 0) 
	{
        g_log.Log(ERROR, "[%s-%d-%s]: bind failure, port:%d, errno:(%s)%d", __FILE__, __LINE__, __FUNCTION__, ntohs(address.sin_port), strerror(errno), errno);
        goto socket_err;
    }

    if (listen(socketfd, 16) < 0)
	{
        g_log.Log(ERROR, "[%s-%d-%s]: socket listen failure, errno:%d", __FILE__, __LINE__, __FUNCTION__, errno);
        goto socket_err;
    }

    if (SocketUtils::setNonblock(socketfd) != 0) 
	{
        g_log.Log(ERROR, "[%s-%d-%s]: set nonblock failure, errno:%d", __FILE__, __LINE__, __FUNCTION__, errno);
        goto socket_err;
    }

    evioSocket.set<ServerEngine, &ServerEngine::acceptCallback>(this);
    evioSocket.start(socketfd, EV_READ);
	
	return true;

socket_err:
	close(socketfd);
    return false;
	
}

void ServerEngine::acceptCallback(ev::io &evio, int revents)
{
	struct sockaddr_in clientAddr;

	static unsigned int recode = 1;
	socklen_t len = sizeof(struct sockaddr_in);

	int fd = accept(evio.fd, (struct sockaddr*)&clientAddr, &len);
	if (fd < 0)
	{
		if (1 == recode)
		{
			g_log.Log(WARNING, "[%s-%d-%s]: accept failure, errno:%s", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
            recode++;
		}
		return;
	}

	int* pFd = new int(fd);
	pthread_t tid;
	pthread_create(&tid, NULL, clientRegist, (void*)pFd);
	
	g_log.Log(DEBUG, "[%s-%d-%s]: New socket:[%d] is created ", __FILE__, __LINE__, __FUNCTION__, fd);
	
}

void ServerEngine::signalCallback(ev::sig &signal, int revents)
{
}

}

