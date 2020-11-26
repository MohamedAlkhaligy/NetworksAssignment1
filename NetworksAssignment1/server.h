#pragma once

#include <thread>
#include "Utilities.h"

#pragma comment (lib, "ws2_32.lib")

class Server
{
private:
	const char* hostname;
	const char* port;
	char buffer[MAX_BUFFER];
	int timeout;
	SOCKET mySocket;

	void changeTimeout();

	void processClient(SOCKET clientSocket);

	void handleGETResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formatedRequest);

	void handlePOSTResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formattedRequest);

public:

	Server(const char* port = DEFAULT_SERVER_PORT, const char* hostname = DEFAULT_SERVER_HOSTNAME):
		hostname(hostname), port(port), timeout(10) { }

	int init();

	int run();


};

