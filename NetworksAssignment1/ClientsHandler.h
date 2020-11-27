#pragma once
#include "Utilities.h"
#include <thread>

class ClientsHandler {
private:

	char buffer[MAX_BUFFER];

	time_t timeSinceIdle;

	std::thread* client;

	SOCKET clientSocket;

	void processClient();

	std::string receiveMessage(SOCKET socket, char* buffer);

	int sendMessage(SOCKET socket, std::string message);

	void handleGETResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formatedRequest);

	void handlePOSTResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formattedRequest);
public:

	bool closed;

	ClientsHandler(SOCKET clientSocket) :
		clientSocket(clientSocket) {
		timeSinceIdle = time(0);
		client = new std::thread([this]() {processClient();});
		closed = false;
		// processClient();
	}

	bool isTimeout(double timeout);

	void stop();
};

