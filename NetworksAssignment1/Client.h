#pragma once

#include "Utilities.h"
#include <map>

static std::map<std::string, std::string> commands = {
	{"client_post", "POST"},
	{"client_get", "GET"}
};

static int COMMAND_SIZE = 4;
static int OPTIONAL_ARGUMENTS = 1;

class Client
{
private:
	const char* serverHostname;
	const char* serverPort;
	bool isConnectionClosed;
	char buffer[MAX_BUFFER];
	SOCKET socketToServer;
	std::string file_path;

	int isValidCommand(std::vector<std::string> tokens);

public: 
	Client(const char* serverHostname = DEFAULT_SERVER_HOSTNAME, const char* serverPort = DEFAULT_SERVER_PORT,
		std::string file_path = DEFAULT_PATH):
		serverHostname(serverHostname), serverPort(serverPort), file_path(file_path), isConnectionClosed(false) { }

	int init();

	int run();

	void handleGETRequest(std::string path, std::string hostname, const char* port);

	void handlePOSTRequest(std::string path, std::string hostname, const char* port);

	void executeCommand(std::string command);
};

