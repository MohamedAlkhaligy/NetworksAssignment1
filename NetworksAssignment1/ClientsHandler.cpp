#include "ClientsHandler.h"

#define SERVER "server\\"
#define RUNNING true

bool ClientsHandler::isTimeout(double timeout) {
	double diff = difftime(time(0), timeSinceIdle);
	// std::cout << diff << std::endl;
	if (diff > timeout) return true;
	return false;
}

void ClientsHandler::processClient() {
	std::cout << "Connected Socket: " << clientSocket << std::endl;
	while (RUNNING) {

		std::string request = receiveMessage(clientSocket, buffer);

		if (request.empty()) {
			std::cout << "Connection Ended" << std::endl;
			closed = true;
			break;
		}

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
	}
}

std::string ClientsHandler::receiveMessage(SOCKET socket, char* buffer) {
	// Receive request
	int numBytesRcvd = 1;
	std::ostringstream oss;
	std::string request, partial_data;
	while (numBytesRcvd > 0) {
		numBytesRcvd = recv(socket, buffer, MAX_BUFFER - 1, 0);
		if (numBytesRcvd < 0) {
			return "";
		} else if (numBytesRcvd == 0) {
			return "";
		}

		time(&timeSinceIdle);
		oss << std::string(buffer, numBytesRcvd);
		std::size_t found = oss.str().rfind(SEPARATOR);
		if (found != std::string::npos) {
			break;
		}
	}
	return oss.str();
}

int ClientsHandler::sendMessage(SOCKET socket, std::string message) {
	int numBytes = send(socket, message.c_str(), message.size(), 0);
	time(&timeSinceIdle);
	int isSent = !SUCCESS;
	if (numBytes < 0) {
		std::cerr << "Client: Sending the request has failed" << std::endl;
	} else if (numBytes != message.size()) {
		std::cerr << "Client: Sending unexpected number of bytes" << std::endl;
	} else {
		isSent = SUCCESS;
	}
	return isSent;
}

void ClientsHandler::handleGETResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formattedRequest) {

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

	// Send response
	sendMessage(clientSocket, response.str());

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
			time(&timeSinceIdle);
		}
		fflush(fp);
		fclose(fp);
	}
}

void ClientsHandler::handlePOSTResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formattedRequest) {

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
	sendMessage(clientSocket, response.str());

	// Receive file from client
	FILE* file = NULL;
	if (!fopen_s(&file, path.c_str(), "wb") && file != NULL) {
		while (expectedFileSize - currentFileSize > 0) {
			int numBytesRcvd = recv(clientSocket, buffer, MAX_BUFFER, 0);
			time(&timeSinceIdle);
			currentFileSize += numBytesRcvd;

			if (numBytesRcvd < 0) {
				std::cerr << "Client: Receiving file has failed" << std::endl;
				break;
			} else if (currentFileSize == expectedFileSize) {
				fwrite(buffer, numBytesRcvd, 1, file);
				break;
			} else {
				fwrite(buffer, numBytesRcvd, 1, file);
			}
		}
		fflush(file);
		fclose(file);
		std::cout << "Sent" << std::endl;
	}
}

void ClientsHandler::stop() {
	std::cout << "Timeout, terminating" << std::endl;
	closesocket(clientSocket);
}
