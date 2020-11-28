#include "server.h"

#define RUNNING true
#define SERVER "server\\"

int Server::init() {

	// Initialize WinSock
	WSADATA data;
	WORD version = MAKEWORD(MAJOR, MINOR);
	int wsaResult = WSAStartup(version, &data);
	if (wsaResult != SUCCESS) {
		std::cerr << "Can't start WinSock, ERROR#" << wsaResult << std::endl;
		return wsaResult;
	}

	// Fill in a hint structure
	int status;
	struct addrinfo hints;
	struct addrinfo* res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
		std::cerr << "getaddrinfo error: %s\n" << gai_strerror(status);
		return status;
	}

	// Create a sokcet
	mySocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (mySocket == INVALID_SOCKET) {
		std::cerr << "Can't create socket, ERROR#" << WSAGetLastError() << std::endl;
	}

	// Bind the ip address and port to a socket
	if (bind(mySocket, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR)
	{
		std::cerr << "Can't create socket, ERROR#" << WSAGetLastError() << std::endl;
		return WSAGetLastError();
	}

	// Tell WinSock the socket is for listening
	if (listen(mySocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Can't create socket, ERROR#" << WSAGetLastError() << std::endl;
		return WSAGetLastError();
	}

	return 0;
}

void Server::watchConnections() {
	while (RUNNING) {
		auto it = clients.begin();
		while (it != clients.end()) {
			if ((*it)->isTimeout(timeout)) {
				(*it)->stop();
				it = clients.erase(it);
				timeout *= RATIO;
				std::cout << "Server Timeout: " << timeout << std::endl;
			} else if ((*it)->closed) {
				it = clients.erase(it);
				timeout *= RATIO;
				std::cout << "CLOSED" << std::endl;
			} else {
				++it;
			}
		}
	}
}

int Server::run() {

	std::thread* watch = new std::thread([this]() {watchConnections();});
	while (RUNNING) {
		SOCKET clientSocket = accept(mySocket, nullptr, nullptr);
		ClientsHandler* client = new ClientsHandler(clientSocket);
		clients.push_back(client);
		timeout /= RATIO;
		std::cout << "Server Timeout: " << timeout << std::endl;
	}
}


