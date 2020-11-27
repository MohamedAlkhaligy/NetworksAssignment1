// NetworksAssignment1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Server.h"
#include "Client.h"
#include "Utilities.h"

#define SERVER_HOST "my_server"
#define CLIENT_HOST "my_client"
#define SERVER_HOSTNAME_INDEX 2
#define SERVER_PORT_INDEX 1
#define CLIENT_HOSTNAME_INDEX 1
#define CLIENT_PORT_INDEX 2

#define SUCCESS 0

int main()
{
    
    std::string command;
    getline(std::cin, command);
    Utilities* utilities = new Utilities();
    std::vector<std::string> tokens = utilities->split(command);

    if (tokens[0] == SERVER_HOST) {
        const char* port = (tokens.size() == 1) ? DEFAULT_SERVER_PORT : tokens[SERVER_PORT_INDEX].c_str();
        const char* hostname = (tokens.size() <= SERVER_HOSTNAME_INDEX) ? DEFAULT_SERVER_HOSTNAME : tokens[SERVER_HOSTNAME_INDEX].c_str();
        std::cout << "Server: IP: " << hostname << " Port: " << port << std::endl;
        Server server = Server(port, hostname);
        if (server.init() == SUCCESS) {
            server.run();
        } else {
            std::cout << "Server Booting Failure: IP: " << hostname << " Port: " << port << std::endl;
        }
    } else if (tokens[0] == CLIENT_HOST) {
        const char* hostname = (tokens.size() == CLIENT_HOSTNAME_INDEX) ? DEFAULT_SERVER_HOSTNAME : tokens[CLIENT_HOSTNAME_INDEX].c_str();
        const char* serverPort = (tokens.size() <= CLIENT_PORT_INDEX) ? DEFAULT_SERVER_PORT : tokens[CLIENT_PORT_INDEX].c_str();
        Client client = Client(hostname, serverPort);
        if (client.init() == SUCCESS) {
            client.run();
        }

    }

    std::cout << "Program finished" << std::endl;
    std::cin.get();
    return 0;
}

