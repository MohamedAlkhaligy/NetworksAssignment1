#include "Client.h"

#define PORT_INDEX 3
#define CLIENT "client\\"
#define OK "200"
#define IGNORE '#'

int Client::init() {

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

	if ((status = getaddrinfo(serverHostname, serverPort, &hints, &res)) != 0) {
		std::cerr << "getaddrinfo error: %s\n" << gai_strerror(status);
		return status;
	}

	// Create a sokcet
	socketToServer = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (socketToServer == INVALID_SOCKET) {
		std::cerr << "Can't create socket, ERROR#" << WSAGetLastError() << std::endl;
		WSACleanup();
		return SOCKET_ERROR;
	}

	// Connect to the server
	int connection = connect(socketToServer, res->ai_addr, res->ai_addrlen);
	if (connection == SOCKET_ERROR) {
		std::cerr << "Can't connect to server, ERROR#" << WSAGetLastError() << std::endl;
		closesocket(socketToServer);
		WSACleanup();
		return SOCKET_ERROR;
	}

	return 0;
}

std::string receiveMessage(SOCKET socket, char* buffer) {
	// Receive request
	int numBytesRcvd = 1;
	std::ostringstream oss;
	std::string request, partial_data;
	while (numBytesRcvd > 0) {
		numBytesRcvd = recv(socket, buffer, MAX_BUFFER, 0);

		if (numBytesRcvd < 0) {
			std::cout << "Client: Receiving failed" << std::endl;
			return "";
		} else if (numBytesRcvd == 0) {
			std::cerr << "Client: Connection Closed" << std::endl;
			return "";
		}

		oss << std::string(buffer, numBytesRcvd);
		std::size_t found = oss.str().rfind(SEPARATOR);
		if (found != std::string::npos) {
			break;
		}
	}
	return oss.str();
}

void Client::handleGETRequest(std::string path, std::string hostname, const char* port) {
	try {
		std::string clientpath = CLIENT + Utilities::getFileName(path);

		// Creating the get request
		std::ostringstream request;
		request << "GET " << path << " HTTP/1.1\r\n";
		request << "Host: " << hostname << "\r\n";
		request << "\r\n";

		// Send get request to the server
		Utilities::sendRequest(socketToServer, request.str());
		
		// Receiving the file specified in the get request path
		std::string response = receiveMessage(socketToServer, buffer);

		// Test connection timeout
		// std::this_thread::sleep_for(std::chrono::seconds(30));
		
		if (response.empty()) {
			return;
		}

		std::vector<std::string> separatedMessage = Utilities::separateMessage(response, SEPARATOR);
		std::cout << separatedMessage[0];

		std::vector<std::vector<std::string>> formattedRequest = Utilities::format(separatedMessage[0]);

		// Part of file that was sent with request.
		int currentFileSize = separatedMessage[1].size(), expectedFileSize = 0;
		for (int i = 0; i < formattedRequest.size(); i++)
			if (formattedRequest[i][0] == CONTENT) 
				expectedFileSize = stoi(formattedRequest[i][1]);

		// Receive file
		FILE* file = NULL;
		if (!fopen_s(&file, clientpath.c_str(), "wb") && file != NULL) {
			fwrite(separatedMessage[1].c_str(), currentFileSize, 1, file);
			while (expectedFileSize - currentFileSize > 0) {
				int numBytesRcvd = recv(socketToServer, buffer, MAX_BUFFER, 0);
				currentFileSize += numBytesRcvd;

				if (numBytesRcvd < 0) {
					std::cerr << "Client: Receiving file has failed" << std::endl;
					isConnectionClosed = true;
					return;
				} else if (numBytesRcvd == 0) {
					std::cerr << "Client: Connection Closed" << std::endl;
					isConnectionClosed = true;
					return;
				} else if (currentFileSize == expectedFileSize) {
					fwrite(buffer, numBytesRcvd, 1, file);
					break;
				} else {
					fwrite(buffer, numBytesRcvd, 1, file);
				}
			}
			fclose(file);
		}
		
	} catch (const std::ifstream::failure& e) {
		std::cout << e.what() << std::endl;
	}
}

