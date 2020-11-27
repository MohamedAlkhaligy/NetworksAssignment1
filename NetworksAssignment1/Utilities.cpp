#include "Utilities.h"

std::vector<std::string> Utilities::split(std::string s) {
    std::stringstream stringstream(s);
    std::istream_iterator<std::string> begin(stringstream);
    std::istream_iterator<std::string> end;
    std::vector<std::string> tokens(begin, end);
    return tokens;
}

std::vector<std::string> Utilities::separateMessage(std::string message, const std::string separator) {

	std::string request, partial_data;
	std::size_t found = message.rfind(separator);
	if (found != std::string::npos) {
		int separator_index = found + separator.size();
		request = std::string(&message[0], &message[separator_index]);
		partial_data = std::string(&message[separator_index], &message[message.size()]);
	} else {
		std::cout << "Wrong Request format, no separator" << std::endl;
	}

	std::vector<std::string> result = { request, partial_data };
	return result;
}

std::vector<std::vector<std::string>> Utilities::format(std::string request) {
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

std::string Utilities::getFileName(std::string path) {
	std::size_t found = path.rfind('\\');
	std::string fileName = path;
	if (found != std::string::npos) {
		int separator_index = found;
		fileName = std::string(&path[separator_index], &path[path.size()]);
	}
	return fileName;
}

int Utilities::sendRequest(SOCKET socket, std::string message) {
	int numBytes = send(socket, message.c_str(), message.size(), 0);
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