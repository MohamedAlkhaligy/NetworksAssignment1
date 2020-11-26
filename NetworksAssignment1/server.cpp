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

void Server::changeTimeout() {

}

int Server::run() {

	while (RUNNING) {
		SOCKET clientSocket = accept(mySocket, nullptr, nullptr);
		// std::thread thread();
		processClient(clientSocket);
	}
}


int sendRequest(SOCKET socket, std::string message) {
	int numBytes = send(socket, message.c_str(), message.size(), 0);
	int isSent = !SUCCESS;
	if (numBytes < 0) {
		std::cerr << "Server: Sending the request has failed" << std::endl;
	}
	else if (numBytes != message.size()) {
		std::cerr << "Server: Sending unexpected number of bytes" << std::endl;
	}
	else {
		isSent = SUCCESS;
	}
	return isSent;
}

void Server::handleGETResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formattedRequest) {

	// Obtain content-lenghth size. (Size of file)
	std::vector<std::string> requestLine = formattedRequest[0];
	std::string myPath = SERVER + requestLine[1];
	std::ifstream file(myPath, std::ios::in | std::ios::binary | std::ifstream::ate);
	int fileSize = 0;
	std::string  statusCode = NOT_FOUND;
	if (file.good()) {
		statusCode = OKAY;
		fileSize = file.tellg();
	}

	// Create response of GET request
	std::ostringstream response;
	response << "HTTP/1.1 " << statusCode << "\r\n";
	response << CONTENT << " " << fileSize;
	response << SEPARATOR;
	std::string ouput = response.str();
	int numBytes = send(clientSocket, ouput.c_str(), ouput.size(), 0);
	if (numBytes < 0) {
		std::cerr << "Server: Sending the request has failed" << std::endl;
	}
	else if (numBytes != ouput.size()) {
		std::cerr << "Server: Sending unexpected number of bytes" << std::endl;
	}

	// Send required file to the client
	FILE* fp = NULL;
	if (!fopen_s(&fp, myPath.c_str(), "rb") && fp != NULL) {
		while (!feof(fp)) {
			size_t size = fread(buffer, 1, MAX_BUFFER, fp);
			int numBytes = send(clientSocket, buffer, size, 0);
			if (numBytes < 0) {
				std::cout << "Client: Sending the request has failed" << std::endl;
				break;
			}
		}
		fclose(fp);
	}
}

void Server::handlePOSTResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formattedRequest) {

	std::vector<std::string> requestLine = formattedRequest[0];
	std::string path = SERVER + requestLine[1];

	int expectedFileSize = 0, currentFileSize = 0;
	for (int i = 0; i < formattedRequest.size(); i++)
		if (formattedRequest[i][0] == CONTENT)
			expectedFileSize = stoi(formattedRequest[i][1]);

	// Create response for POST request
	std::ostringstream response;
	response << "HTTP/1.1 " << OKAY << "\r\n";
	response << SEPARATOR;

	// Send the response
	std::string ouput = response.str();
	int numBytes = send(clientSocket, ouput.c_str(), ouput.size(), 0);
	if (numBytes < 0) {
		std::cerr << "Server: Sending the request has failed" << std::endl;
	} else if (numBytes != ouput.size()) {
		std::cerr << "Server: Sending unexpected number of bytes" << std::endl;
	}

	// Receive file from client
	FILE* file = NULL;
	if (!fopen_s(&file, path.c_str(), "wb") && file != NULL) {
		while (expectedFileSize - currentFileSize > 0) {
			int numBytesRcvd = recv(clientSocket, buffer, MAX_BUFFER, 0);
			currentFileSize += numBytesRcvd;

			if (numBytesRcvd < 0) {
				std::cerr << "Client: Receiving file has failed" << std::endl;
				return;
			}
			else if (currentFileSize == expectedFileSize) {
				fwrite(buffer, numBytesRcvd, 1, file);
				break;
			}
			else {
				fwrite(buffer, numBytesRcvd, 1, file);
			}
		}
		fclose(file);
	}
}

void Server::processClient(SOCKET clientSocket) {

	while (RUNNING) {

		std::string request = Utilities::receiveMessage(clientSocket, buffer);

		// Separate entity body from request line and headers and restructre them.
		std::vector<std::string> separatedMessage = Utilities::separateMessage(request, SEPARATOR);
		std::cout << separatedMessage[0] << std::endl;

		// Formatted request is simply 
		std::vector<std::vector<std::string>> formattedRequest = Utilities::format(separatedMessage[0]);
	
		std::vector<std::string> requestLine = formattedRequest[0];
		if (requestLine[0] == GET) {
			handleGETResponse(clientSocket, formattedRequest);
		} else if (requestLine[0] == POST) {
			handlePOSTResponse(clientSocket, formattedRequest);
		}

		request.clear();

	}

}




