#include "SocketUtils.h"

using namespace std;

int SocketUtils::setNonblock(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL);
    if(flags & O_NONBLOCK)
	{
		return flags;
    }
	
    flags = flags | O_NONBLOCK;

	return fcntl(fd, F_SETFL, flags);
}

bool SocketUtils::setReuseaddr(FD fd)
{
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) != 0)
	{
        return false;
    }
    return true;
}

bool SocketUtils::setNodelay(FD fd)
{
    int flag = 1;
    if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) != 0)
	{
        return false;
    }
    return true;
}

bool SocketUtils::setSocketOpt(FD fd, int level, int opt, int value)
{
    int flag = value;

    if(setsockopt(fd, level, opt, &flag, sizeof(flag)) != 0)
	{
        return false;
    }
    return true;
}

