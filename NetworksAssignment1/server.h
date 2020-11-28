#pragma once

#include "ClientsHandler.h"
#include "Utilities.h"
#include <list>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

static const double RATIO = 1.25;

// The delay when one socket is connected
static const double TIMEOUT = 10;

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
		std::cout << "Server Timeout: " << timeout << std::endl;
	}

	int init();

	int run();

};

