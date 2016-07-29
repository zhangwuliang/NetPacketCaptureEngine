#include "ServerEngine.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>



namespace LIBEVSERVER
{

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
}

void ServerEngine::unloop(ev::how_t how)
{
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

    g_log.Log(INFO, "[%s-%d-%s]: Create listen socket: [%d]", __FILE__, __LINE__, __FUNCTION__, socketfd);

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
}

void ServerEngine::signalCallback(ev::sig &signal, int revents)
{
}

}

