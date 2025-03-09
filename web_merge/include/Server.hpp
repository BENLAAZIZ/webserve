


// #ifndef SERVER_HPP
// #define SERVER_HPP

// #include "Config.hpp"
// #include "Client.hpp"
// #include <vector>
// #include <map>
// #include <poll.h>

// class Server {
// private:
//     Config _config;
//     std::vector<int> _listenSockets;
//     std::map<int, ServerConfig*> _socketToServer;
//     std::map<int, Client*> _clients;
//     std::vector<struct pollfd> _pollFds;
//     bool _running;
	
//     // Initialization
//     bool setupServerSockets();
//     bool configureSocket(int socket);
	
//     // Event handling
//     void handleNewConnection(int listenSocket);
//     void handleClientRead(int clientSocket);
//     void handleClientWrite(int clientSocket);
	
//     // Client management
//     void addClient(int clientSocket, ServerConfig* serverConfig);
//     void removeClient(int clientSocket);
//     void updatePollFds();
	
//     // Process client requests
//     void processClient(Client* client);
	
// public:
//     Server();
//     ~Server();
	
//     bool init(const std::string& configPath);
//     void run();
//     void stop();
// };

// #endif // SERVER_HPP


// //========================================================

#ifndef SERVER_HPP
#define SERVER_HPP


#include <vector>
#include <string>
#include <map>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "Config.hpp"
#include "Client.hpp"
#define MAX_CLIENTS 10

class Server {
	public:
		ServerConfig _config;
		int serverSocket;
		std::map<int, Client> _clients;
		std::vector<pollfd> _fds;
		std::vector<int> fds;


		int server_fd;
		bool throw_error;

		
		// Makes a socket non-blocking
		bool setNonBlocking(int sock);
		
		// Accepts new client connections
		void handleNewConnection();
		
		// Handles client data
		void handleClientData(int clientFd);
		
		// Removes disconnected client
		void removeClient(int clientFd);

	public:
		Server(const ServerConfig& config);
		Server(int port);
		~Server();
		
		bool createServer();
		void processEvents();
		// bool handleClientResponse(int clientFd);
		// bool clientReadyToSend(int clientFd);
		// int acceptNewConnection();
		// bool handleClientData(int clientFd);


};

#endif // SERVER_HPP