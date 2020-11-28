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

	std::string prevMsgRemains;

	std::string receiveMessage(SOCKET socket, char* buffer, std::string& prevMsgRemains);

	int sendMessage(SOCKET socket, std::string message);

	void handleGETResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formatedRequest);

	void handlePOSTResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formattedRequest);

	std::string receivePOSTFile(SOCKET socket, std::string path, char* buffer, int expectedFileSize);

public:

	bool closed;

	ClientsHandler(SOCKET clientSocket) :
		clientSocket(clientSocket) {
		timeSinceIdle = time(0);
		client = new std::thread([this]() {processClient();});
		closed = false;
		prevMsgRemains = "";
		std::cout << "Connected Socket: " << clientSocket << std::endl;
		// processClient();
	}

	bool isTimeout(double timeout);

	void stop();
};

