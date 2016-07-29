#ifndef __SERVER_ENGINE_H__
#define __SERVER_ENGINE_H__

#include <ev++.h>
#include <errno.h>
#include <cpp_common/SocketUtils.h>

#include "Definitions.h"

namespace LIBEVSERVER
{

class ServerEngine
{
public:
	ServerEngine();
	virtual ~ServerEngine();

	bool start();

	void stop();

	static void loop(int flags = 0);
	static void unloop(ev::how_t how = ev::ONE);

private:
	bool run();
	bool init();
	bool createServer();
	void acceptCallback(ev::io &evio, int revents);
	void signalCallback(ev::sig &signal, int revents);


private:
	int socketfd;
	ev::io evioSocket;
	ev::sig evsio;
	
};


}

#endif