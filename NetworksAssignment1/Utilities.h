#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <direct.h>
#include <WS2tcpip.h>
#include <time.h>

#include <chrono>
#include <thread>

static const char* DEFAULT_SERVER_HOSTNAME = "localhost";
static const char* DEFAULT_SERVER_PORT = "80";
static const int MAX_BUFFER = 4096;
static const int MAJOR = 2;
static const int MINOR = 2;
static const int SUCCESS = 0;
static const std::string DEFAULT_PATH = "commands.txt";
static const std::string SEPARATOR = "\r\n\r\n";
static const std::string GET = "GET";
static const std::string POST = "POST";
static const std::string CONTENT = "Content-Length:";
static const std::string NOT_FOUND = "404 Not Found";
static const std::string OKAY = "200 OK";

class Utilities
{

public:
	static std::vector<std::string> split(std::string s);

	static std::vector<std::string> separateMessage(std::string message, const std::string separator);

	static std::vector<std::vector<std::string>> format(std::string request);

	static std::string getFileName(std::string path);

	static int sendRequest(SOCKET socket, std::string message);

};

