#include "ClientsHandler.h"

#define SERVER "server\\"
#define RUNNING true

bool ClientsHandler::isTimeout(double timeout) {
	double diff = difftime(time(0), timeSinceIdle);
	// std::cout << diff << std::endl;
	if (diff > timeout) return true;
	return false;
}

std::vector<std::string> separateMessage(std::string message, const std::string separator) {

	std::string request, partial_data;
	std::size_t found = message.rfind(separator);
	if (found != std::string::npos) {
		int separator_index = found + separator.size();
		request = std::string(&message[0], &message[separator_index]);
		partial_data = std::string(&message[separator_index], &message[message.size()]);
	}
	else {
		std::cout << "Wrong Request format, no separator" << std::endl;
	}

	std::vector<std::string> result = { request, partial_data };
	return result;
}

std::vector<std::vector<std::string>> format(std::string request) {
	std::vector<std::vector<std::string>> formattedRequest;

	// Separate string into different lines.
	std::istringstream iss(request);
	std::string line;
	while (std::getline(iss, line, SEPARATOR[0])) {
		if (!line.empty()) {
			line.erase(std::remove(line.begin(), line.end(), SEPARATOR[1]), line.end());

			// Separate line into different tokens
			std::istringstream temp(line);
			std::vector<std::string> tokens;
			std::string token;
			while (std::getline(temp, token, ' ')) {
				if (!token.empty())
					tokens.push_back(token);
			}
			if (!tokens.empty())
				formattedRequest.push_back(tokens);
		}
	}

	return formattedRequest;
}


void ClientsHandler::processClient() {
	while (RUNNING) {

		std::string request = receiveMessage(clientSocket, buffer, prevMsgRemains);

		if (request.empty()) {
			std::cout << "Connection Ended" << std::endl;
			closed = true;
			break;
		}

		// Separate entity body from request line and headers and restructre them.
		std::vector<std::string> separatedMessage = separateMessage(request, SEPARATOR);
		std::cout << separatedMessage[0] << std::endl;

		// Formatted request is simply 
		std::vector<std::vector<std::string>> formattedRequest = format(separatedMessage[0]);

		std::vector<std::string> requestLine = formattedRequest[0];
		if (requestLine[0] == GET) {
			handleGETResponse(clientSocket, formattedRequest);
		} else if (requestLine[0] == POST) {
			handlePOSTResponse(clientSocket, formattedRequest);
		}
	}
}

std::string ClientsHandler::receiveMessage(SOCKET socket, char* buffer, std::string& prevMsgRemains) {
	// Receive request
	int numBytesRcvd = 1;
	std::ostringstream oss;
	oss << prevMsgRemains;
	prevMsgRemains = "";
	std::string request, partial_data;
	while (numBytesRcvd > 0) {
		numBytesRcvd = recv(socket, buffer, MAX_BUFFER, 0);
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
		std::cerr << "Server: Sending the request has failed" << std::endl;
	} else if (numBytes != message.size()) {
		std::cerr << "Server: Sending unexpected number of bytes" << std::endl;
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
				std::cout << "Server: Sending the request has failed" << std::endl;
				break;
			}
			time(&timeSinceIdle);
		}
		fclose(fp);
	}
}

std::string ClientsHandler::receivePOSTFile(SOCKET socket, std::string path, char* buffer, int expectedFileSize) {
	FILE* file = NULL;
	std::string restOfMessage;
	if (!fopen_s(&file, path.c_str(), "wb") && file != NULL) {
		while (expectedFileSize > 0) {
			int numBytesRcvd = recv(socket, buffer, MAX_BUFFER, 0);
			time(&timeSinceIdle);

			if (numBytesRcvd < 0) {
				std::cerr << "Server: Receiving file from client has failed" << std::endl;
				break;
			} else if (numBytesRcvd == 0) {
				std::cerr << "Server: socket disconnected" << std::endl;
				break;
			} else if (expectedFileSize - numBytesRcvd <= 0) {
				std::string temp = std::string(buffer, numBytesRcvd);
				restOfMessage = temp.substr(expectedFileSize, temp.size());
				fwrite(buffer, expectedFileSize, 1, file);
				std::cout << "Notification: File Received" << std::endl;
				break;
			} else {
				fwrite(buffer, numBytesRcvd, 1, file);
			}

			expectedFileSize -= numBytesRcvd;
		}
		fclose(file);
	}
	return restOfMessage;
}

void ClientsHandler::handlePOSTResponse(SOCKET clientSocket, std::vector<std::vector<std::string>>& formattedRequest) {

	// Get file path
	std::vector<std::string> requestLine = formattedRequest[0];
	std::string path = SERVER + requestLine[1];

	// Get expected file size from request headers
	int expectedFileSize = 0;
	for (int i = 0; i < formattedRequest.size(); i++)
		if (formattedRequest[i][0] == CONTENT)
			expectedFileSize = stoi(formattedRequest[i][1]);

	// Create response for POST request
	std::ostringstream response;
	response << "HTTP/1.1 " << OKAY << "\r\n";
	response << SEPARATOR;

	// Send the response
	sendMessage(clientSocket, response.str());

	// Receive file from client and return remaining message data afer file if exists.
	prevMsgRemains = receivePOSTFile(clientSocket, path, buffer, expectedFileSize);
}

void ClientsHandler::stop() {
	std::cout << "Timeout, terminating" << std::endl;
	closesocket(clientSocket);
}