void Client::handlePOSTRequest(std::string path, std::string hostname, const char* port) {
	try {
		std::string myPath = CLIENT + Utilities::getFileName(path);

		// obtain content-lenghth size. (Size of file)
		std::ifstream ifs(myPath, std::ios::in | std::ios::binary | std::ifstream::ate);
		if (!ifs.good()) {
			std::cerr << "Client: Local file error." << std::endl;
			return;
		}

		// Create the POST request
		std::ostringstream request;
		request << "POST " << path << " HTTP/1.1\r\n";
		request << "Host: " << hostname << "\r\n";
		request << CONTENT << " " << ifs.tellg();
		request << SEPARATOR;

		// Send POST request to the server
		Utilities::sendRequest(socketToServer, request.str());

		// Receive a confirm from the server
		std::string response = receiveMessage(socketToServer, buffer);
		std::vector<std::string> separatedMessage = Utilities::separateMessage(response, SEPARATOR);
		std::cout << separatedMessage[0];

		// Send the file if the server has confirmed receiving it.
		std::vector<std::vector<std::string>> formattedRequest = Utilities::format(separatedMessage[0]);
		std::vector<std::string> requestLine = formattedRequest[0];
		if (requestLine[1] == OK) {
			FILE* fp = NULL;
			int bytes = 0;
			if (!fopen_s(&fp, myPath.c_str(), "rb") && fp != NULL) {
				while (!feof(fp)) {
					size_t size = fread(buffer, 1, MAX_BUFFER, fp);
					int numBytes = send(socketToServer, buffer, size, 0);
					bytes += numBytes;
					if (numBytes < 0) {
						std::cout << "Client: Sending the request has failed" << std::endl;
						break;
					}
				}
				fclose(fp);
			}
		}
	}
	catch (const std::ifstream::failure& e) {
		std::cout << e.what() << std::endl;
	}
}

int Client::isValidCommand(std::vector<std::string> tokens) {

	// Check whether number of command arguments is valid or not.
	if (tokens.size() < COMMAND_SIZE - OPTIONAL_ARGUMENTS || tokens.size() > COMMAND_SIZE) {
		std::cout << "Client command error!" << std::endl;
		return !SUCCESS;
	}


	// Check if the given command is supported (GET or POST)
	auto itr = commands.find(tokens[0]);
	if (itr == commands.end()) {
		std::cout << "Not supported command!" << std::endl;
		return !SUCCESS;
	}

	// If port argument exists, check whether it is a number or not. 
	if (tokens.size() == COMMAND_SIZE) {
		std::string port = tokens[PORT_INDEX];
		if (port.empty() && std::find_if(port.begin(), port.end(),
			[](unsigned char c) { return !std::isdigit(c); }) == port.end()) {
			std::cout << "Port must be a number!" << std::endl;
			return !SUCCESS;
		}
	}

	return SUCCESS;
}


void Client::executeCommand(std::string command) {

	// Split the command into its arguments.
	std::vector<std::string> tokens = Utilities::split(command);

	// Check if the given command arguments follow the specified format and since 
	// port argument is optional then complete it with default port 80 if it doesn't exists.
	// Format: Method Path Host-Name Port-Number
	if (isValidCommand(tokens) != SUCCESS) {
		std::cerr << "Not valid command" << std::endl;
		return;
	}

	// Execute the given command.
	std::string method = commands.find(tokens[0])->second;
	std::string path = tokens[1];
	std::string hostname = tokens[2];
	const char* port = (tokens.size() == PORT_INDEX) ? DEFAULT_SERVER_PORT : tokens[PORT_INDEX].c_str();

	if (method == GET) {
		handleGETRequest(path, hostname, port);
	} else if (method == POST) {
		handlePOSTRequest(path, hostname, port);
	}

}

int Client::run() {

	// Try to read the command file and execute each command separately.
	try {
		std::ifstream commandsFile(file_path);
		std::string command;
		while (getline(commandsFile, command) && !isConnectionClosed) {
			if (!command.empty() && command[0] != IGNORE)
				executeCommand(command);
		}
		closesocket(socketToServer);
	} catch(const std::ifstream::failure& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}

