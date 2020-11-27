#pragma once

#include "ClientsHandler.h"
#include "Utilities.h"
#include <list>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

static const double RATIO = 1.5;
static const double TIMEOUT = 15;

class Server
{
private:
	const char* hostname;
	const char* port;
	SOCKET mySocket;
	double timeout; // in seconds
	std::list<ClientsHandler*> clients;

	void watchConnections();

public:

	Server(const char* port = DEFAULT_SERVER_PORT, const char* hostname = DEFAULT_SERVER_HOSTNAME):
		hostname(hostname), port(port) { 
		timeout = TIMEOUT * RATIO;
	}

	int init();

	int run();

};

