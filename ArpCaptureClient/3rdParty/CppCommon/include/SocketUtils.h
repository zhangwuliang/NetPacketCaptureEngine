#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__

#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <arpa/inet.h>
#include <string.h>

/**
 * Socket tool
 */
typedef int FD;

class SocketUtils 
{
public:
    static int setNonblock(FD fd);

    static bool setReuseaddr(FD fd);

    static bool setNodelay(FD fd);

    static bool setSocketOpt(FD fd, int level, int opt, int value);

private:
    SocketUtils();
    virtual ~SocketUtils();
};

#endif	/* __SOCKET_UTILS_H__ */

